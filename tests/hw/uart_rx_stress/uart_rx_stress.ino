// uart_rx_stress -- hardware bench test for the core Uart RX path.
//
// Streams a known, modem-shaped line pattern out of Serial1 TX and back into
// Serial1 RX through a jumper, while a high-priority task periodically hogs
// the CPU the way LMIC / BLE / QSPI work does in the field. Every received
// line is checked byte-for-byte against what was sent. Any dropped, doubled
// or corrupted byte shows up as a bad line with the offending text printed.
//
// Wiring (Connectivity 840):  jumper D2 (Serial1 TX)  <->  D3 (Serial1 RX)
// Monitor:                    USB CDC "Serial", any baud
//
// Expected result on a healthy driver: after a 60 s run,
//     bad=0 lost=0 overruns=0        -> "verdict: PASS"
//
// Background: the field failure this reproduces was a single byte dropped
// mid-URC ("ADRASEA-IReady") on a device whose UART RX was already
// double-buffered EasyDMA with an idle-flush software timer. The drop came
// from the idle-flush stopping RX mid-byte when its 1 ms tick ran late and
// FreeRTOS replayed the missed periods back to back; without RXTO wait +
// FLUSHRX, the byte in flight was discarded. The stall task here recreates
// that late tick on demand.
//
// Keys over USB serial (one-way, no echo):
//   s  toggle stall task            (default ON)
//   c  toggle critical-section stalls (default ON)
//   g  toggle mid-line TX gaps      (default ON)
//   r  reset counters
//   ?  print this key list

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>   // pulls in the USB CDC "Serial" used for the report

#ifndef STRESS_BAUD
#define STRESS_BAUD 115200
#endif

#define REPORT_MS       5000
#define RUN_VERDICT_MS  60000
#define MAX_LINE        96
#define N_SAMPLES       6

// Same shape as the Adrastea "LTE ready" URC that was corrupted in the field,
// plus a 6-digit sequence number so lost whole lines are detectable too.
static const char LINE_FMT[] = "%%SCMNOTIFYEV:\"ADRASTEA-IReady\" #%06lu\r\n";

// ---- knobs ----------------------------------------------------------------
static volatile bool g_stall = true;   // prio-3 busy stalls (delays the FreeRTOS timer task)
static volatile bool g_crit  = true;   // short taskENTER_CRITICAL stalls (masks IRQs, like LMIC hal_disableIRQs)
static volatile bool g_gaps  = true;   // split some TX lines with a 0.7..3.2 ms mid-line pause (modem-style stall)

// ---- counters (single writer each, read from loop()) ----------------------
static volatile uint32_t c_tx_lines   = 0;
static volatile uint32_t c_rx_bytes   = 0;
static volatile uint32_t c_rx_ok      = 0;
static volatile uint32_t c_rx_bad     = 0;   // line content != expected for its seq
static volatile uint32_t c_rx_lost    = 0;   // seq numbers never seen (whole line lost / unparseable)
static volatile uint32_t c_rx_overlen = 0;   // no '\n' within MAX_LINE
static volatile uint32_t c_stalls     = 0;
static volatile uint32_t c_split_tx   = 0;

static char     s_samples[N_SAMPLES][MAX_LINE * 4];
static volatile uint32_t s_nsamples = 0;
static volatile bool     s_reset_req = false;

// Small deterministic PRNG so both tasks stay independent of libc rand().
static uint32_t s_lfsr = 0xACE1u;
static uint32_t prng(void)
{
  uint32_t x = s_lfsr;
  x ^= x << 13; x ^= x >> 17; x ^= x << 5;
  s_lfsr = x ? x : 0xACE1u;
  return x;
}

static void escape_into(char* dst, size_t cap, const uint8_t* src, size_t n)
{
  size_t o = 0;
  for (size_t i = 0; i < n && o + 5 < cap; i++)
  {
    uint8_t c = src[i];
    if (c >= 0x20 && c < 0x7F && c != '\\') dst[o++] = (char)c;
    else o += snprintf(dst + o, cap - o, "\\x%02X", c);
  }
  dst[o] = 0;
}

static void record_sample(const uint8_t* line, size_t n, const char* why)
{
  uint32_t i = s_nsamples;
  if (i >= N_SAMPLES) return;
  size_t cap = sizeof(s_samples[0]);
  int o = snprintf(s_samples[i], cap, "[%s] ", why);
  if (o < 0) o = 0;
  escape_into(s_samples[i] + o, cap - (size_t)o, line, n);
  s_nsamples = i + 1;
}

// ---- TX task (prio LOW, like loop()) ---------------------------------------
static void txTask(void*)
{
  char buf[MAX_LINE];
  uint32_t seq = 0;
  for (;;)
  {
    if (s_reset_req) { vTaskDelay(pdMS_TO_TICKS(20)); continue; }

    int n = snprintf(buf, sizeof(buf), LINE_FMT, (unsigned long)seq);

    if (g_gaps && (prng() & 3) == 0)
    {
      // Modem-style mid-line stall: send a prefix, let it drain to the wire,
      // pause long enough for the idle-flush to consider the line quiet, then
      // send the rest. The first byte after the pause is the one at risk.
      int split = 1 + (int)(prng() % (uint32_t)(n - 2));
      Serial1.write((const uint8_t*)buf, split);
      Serial1.flush();
      delayMicroseconds(700 + (prng() % 2500));
      Serial1.write((const uint8_t*)buf + split, n - split);
      c_split_tx++;
    }
    else
    {
      Serial1.write((const uint8_t*)buf, n);
    }
    seq++;
    c_tx_lines++;

    // Every 16 lines let the line go idle for 1..4 ms so the idle-flush path
    // is exercised constantly (short partial buffers, not just full ones).
    if ((seq & 15) == 0)
    {
      Serial1.flush();
      vTaskDelay(pdMS_TO_TICKS(1 + (prng() % 4)));
    }
    if (seq >= 1000000UL) seq = 0;
  }
}

// ---- RX task (prio NORMAL, same as the modem rxTask in the field) ---------
static void check_line(const uint8_t* line, size_t n, uint32_t* last_seq, bool* have_last)
{
  const uint8_t* hash = (const uint8_t*)memchr(line, '#', n);
  if (!hash || (size_t)(hash - line) + 7 > n)
  {
    c_rx_bad++; c_rx_lost++;          // cannot even attribute it to a seq
    record_sample(line, n, "no-seq");
    return;
  }
  uint32_t seq = 0;
  for (int i = 1; i <= 6; i++)
  {
    uint8_t d = hash[i];
    if (d < '0' || d > '9') { c_rx_bad++; c_rx_lost++; record_sample(line, n, "bad-seq"); return; }
    seq = seq * 10 + (d - '0');
  }

  char expect[MAX_LINE];
  int en = snprintf(expect, sizeof(expect), LINE_FMT, (unsigned long)seq);

  if ((size_t)en == n && memcmp(expect, line, n) == 0) c_rx_ok++;
  else { c_rx_bad++; record_sample(line, n, "mismatch"); }

  if (*have_last && seq > *last_seq + 1) c_rx_lost += (seq - *last_seq - 1);
  *last_seq = seq;
  *have_last = true;
}

static void rxTask(void*)
{
  static uint8_t line[MAX_LINE];
  size_t len = 0;
  uint32_t last_seq = 0;
  bool have_last = false;

  for (;;)
  {
    if (s_reset_req)
    {
      while (Serial1.available()) Serial1.read();
      len = 0; have_last = false;
      vTaskDelay(pdMS_TO_TICKS(30));
      s_reset_req = false;
      continue;
    }
    while (Serial1.available())
    {
      int c = Serial1.read();
      if (c < 0) break;
      c_rx_bytes++;
      line[len++] = (uint8_t)c;
      if (c == '\n')
      {
        check_line(line, len, &last_seq, &have_last);
        len = 0;
      }
      else if (len >= MAX_LINE)
      {
        c_rx_overlen++; c_rx_bad++;
        record_sample(line, len, "overlen");
        len = 0;
      }
    }
    vTaskDelay(1);
  }
}

// ---- stall task (prio HIGH, like the Bluefruit task) -----------------------
// Every 17 ms: hog the CPU for ~3 ms so the FreeRTOS timer service task
// (prio NORMAL) misses ticks, then optionally a 500 us critical section, the
// kind of IRQ-masked stretch LMIC's radio driver produces around SPI bursts.
static void stallTask(void*)
{
  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(17));
    if (g_stall)
    {
      uint32_t t0 = millis();
      while ((uint32_t)(millis() - t0) < 3) { __NOP(); }
      c_stalls++;
    }
    if (g_crit)
    {
      taskENTER_CRITICAL();
      delayMicroseconds(500);
      taskEXIT_CRITICAL();
    }
  }
}

// ---- reporting --------------------------------------------------------------
static void print_keys(void)
{
  Serial.println(F("keys: s=stall c=critical g=gaps r=reset ?=help"));
}

static void report(uint32_t elapsed_ms, bool final_verdict)
{
  uint32_t ok = c_rx_ok, bad = c_rx_bad, lost = c_rx_lost;
  uint32_t ovr = Serial1.getRxOverruns();
#ifdef UART_RX_FIFO_FLUSH_BYTES
  uint32_t rescued = Serial1.getRxFifoRescued();   // bytes saved by FLUSHRX (fixed driver only)
#else
  uint32_t rescued = 0;
#endif

  Serial.printf("[%6lu s] tx=%lu rx_ok=%lu bad=%lu lost=%lu overlen=%lu bytes=%lu "
                "hw_overruns=%lu fifo_rescued=%lu stalls=%lu split_tx=%lu  [stall=%c crit=%c gaps=%c]\n",
                (unsigned long)(elapsed_ms / 1000), (unsigned long)c_tx_lines,
                (unsigned long)ok, (unsigned long)bad, (unsigned long)lost,
                (unsigned long)c_rx_overlen, (unsigned long)c_rx_bytes,
                (unsigned long)ovr, (unsigned long)rescued, (unsigned long)c_stalls, (unsigned long)c_split_tx,
                g_stall ? 'Y' : 'n', g_crit ? 'Y' : 'n', g_gaps ? 'Y' : 'n');

  uint32_t ns = s_nsamples;
  for (uint32_t i = 0; i < ns; i++) Serial.printf("   sample %lu: %s\n", (unsigned long)i, s_samples[i]);

  if (c_rx_bytes == 0 && elapsed_ms > 2000)
    Serial.println(F("   !! no RX data at all: is D2 (Serial1 TX) jumpered to D3 (Serial1 RX)?"));

  if (final_verdict)
  {
    bool pass = (bad == 0 && lost == 0 && ovr == 0 && ok > 1000);
    Serial.printf("verdict: %s (%lu lines checked, %lu bad, %lu lost, %lu hw overruns)\n",
                  pass ? "PASS" : "FAIL", (unsigned long)ok, (unsigned long)bad,
                  (unsigned long)lost, (unsigned long)ovr);
  }
}

static void reset_counters(void)
{
  s_reset_req = true;
  c_tx_lines = c_rx_bytes = c_rx_ok = c_rx_bad = c_rx_lost = c_rx_overlen = 0;
  c_stalls = c_split_tx = 0;
  s_nsamples = 0;
  Serial1.clearRxOverruns();
}

static uint32_t s_t0;
static uint32_t s_last_report;
static bool     s_verdict_done;

void setup()
{
  Serial.begin(115200);
  uint32_t w = millis();
  while (!Serial && (millis() - w) < 3000) delay(10);

  Serial.println();
  Serial.println(F("uart_rx_stress: Serial1 loopback RX integrity under CPU/IRQ stalls"));
  Serial.printf("baud=%lu dma=%u ring=%u  jumper D2<->D3\n",
                (unsigned long)STRESS_BAUD, (unsigned)UART_RX_DMA_SIZE, (unsigned)SERIAL_BUFFER_SIZE);
  print_keys();

  Serial1.begin(STRESS_BAUD);
  delay(50);
  while (Serial1.available()) Serial1.read();

  xTaskCreate(rxTask,    "stressRX",    1024, NULL, TASK_PRIO_NORMAL, NULL);
  xTaskCreate(txTask,    "stressTX",    1024, NULL, TASK_PRIO_LOW,    NULL);
  xTaskCreate(stallTask, "stressStall",  512, NULL, TASK_PRIO_HIGH,   NULL);

  s_t0 = millis();
  s_last_report = s_t0;
  s_verdict_done = false;
}

void loop()
{
  while (Serial.available())
  {
    int k = Serial.read();
    switch (k)
    {
      case 's': g_stall = !g_stall; Serial.printf("stall %s\n", g_stall ? "ON" : "off"); break;
      case 'c': g_crit  = !g_crit;  Serial.printf("critical %s\n", g_crit ? "ON" : "off"); break;
      case 'g': g_gaps  = !g_gaps;  Serial.printf("gaps %s\n", g_gaps ? "ON" : "off"); break;
      case 'r': reset_counters(); s_t0 = millis(); s_verdict_done = false; Serial.println(F("counters reset")); break;
      case '?': print_keys(); break;
      default: break;
    }
  }

  uint32_t now = millis();
  if ((uint32_t)(now - s_last_report) >= REPORT_MS)
  {
    s_last_report = now;
    uint32_t elapsed = now - s_t0;
    bool verdict = (!s_verdict_done && elapsed >= RUN_VERDICT_MS);
    report(elapsed, verdict);
    if (verdict) s_verdict_done = true;
  }
  delay(20);
}
