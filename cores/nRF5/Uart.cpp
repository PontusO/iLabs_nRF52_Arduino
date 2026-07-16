/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Uart.h"
#include "Arduino.h"
#include "wiring_private.h"


void serialEventRun(void)
{
  if (serialEvent && Serial.available() ) serialEvent();

#if defined(PIN_SERIAL1_RX) && defined(PIN_SERIAL1_TX)
  if (serialEvent1 && Serial1.available() ) serialEvent1();
#endif

#if defined(PIN_SERIAL2_RX) && defined(PIN_SERIAL2_TX)
  if (serialEvent2 && Serial2.available() ) serialEvent2();
#endif
}

Uart::Uart(NRF_UARTE_Type *_nrfUart, IRQn_Type _IRQn, uint8_t _pinRX, uint8_t _pinTX)
{
  nrfUart = _nrfUart;
  IRQn = _IRQn;
  uc_pinRX = g_ADigitalPinMap[_pinRX];
  uc_pinTX = g_ADigitalPinMap[_pinTX];
  uc_hwFlow = 0;

  _end_tx_sem = NULL;
  _begun = false;
  _rxIdx = 0;
  _rxActivity = false;
  _rxFlushTimer = NULL;
}

Uart::Uart(NRF_UARTE_Type *_nrfUart, IRQn_Type _IRQn, uint8_t _pinRX, uint8_t _pinTX, uint8_t _pinCTS, uint8_t _pinRTS)
{
  nrfUart = _nrfUart;
  IRQn = _IRQn;
  uc_pinRX = g_ADigitalPinMap[_pinRX];
  uc_pinTX = g_ADigitalPinMap[_pinTX];
  uc_pinCTS = g_ADigitalPinMap[_pinCTS];
  uc_pinRTS = g_ADigitalPinMap[_pinRTS];
  uc_hwFlow = 1;

  _end_tx_sem = NULL;
  _begun = false;
  _rxIdx = 0;
  _rxActivity = false;
  _rxFlushTimer = NULL;
}

void Uart::setPins(uint8_t pin_rx, uint8_t pin_tx)
{
  uc_pinRX = g_ADigitalPinMap[pin_rx];
  uc_pinTX = g_ADigitalPinMap[pin_tx];
}

void Uart::begin(unsigned long baudrate)
{
  begin(baudrate, (uint16_t)SERIAL_8N1);
}

void Uart::begin(unsigned long baudrate, uint16_t config)
{
  // skip if already begun
  if ( _begun ) return;

  nrfUart->PSEL.TXD = uc_pinTX;
  nrfUart->PSEL.RXD = uc_pinRX;

  if (uc_hwFlow == 1) {
    nrfUart->PSEL.CTS = uc_pinCTS;
    nrfUart->PSEL.RTS = uc_pinRTS;
    nrfUart->CONFIG = config | (UARTE_CONFIG_HWFC_Enabled << UARTE_CONFIG_HWFC_Pos);
  } else {
    nrfUart->CONFIG = config | (UARTE_CONFIG_HWFC_Disabled << UARTE_CONFIG_HWFC_Pos);
  }

  uint32_t nrfBaudRate;

  // Use Nordic's pre-computed values for standard baudrates to match other
  // nRF devices exactly; fall back to runtime computation for custom values.
  switch (baudrate) {
    case 1200:    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud1200;   break;
    case 2400:    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud2400;   break;
    case 4800:    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud4800;   break;
    case 9600:    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud9600;   break;
    case 14400:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud14400;  break;
    case 19200:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud19200;  break;
    case 28800:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud28800;  break;
    case 31250:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud31250;  break;
    case 38400:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud38400;  break;
    case 56000:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud56000;  break;
    case 57600:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud57600;  break;
    case 76800:   nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud76800;  break;
    case 115200:  nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud115200; break;
    case 230400:  nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud230400; break;
    case 250000:  nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud250000; break;
    case 460800:  nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud460800; break;
    case 921600:  nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud921600; break;
    case 1000000: nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud1M;     break;
    default: {
      // BAUDRATE = round(baudrate * 2^32 / 16e6) and round to nearest 0x1000.
      // https://devzone.nordicsemi.com/f/nordic-q-a/391/uart-baudrate-register-values
      const uint32_t regValue = (uint32_t)(((uint64_t)baudrate * 268435456ULL + 500000ULL) / 1000000ULL);
      nrfBaudRate = (regValue + 0x800) & 0xFFFFF000;
      break;
    }
  }

  nrfUart->BAUDRATE = nrfBaudRate;

  nrfUart->ENABLE = UARTE_ENABLE_ENABLE_Enabled;

  nrfUart->TXD.PTR = (uint32_t)txBuffer;
  nrfUart->EVENTS_ENDTX = 0x0UL;

  // Double-buffered continuous RX. The ENDRX->STARTRX short lets the hardware
  // re-arm reception into the next buffer with no ISR in the loop, so a burst
  // keeps landing in RAM even while the RX ISR is starved. RXSTARTED queues the
  // OTHER buffer as the next DMA target (ping-pong); ENDRX drains a filled
  // buffer to the ring. Partial (short) lines are surfaced by rxFlushTick().
  _rxIdx = 0;
  _rxActivity = false;
  nrfUart->EVENTS_ENDRX     = 0x0UL;
  nrfUart->EVENTS_RXSTARTED = 0x0UL;
  nrfUart->EVENTS_RXDRDY    = 0x0UL;
  nrfUart->RXD.PTR    = (uint32_t)rxDma[0];
  nrfUart->RXD.MAXCNT = UART_RX_DMA_SIZE;
  nrfUart->SHORTS     = UARTE_SHORTS_ENDRX_STARTRX_Msk;
  nrfUart->TASKS_STARTRX = 0x1UL;

  nrfUart->INTENSET = UARTE_INTENSET_ENDRX_Msk | UARTE_INTENSET_RXSTARTED_Msk
                    | UARTE_INTENSET_ENDTX_Msk;

  NVIC_ClearPendingIRQ(IRQn);
  NVIC_SetPriority(IRQn, 3);
  NVIC_EnableIRQ(IRQn);

  _end_tx_sem = xSemaphoreCreateBinary();
  xSemaphoreGive(_end_tx_sem);
  _begun = true;

  // ~1 ms idle-flush tick (see rxFlushTick). Created after _begun so the
  // callback is safe to run immediately.
  _rxFlushTimer = xTimerCreate("uartRxFlush", pdMS_TO_TICKS(1) ? pdMS_TO_TICKS(1) : 1,
                               pdTRUE, this, Uart::rxFlushTimerCb);
  if (_rxFlushTimer) xTimerStart(_rxFlushTimer, 0);
}

void Uart::end()
{
  // Stop + delete the idle-flush timer first, and drop _begun, so its callback
  // can't touch the UARTE while we tear it down.
  _begun = false;
  if (_rxFlushTimer)
  {
    xTimerStop(_rxFlushTimer, portMAX_DELAY);
    xTimerDelete(_rxFlushTimer, portMAX_DELAY);
    _rxFlushTimer = NULL;
  }

  NVIC_DisableIRQ(IRQn);

  nrfUart->INTENCLR = UARTE_INTENSET_ENDRX_Msk | UARTE_INTENSET_RXSTARTED_Msk
                    | UARTE_INTENSET_ENDTX_Msk;

  nrfUart->SHORTS = 0;   // stop ENDRX->STARTRX so STOPRX actually stops RX

  nrfUart->EVENTS_RXTO = 0;
  nrfUart->EVENTS_TXSTOPPED = 0;

  nrfUart->TASKS_STOPRX = 0x1UL;
  nrfUart->TASKS_STOPTX = 0x1UL;

  // Wait for TXSTOPPED event and for RXTO event
  // This is required before disabling UART to fully power down transceiver PHY.
  // Otherwise transceiver will continue to consume ~900uA
  while ( !(nrfUart->EVENTS_TXSTOPPED && nrfUart->EVENTS_RXTO) ) yield();

  nrfUart->ENABLE = UARTE_ENABLE_ENABLE_Disabled;

  nrfUart->PSEL.TXD = 0xFFFFFFFF;
  nrfUart->PSEL.RXD = 0xFFFFFFFF;

  nrfUart->PSEL.RTS = 0xFFFFFFFF;
  nrfUart->PSEL.CTS = 0xFFFFFFFF;

  rxBuffer.clear();

  vSemaphoreDelete(_end_tx_sem);
  _end_tx_sem = NULL;
  _begun = false;
}

void Uart::flush()
{
  if ( _begun ) {
    xSemaphoreTake(_end_tx_sem, portMAX_DELAY);
    xSemaphoreGive(_end_tx_sem);
  }
}

void Uart::IrqHandler()
{
  // A DMA buffer completed (filled to MAXCNT, or ended by rxFlushTick's STOPRX).
  // Drain it to the ring and advance to the buffer the short is now filling.
  // Handle ENDRX BEFORE RXSTARTED so, when both are pending (ISR ran late), we
  // advance _rxIdx first and then queue the correct next buffer.
  if (nrfUart->EVENTS_ENDRX)
  {
    nrfUart->EVENTS_ENDRX = 0x0UL;
    uint32_t amount = nrfUart->RXD.AMOUNT;
    const uint8_t* buf = rxDma[_rxIdx];
    for (uint32_t i = 0; i < amount; i++)
    {
      rxBuffer.store_char(buf[i]);
    }
    _rxIdx ^= 1;
  }

  // Reception into a new buffer has started: point the NEXT transfer (the one
  // the ENDRX->STARTRX short will trigger) at the other buffer -- ping-pong.
  if (nrfUart->EVENTS_RXSTARTED)
  {
    nrfUart->EVENTS_RXSTARTED = 0x0UL;
    nrfUart->RXD.PTR = (uint32_t)rxDma[_rxIdx ^ 1];
  }

  if (nrfUart->EVENTS_ENDTX)
  {
    nrfUart->EVENTS_ENDTX = 0x0UL;
    xSemaphoreGiveFromISR(_end_tx_sem, NULL);
  }
}

// ~1 ms tick: surface a partially-filled RX buffer once the line goes quiet.
// The ENDRX interrupt only fires on a FULL buffer, so without this a short line
// (e.g. "OK\r\n") would sit in DMA until more bytes arrive. RXDRDY is set for
// every received byte; we treat "RXDRDY since last tick" as "still arriving" and
// only flush on a quiet tick, so STOPRX never races an in-flight byte.
void Uart::rxFlushTick()
{
  if (!_begun) return;

  if (nrfUart->EVENTS_RXDRDY)
  {
    nrfUart->EVENTS_RXDRDY = 0x0UL;
    _rxActivity = true;                 // bytes arriving; let the burst run
    return;
  }
  if (!_rxActivity) return;             // idle and nothing new since last flush
  _rxActivity = false;

  // Quiet interval after activity: flush the current partial buffer. Take the
  // short down so stop/restart is deterministic, and mask the RX ISR so it
  // can't race the manual sequence. Because the line is quiet, no in-flight
  // byte is lost across the STOPRX/STARTRX.
  NVIC_DisableIRQ(IRQn);

  nrfUart->SHORTS       = 0;
  nrfUart->EVENTS_ENDRX = 0x0UL;
  nrfUart->TASKS_STOPRX = 0x1UL;
  uint32_t guard = 0;
  while (!nrfUart->EVENTS_ENDRX && ++guard < 100000) { }
  nrfUart->EVENTS_ENDRX = 0x0UL;

  uint32_t amount = nrfUart->RXD.AMOUNT;
  const uint8_t* buf = rxDma[_rxIdx];
  for (uint32_t i = 0; i < amount; i++)
  {
    rxBuffer.store_char(buf[i]);
  }

  // Re-arm: refill the same buffer and restore the short. The RXSTARTED that
  // STARTRX raises will queue the other buffer as before.
  nrfUart->EVENTS_RXSTARTED = 0x0UL;
  nrfUart->EVENTS_ENDRX     = 0x0UL;
  nrfUart->EVENTS_RXTO      = 0x0UL;
  nrfUart->RXD.PTR    = (uint32_t)rxDma[_rxIdx];
  nrfUart->RXD.MAXCNT = UART_RX_DMA_SIZE;
  nrfUart->SHORTS     = UARTE_SHORTS_ENDRX_STARTRX_Msk;
  nrfUart->TASKS_STARTRX = 0x1UL;

  NVIC_EnableIRQ(IRQn);
}

void Uart::rxFlushTimerCb(TimerHandle_t t)
{
  Uart* self = (Uart*)pvTimerGetTimerID(t);
  if (self) self->rxFlushTick();
}

int Uart::available()
{
  return rxBuffer.available();
}

int Uart::peek()
{
  return rxBuffer.peek();
}

int Uart::read()
{
  return rxBuffer.read_char();
}

size_t Uart::write(uint8_t data)
{
  return write(&data, 1);
}

size_t Uart::write(const uint8_t *buffer, size_t size)
{
  if(size == 0) return 0;

  size_t sent = 0;

  do
  {
    size_t remaining = size - sent;
    size_t txSize = min(remaining, (size_t)SERIAL_BUFFER_SIZE);

    xSemaphoreTake(_end_tx_sem, portMAX_DELAY);

    memcpy(txBuffer, buffer + sent, txSize);

    nrfUart->TXD.MAXCNT = txSize;
    nrfUart->TASKS_STARTTX = 0x1UL;
    sent += txSize;

  } while (sent < size);

  return sent;
}

int Uart::availableForWrite(void) {
  // UART does not use ring buffer for TX, therefore it is either busy or not
  UBaseType_t available = uxSemaphoreGetCount(_end_tx_sem);
  return available ? SERIAL_BUFFER_SIZE : 0;
}

//------------- Serial1 (or Serial in case of nRF52832) -------------//
#ifdef NRF52832_XXAA
  Uart Serial( NRF_UARTE0, UARTE0_UART0_IRQn, PIN_SERIAL_RX, PIN_SERIAL_TX );
#else
  Uart Serial1( NRF_UARTE0, UARTE0_UART0_IRQn, PIN_SERIAL1_RX, PIN_SERIAL1_TX );
#endif

extern "C"
{
  void UARTE0_UART0_IRQHandler()
  {
    SERIAL_PORT_HARDWARE.IrqHandler();
  }
}

//------------- Serial2 -------------//
#if defined(PIN_SERIAL2_RX) && defined(PIN_SERIAL2_TX) && \
    defined(PIN_SERIAL2_CTS) && defined(PIN_SERIAL2_RTS)
Uart Serial2( NRF_UARTE1, UARTE1_IRQn, PIN_SERIAL2_RX, PIN_SERIAL2_TX,
              PIN_SERIAL2_CTS, PIN_SERIAL2_RTS );
#elif defined(PIN_SERIAL2_RX) && defined(PIN_SERIAL2_TX)
Uart Serial2( NRF_UARTE1, UARTE1_IRQn, PIN_SERIAL2_RX, PIN_SERIAL2_TX );
#endif

#if defined(PIN_SERIAL2_RX) && defined(PIN_SERIAL2_TX)
extern "C"
{
  void UARTE1_IRQHandler()
  {
    Serial2.IrqHandler();
  }
}
#endif

