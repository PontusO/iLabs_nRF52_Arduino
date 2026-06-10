/*
  NFC class for nRF52.
  Written by Chiara Ruggeri (chiara@arduino.org)

  Copyright (c) 2016 Arduino.  All right reserved.

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

  Enjoy!
*/


#ifndef NFC_h
#define NFC_h

#include "Arduino.h"
#include "nrf_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "nfc/nfc_t2t_lib.h"
#include "nfc/nfc_uri_msg.h"
#include "nfc/nfc_text_rec.h"
#include "nfc/nfc_launchapp_msg.h"
#include "nfc/nfc_ble_pair_msg.h"

#ifdef __cplusplus
}
#endif


class NFCClass{

	public:

		/**
		 * @brief Initialise the NFC stack. Must be called once before any
		 *        setXxxmessage() / start() call. Idempotent — repeated
		 *        calls are no-ops.
		 *
		 *        The NFC stack uses SoftDevice clock arbitration internally
		 *        (HFCLK is requested via sd_clock_hfclk_request when a tag
		 *        field is detected). Make sure the SoftDevice is enabled
		 *        before calling NFC.start() — on this BSP, that's done with
		 *        Bluefruit.begin().
		 *
		 * @return true on success.
		 */
		bool begin(void);


		/**
		 * @brief Specify a text message that will pop up on a device when
		 *        it is near to the board.
		 * @param TXTMessage  array of char containing the message
		 * @param language    array of char containing the message's language code
		 * @return true if the NDEF message was encoded successfully; false
		 *         if begin() was not called or the encoded message would
		 *         exceed the 256-byte payload buffer.
		 */
		bool setTXTmessage(const char TXTMessage[], const char language[]);


		/**
		 * @brief Specify an URI message that will pop up on a device when
		 *        it is near to the board.
		 * @param URL   address to the resource to reach
		 * @param type  type of the URI message (see documentation for details)
		 * @return true if the NDEF message was encoded successfully; false
		 *         if begin() was not called or the encoded message would
		 *         exceed the 256-byte payload buffer.
		 */
		bool setURImessage(const char URL[], nfc_uri_id_t type);


		/**
		 * @brief Specify an application that will be launched on a device
		 *        when it is near to the board.
		 * @param android_app  package of the Android application
		 * @param windows_app  ID of the Windows application
		 * @return true if the NDEF message was encoded successfully; false
		 *         if begin() was not called or the encoded message would
		 *         exceed the 256-byte payload buffer.
		 */
		bool setAPPmessage(const char android_app[], const char windows_app[]);


		/**
		 * @brief Set a message for pairing the board with another BLE device.
		 *        This function is part of the OOB bond procedure and requires
		 *        the SoftDevice to be enabled (Bluefruit.begin() called first).
		 * @return true on success; false if begin() was not called, the
		 *         SoftDevice is not enabled, sd_rand_* failed to produce a
		 *         key, or the encoded message would not fit.
		 */
		bool setOobPairingKey(void);


		/**
		 * @brief Start the NFC module. begin() must have been called first.
		 * @return true on success; false if begin() was not called.
		 */
		bool start(void);


		/**
		 * @brief Stop the NFC module.
		 */
		void stop(void);


		/**
		 * @brief Attach a function to the "field detected" event.
		 * @param function  function to be called when the event happens
		 */
		void registerCallback(void(*function)(void));


		/**
		 * @brief Service function called by ISR. Not for user code.
		 */
		void onService(void);

	private:
		void (*Callback)(void) = nullptr;
		bool _initialized = false;
};

extern NFCClass NFC;

#endif //NFC_h
