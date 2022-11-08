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
// define USE_LFRC    // Board uses RC for LF

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Number of pins defined in PinDescription array
#define PINS_COUNT           (36u)
#define NUM_DIGITAL_PINS     (36u)
#define NUM_ANALOG_INPUTS    (8u)
#define NUM_ANALOG_OUTPUTS   (0u)

// LEDs
#define PIN_LED1             (27u)
#define PIN_LED2             (28u)

#define LED_BUILTIN          PIN_LED1
#define LED_CONN             PIN_LED2

#define LED_GREEN            PIN_LED1
#define LED_BLUE             PIN_LED2
#define LED_NEOPIXEL         (30u)
#define NEOPIXEL             LED_NEOPIXEL

#define LED_STATE_ON         1         // State when LED is litted

/*
 * Analog pins
 */
#define PIN_A0               (14u)
#define PIN_A1               (15u)
#define PIN_A2               (16u)
#define PIN_A3               (17u)
#define PIN_A4               (18u)
#define PIN_A5               (19u)
#define PIN_A6               (20u)
#define PIN_A7               (21u)

static const uint8_t A0  = PIN_A0 ;
static const uint8_t A1  = PIN_A1 ;
static const uint8_t A2  = PIN_A2 ;
static const uint8_t A3  = PIN_A3 ;
static const uint8_t A4  = PIN_A4 ;
static const uint8_t A5  = PIN_A5 ;
static const uint8_t A6  = PIN_A6 ;
static const uint8_t A7  = PIN_A7 ;
#define ADC_RESOLUTION    14

// Other pins
#define PIN_AREF           (21u)
#define PIN_VBAT           PIN_A6

static const uint8_t AREF = PIN_AREF;

/*
 * Serial interfaces
 */
#define PIN_SERIAL1_RX      (1u)
#define PIN_SERIAL1_TX      (0u)

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 2

#define PIN_SPI_MISO         (24u)
#define PIN_SPI_MOSI         (25u)
#define PIN_SPI_SCK          (26u)

static const uint8_t SS   = 5u ;
static const uint8_t MOSI = PIN_SPI_MOSI ;
static const uint8_t MISO = PIN_SPI_MISO ;
static const uint8_t SCK  = PIN_SPI_SCK ;

#define PIN_SPI1_MISO        (34u)
#define PIN_SPI1_MOSI        (33u)
#define PIN_SPI1_SCK         (32u)
#define PIN_SPI1_CS          (35u)
/*
 * Wire Interfaces
 */
#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (3u)
#define PIN_WIRE_SCL         (4u)

/*
 * On-board SPI Flash
 */
#define EXTERNAL_FLASH_USE_SPI   SPI1
#define EXTERNAL_FLASH_USE_CS    PIN_SPI1_CS
/*
 * Also needed here are blank declarations of QSPI pins for the Adafruit
 * libraries to build properly. This way you can still create another instance
 * of the driver library to connect to another external QSPI chip.
 */
#define PIN_QSPI_SCK             -1
#define PIN_QSPI_CS              -1
#define PIN_QSPI_IO0             -1
#define PIN_QSPI_IO1             -1
#define PIN_QSPI_IO2             -1
#define PIN_QSPI_IO3             -1

/*
 * On board on/off controllable LDO
 */
#define PIN_LDO_CONTROL      (31u)

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#endif
