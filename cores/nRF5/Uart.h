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

#pragma once

#include <nrf.h>

#include "HardwareSerial.h"
#include "RingBuffer.h"
#include "rtos.h"
#include "variant.h"

#include <cstddef>

// RX EasyDMA buffer size (per ping-pong buffer). The receiver double-buffers
// with an ENDRX->STARTRX hardware short, so the UARTE can DMA up to this many
// bytes into RAM with NO per-byte ISR -- tolerating that many bytes' worth of
// interrupt latency before an overrun (vs. ~4-6 bytes with the old 1-byte DMA).
// At 115200 baud, 128 bytes ~= 11 ms of ISR-latency headroom.
#ifndef UART_RX_DMA_SIZE
#define UART_RX_DMA_SIZE 128
#endif

// Idle-flush quiet threshold, in whole RTOS ticks (1024 Hz -> ~2 ms). The RX
// line must have shown no RXDRDY for this long before a partially-filled DMA
// buffer is flushed to the ring. Must be >= 2: FreeRTOS replays missed
// auto-reload periods back to back, and every replayed callback reads the same
// tick count, so a tick-count threshold is what stops a late tick from
// flushing (and stopping RX on) a line that is still live.
#ifndef UART_RX_IDLE_TICKS
#define UART_RX_IDLE_TICKS 2
#endif

// Scratch buffer for TASKS_FLUSHRX. The UARTE keeps a 4-byte internal RX FIFO
// that still accepts bytes after STOPRX; FLUSHRX moves it into RAM. 8 leaves
// margin above the hardware size.
#define UART_RX_FIFO_FLUSH_BYTES 8

// Bounded spin for RXTO / FLUSHRX completion inside the stop sequence. RXTO on
// an idle line is immediate; with a byte in flight it takes one byte time
// (87 us at 115200, ~1 ms at 9600). 50000 iterations is several ms at 64 MHz,
// a hardware-misbehaviour backstop, not a path that is expected to run.
#define UART_RX_STOP_GUARD 50000

class Uart : public HardwareSerial
{
  public:
    Uart(NRF_UARTE_Type *_nrfUart, IRQn_Type _IRQn, uint8_t _pinRX, uint8_t _pinTX);
    Uart(NRF_UARTE_Type *_nrfUart, IRQn_Type _IRQn, uint8_t _pinRX, uint8_t _pinTX, uint8_t _pinCTS, uint8_t _pinRTS);

    void setPins(uint8_t pin_rx, uint8_t pin_tx);
    void begin(unsigned long baudRate);
    void begin(unsigned long baudrate, uint16_t config);
    void end();
    int available();
    int availableForWrite(void);
    int peek();
    int read();
    void flush();
    size_t write(uint8_t data);
    size_t write(const uint8_t *buffer, size_t size);
    using Print::write; // pull in write(str) from Print

    void IrqHandler();

    operator bool ()
    {
      return _begun;
    }

    // Cumulative count of UARTE RX overruns since begin()/clear -- bytes the
    // hardware dropped because the RX FIFO/DMA overflowed before the ISR could
    // service it (i.e. under heavy interrupt latency). 0 == no loss. Additive
    // diagnostic; the rest of the HardwareSerial API is unchanged. Handy for
    // validating and tuning RX robustness under contention.
    uint32_t getRxOverruns() const { return _rxOverruns; }
    void     clearRxOverruns()     { _rxOverruns = 0; }

    // Cumulative count of bytes recovered from the UARTE internal RX FIFO by
    // FLUSHRX during idle-flush stop/restart cycles. Each one is a byte that
    // arrived while reception was being stopped and would have been lost by a
    // STOPRX/STARTRX without the flush. Diagnostic; 0 is normal on a quiet
    // line, non-zero is the safety net doing its job.
    uint32_t getRxFifoRescued() const { return _rxFifoRescued; }

  private:
    // Idle-flush: drains a partially-filled RX DMA buffer once the line goes
    // quiet, so short/partial lines surface via available()/read() promptly
    // (the ENDRX interrupt alone only fires on a FULL buffer). Runs off a ~1 ms
    // FreeRTOS software timer; entirely internal -- the public API is unchanged.
    void rxFlushTick();
    void rxStopFlushRestart();
    void rxDrainDma(uint8_t idx, uint32_t amount);
    static void rxFlushTimerCb(TimerHandle_t t);

    NRF_UARTE_Type *nrfUart;
    RingBuffer rxBuffer;
    uint8_t rxDma[2][UART_RX_DMA_SIZE];   // ping-pong EasyDMA RX buffers
    volatile uint8_t  _rxIdx;             // which buffer is currently filling (0/1)
    volatile bool     _rxActivity;        // RXDRDY seen since last idle flush
    volatile TickType_t _rxLastTick;      // tick count when RXDRDY was last seen
    volatile uint32_t _rxOverruns;        // cumulative UARTE RX overrun count
    volatile uint32_t _rxFifoRescued;     // bytes recovered via FLUSHRX (see getter)
    uint8_t rxFlush[UART_RX_FIFO_FLUSH_BYTES]; // FLUSHRX scratch target
    TimerHandle_t    _rxFlushTimer;
    uint8_t txBuffer[SERIAL_BUFFER_SIZE];

    IRQn_Type IRQn;

    uint8_t uc_pinRX;
    uint8_t uc_pinTX;
    uint8_t uc_pinCTS;
    uint8_t uc_pinRTS;
    uint8_t uc_hwFlow;

    bool _begun;

    // Adafruit
    SemaphoreHandle_t _end_tx_sem;
};


// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
#ifdef NRF52832_XXAA
  #define SERIAL_PORT_MONITOR         Serial
  #define SERIAL_PORT_HARDWARE        Serial

#else
  #define SERIAL_PORT_MONITOR         Serial
  #define SERIAL_PORT_USBVIRTUAL      Serial

  #define SERIAL_PORT_HARDWARE        Serial1
  #define SERIAL_PORT_HARDWARE_OPEN   Serial1

#endif

extern Uart SERIAL_PORT_HARDWARE;

#if defined(PIN_SERIAL2_RX) && defined(PIN_SERIAL2_TX)
extern Uart Serial2;
#endif
