/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.
  Copyright (c) 2018, Adafruit Industries (adafruit.com)

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

#include "variant.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include "nrf.h"

#define _PINNUM(port, pin)     ((port)*32 + (pin))

const uint32_t g_ADigitalPinMap[] = {
  _PINNUM(0,  7), // D0 is also SDA and available on expansion connector *
  _PINNUM(0, 26), // D1 is also SCL and available on expansion connector *
  _PINNUM(1,  4), // D2 is also TXD and available on expansion connector *
  _PINNUM(0, 25), // D3 is also RXD and available on expansion connector *
  _PINNUM(1,  2), // D4 is connected to NBIOT_PWR_ENABLE and is internal only *
  _PINNUM(1,  6), // D5 is connected to #NBIOT_WAKEUP and is internal only *
  _PINNUM(1,  5), // D6 is connected to NBIOT_RST and is internal only *
  _PINNUM(1,  1), // D7 is connected to ACC_INT and is internal only *

  _PINNUM(1, 13), // D8 is available on expansion connector *
  _PINNUM(1, 15), // D9 is available on expansion connector *
  _PINNUM(0, 29), // D10 is available on expansion connector *
  _PINNUM(0, 31), // D11 is also GNSS_BIAS_ENABLE and available on expansion connector *
  _PINNUM(1, 14), // D12 is also SPI0/SDO and available on expansion connector *
  _PINNUM(1, 10), // D13 is also SPI0/SCK and available on expansion connector *
  _PINNUM(1, 12), // D14 is also SPI0/#CS and available on expansion connector *
  _PINNUM(1, 11), // D15 is also SPI0/SDI and available on expansion connector *

  _PINNUM(0,  2), // D16 is also A0 and available on expansion connector *
  _PINNUM(0,  3), // D17 is also A1 and available on expansion connector *
  _PINNUM(0,  4), // D18 is also A2 and available on expansion connector *
  _PINNUM(0,  5), // D19 is also A3 and available on expansion connector *
  _PINNUM(0, 28), // D20 is A4 (Battery measurement) and is internal only *
  _PINNUM(0, 30), // D21 is A5 (ARef) and is available on expansion connector *
  _PINNUM(0, 14), // D22 is also LORA_DIO2 and is available on expansion connector *
  _PINNUM(0, 12), // D23 is #LORA_RST *

  _PINNUM(0, 21), // D24 is SPI_#LORA_CS *
  _PINNUM(0, 22), // D25 is SPI_LORA_SCK *
  _PINNUM(0, 20), // D26 is SPI_LORA_SDO *
  _PINNUM(0, 23), // D27 is SPI_LORA_SDI *
  _PINNUM(0, 24), // D28 is LORA_DIO0 *
  _PINNUM(1,  0), // D29 is LORA_DIO1 *
  _PINNUM(1,  7), // D30 is connected to LORA_PWR_EN and is internal only *
  _PINNUM(0, 27), // D31 is LED1 (Green) *

  _PINNUM(1,  3), // D32 is the bootsel button *
  _PINNUM(0,  6), // D33 is FLASH_SPI_SD0 *
  _PINNUM(1,  9), // D34 is FLASH_SPI_SD2 *
  _PINNUM(0,  8), // D35 is FLASH_SPI_SD1 *
  _PINNUM(0, 16), // D36 is #FLASH_QSPI_CS *
  _PINNUM(1,  8), // D37 is FLASH_SPI_SCLK *
  _PINNUM(0, 11), // D38 is FLASH_SPI_SD3 *
  _PINNUM(0, 17), // D39 is NBIOT_RXD i.e. the TX pin of the MCU *

  _PINNUM(0, 19), // D40 is NBIOT_CTS i.e. the RTS pin of the MCU *
  _PINNUM(0, 13), // D41 is NBIOT_RTS i.e. the CTS pin of the MCU *
  _PINNUM(0, 15), // D42 is NBIOT_TXD i.e. the RX pin of the MCU *
  _PINNUM(0,  9), // Connected to NFC antenna, do not use as GPIO *
  _PINNUM(0, 10), // Connected to NFC antenna, do not use as GPIO *
  _PINNUM(0, 18), // Used as reset *
  _PINNUM(0,  0), // Not connected (Internal oscillator) *
  _PINNUM(0,  1), // Not connected (Internal oscillator) *
};

void initVariant()
{
  // LED1
  pinMode(PIN_LED1, OUTPUT);
  ledOff(PIN_LED1);

  pinMode(PIN_NBIOT_PWR_ENABLE, OUTPUT);
  digitalWrite(PIN_NBIOT_PWR_ENABLE, LOW);

  pinMode(PIN_LORA_PWR_ENABLE, OUTPUT);
  digitalWrite(PIN_LORA_PWR_ENABLE, LOW);
}
