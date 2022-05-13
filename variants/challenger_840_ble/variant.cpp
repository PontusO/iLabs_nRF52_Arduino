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
  _PINNUM(0, 23), // D0 is UART TX
  _PINNUM(0, 21), // D1 is UART RX
  _PINNUM(1,  1), // D2 is not connected
  _PINNUM(1, 11), // D3 is SDA
  _PINNUM(1, 12), // D4 is SCL
  _PINNUM(1, 10), // D5
  _PINNUM(1, 14), // D6
  _PINNUM(0, 25), // D7 is not connected
  _PINNUM(1,  0), // D8 is not connected
  _PINNUM(1, 13), // D9
  _PINNUM(1, 15), // D10
  _PINNUM(0, 27), // D11
  _PINNUM(0,  7), // D12
  _PINNUM(0,  6), // D13

  _PINNUM(0,  4), // D14 is A0
  _PINNUM(0,  5), // D15 is A1
  _PINNUM(0, 28), // D16 is A2
  _PINNUM(0, 29), // D17 is A3
  _PINNUM(0,  2), // D18 is A4
  _PINNUM(0,  3), // D19 is A5
  _PINNUM(0, 30), // D20 is A6 (Battery)
  _PINNUM(0, 31), // D21 is A7 (ARef)

  _PINNUM(1,  5), // D22 is not connected
  _PINNUM(1,  7), // D23 is not connected

  _PINNUM(0, 17), // D24 is SPI MISO
  _PINNUM(0, 15), // D25 is SPI MOSI
  _PINNUM(0, 13), // D26 is SPI SCK

  _PINNUM(0, 26), // D27 is LED1 (Green)
  _PINNUM(0, 12), // D28 is LED2 (Blue)
  _PINNUM(0, 19), // D29 is the push button
  _PINNUM(1,  8), // D30 is NeoPixel
  _PINNUM(1,  9), // D31 is V Enable

  _PINNUM(0, 14), // D32 is FLASH_SCK
  _PINNUM(0, 16), // D33 is FLASH_SDO
  _PINNUM(0, 11), // D34 is FLASH_SDI
  _PINNUM(0,  8), // D35 is FLASH_CS

  _PINNUM(0,  0), // D36 is not connected
  _PINNUM(0,  1), // D37 is not connected
  _PINNUM(0,  9), // D38 is not connected
  _PINNUM(0, 10), // D39 is not connected
  _PINNUM(0, 20), // D40 is not connected
  _PINNUM(0, 22), // D41 is not connected
  _PINNUM(0, 24), // D42 is not connected

  _PINNUM(1,  2), // D43 is not connected
  _PINNUM(1,  3), // D44 is not connected
  _PINNUM(1,  4), // D45 is not connected
  _PINNUM(1,  6), // D46 is not connected
};

void initVariant()
{
  // LED1 & LED2
  pinMode(PIN_LED1, OUTPUT);
  ledOff(PIN_LED1);

  pinMode(PIN_LED2, OUTPUT);
  ledOff(PIN_LED2);

  pinMode(PIN_LDO_CONTROL, OUTPUT);
  digitalWrite(PIN_LDO_CONTROL, HIGH);

  pinMode(PIN_SPI1_CS, OUTPUT);
  digitalWrite(PIN_SPI1_CS, HIGH);
}
