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

#ifndef _VARIANT_CHALLENGER_840_
#define _VARIANT_CHALLENGER_840_

/** Master clock frequency */
#define VARIANT_MCK       (64000000ul)

#define USE_LFXO      // Board uses 32khz crystal for LF
//#define USE_LFRC    // Board uses RC for LF

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Number of pins defined in PinDescription array
#define PINS_COUNT           (47u)
#define NUM_DIGITAL_PINS     (47u)
#define NUM_ANALOG_INPUTS    (5u)
#define NUM_ANALOG_OUTPUTS   (0u)

// LEDs
#define PIN_LED1             (31u)

#define LED_BUILTIN          PIN_LED1
#define LED_GREEN            PIN_LED1
#define LED_BLUE             PIN_LED1

#define LED_STATE_ON         1         // State when LED is litt

/*
 * Analog pins
 */
#define PIN_A0               (16u)
#define PIN_A1               (17u)
#define PIN_A2               (18u)
#define PIN_A3               (19u)
#define PIN_A4               (20u)

static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
static const uint8_t A4  = PIN_A4; // Battery measurement pin
#define ADC_RESOLUTION    14

// Other pins
#define PIN_AREF           (21u)
#define PIN_VBAT           PIN_A4

static const uint8_t AREF = PIN_AREF;

/*
 * Serial interfaces
 */

/*
 * Serial1 available on the expansion header
 */
#define PIN_SERIAL1_RX      (3u)
#define PIN_SERIAL1_TX      (2u)

/*
 * Serial2 that connects to the Modem
 * Notice the use of non BSP pin names to ensure that
 * it does not instantiate the serial port.
 */
#define PIN_SERIAL2_RX      (42u)
#define PIN_SERIAL2_TX      (39u)
#define PIN_SERIAL2_RTS     (40u)
#define PIN_SERIAL2_CTS     (41u)

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 2

/*
 * SPI interface on expansion connector
 */
#define PIN_SPI_MISO         (15u)
#define PIN_SPI_MOSI         (12u)
#define PIN_SPI_SCK          (13u)

static const uint8_t SS   = 14u;
static const uint8_t MOSI = PIN_SPI_MOSI ;
static const uint8_t MISO = PIN_SPI_MISO ;
static const uint8_t SCK  = PIN_SPI_SCK ;

/*
 * LoRa SPI
 */
#define PIN_SPI1_MISO        (27u)
#define PIN_SPI1_MOSI        (26u)
#define PIN_SPI1_SCK         (25u)
#define PIN_SPI1_CS          (24u)
#define LORA_SPI             SPI1

/*
 * LoRa Sideband pins
 */
#define PIN_LORA_DIO0        (28u)
#define PIN_LORA_DIO1        (29u)
#define PIN_LORA_DIO2        (30u)

/*
 * Wire Interfaces
 */
#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (0u)
#define PIN_WIRE_SCL         (1u)

// QSPI Pins
#define PIN_QSPI_SCK         (37u)
#define PIN_QSPI_CS          (36u)
#define PIN_QSPI_IO0         (33u)
#define PIN_QSPI_IO1         (35u)
#define PIN_QSPI_IO2         (34u)
#define PIN_QSPI_IO3         (38u)

// On-board QSPI Flash
// EXTERNAL_FLASH_DEVICES is defined in the Arduino menu
#define EXTERNAL_FLASH_USE_QSPI

/*
 * On board on/off controllable power domains
 */
#define PIN_NBIOT_PWR_ENABLE  (4u)
#define PIN_NBIOT_WAKEUP      (5u)
#define PIN_NBIOT_RST         (6u)
#define PIN_LORA_PWR_ENABLE   (30u)
#define PIN_LORA_RST          (23u)

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#endif
