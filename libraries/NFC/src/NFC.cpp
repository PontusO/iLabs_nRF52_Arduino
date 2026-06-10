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


#include "NFC.h"
#include "nrf_sdm.h"


static void nfc_callback(void *context, NfcEvent event, const char *data, size_t dataLength)
{
	(void) context;
	(void) event;
	(void) data;
	(void) dataLength;
	NFC.onService();
}


/* The encoded NDEF message lives in a fixed 256-byte buffer that's the
 * payload pointed at by nfcSetPayload(). Each setXxxmessage() zeroes it
 * before encoding so leftover bytes from a previous (longer) message can't
 * leak into the next broadcast. */
static uint8_t ndef_msg_buf[256];


bool NFCClass::begin(void)
{
	if (_initialized) {
		return true;
	}
	nfcSetup(nfc_callback, NULL);
	_initialized = true;
	return true;
}


bool NFCClass::setTXTmessage(const char TXTMessage[], const char language[])
{
	if (!_initialized) return false;

	memset(ndef_msg_buf, 0, sizeof(ndef_msg_buf));
	uint32_t len   = sizeof(ndef_msg_buf);
	uint8_t  sizeM = strlen(TXTMessage);
	uint8_t  sizeL = strlen(language);

	NFC_NDEF_MSG_DEF(welcome_msg, 1);
	NFC_NDEF_TEXT_RECORD_DESC_DEF(en_text_rec,
	                              UTF_8,
	                              (uint8_t *)language,
	                              sizeL,
	                              (uint8_t *)TXTMessage,
	                              sizeM);
	if (nfc_ndef_msg_record_add(&NFC_NDEF_MSG(welcome_msg),
	                            &NFC_NDEF_TEXT_RECORD_DESC(en_text_rec)) != NRF_SUCCESS) {
		return false;
	}

	if (nfc_ndef_msg_encode(&NFC_NDEF_MSG(welcome_msg),
	                        ndef_msg_buf,
	                        &len) != NRF_SUCCESS) {
		return false;
	}

	nfcSetPayload((char *) ndef_msg_buf, len);
	return true;
}


bool NFCClass::setURImessage(const char URL[], nfc_uri_id_t type)
{
	if (!_initialized) return false;

	memset(ndef_msg_buf, 0, sizeof(ndef_msg_buf));
	uint32_t len  = sizeof(ndef_msg_buf);
	uint8_t  size = strlen(URL);

	if (nfc_uri_msg_encode(type,
	                       (uint8_t *) URL,
	                       size,
	                       ndef_msg_buf,
	                       &len) != NRF_SUCCESS) {
		return false;
	}

	nfcSetPayload((char *) ndef_msg_buf, len);
	return true;
}


bool NFCClass::setAPPmessage(const char android_app[], const char windows_app[])
{
	if (!_initialized) return false;

	memset(ndef_msg_buf, 0, sizeof(ndef_msg_buf));
	uint32_t len   = sizeof(ndef_msg_buf);
	uint8_t  sizeA = strlen(android_app);
	uint8_t  sizeW = strlen(windows_app);

	if (nfc_launchapp_msg_encode((uint8_t *)android_app,
	                             sizeA,
	                             (uint8_t *)windows_app,
	                             sizeW,
	                             ndef_msg_buf,
	                             &len) != NRF_SUCCESS) {
		return false;
	}

	nfcSetPayload((char *) ndef_msg_buf, len);
	return true;
}


bool NFCClass::setOobPairingKey(void)
{
	if (!_initialized) return false;

	uint8_t sd_enabled = 0;
	if (sd_softdevice_is_enabled(&sd_enabled) != NRF_SUCCESS || !sd_enabled) {
		/* OOB pairing key generation depends on the SoftDevice RNG service.
		 * Without an enabled SD the sd_rand_* calls would fail silently and
		 * the fallback loop below would seed the key with 0x00..0x0F — a
		 * trivially-guessable "secret". Refuse rather than ship that. */
		return false;
	}

	uint8_t key[16];
	uint8_t random_values_length = 0;
	uint8_t generated             = 0;
	static ble_advdata_tk_value_t oob_auth_key;

	if (sd_rand_application_pool_capacity_get(&random_values_length) != NRF_SUCCESS) {
		return false;
	}
	if (random_values_length > 16) random_values_length = 16;

	/* Wait until the pool has enough entropy. Bounded by SD's RNG; in
	 * practice tens of microseconds. */
	uint32_t spin = 0;
	do {
		if (sd_rand_application_bytes_available_get(&generated) != NRF_SUCCESS) {
			return false;
		}
		if (++spin > 100000) return false; /* belt-and-braces watchdog */
	} while (generated < random_values_length);

	if (sd_rand_application_vector_get(key, random_values_length) != NRF_SUCCESS) {
		return false;
	}

	/* If the SD couldn't give us a full 16 bytes, pad with a stable pattern.
	 * Documented limitation — caller can avoid by ensuring the RNG pool is
	 * sized appropriately in sd_softdevice_enable. */
	for (uint8_t i = random_values_length; i < 16; i++) {
		key[i] = i;
	}

	memcpy(oob_auth_key.tk, key, 16);

	memset(ndef_msg_buf, 0, sizeof(ndef_msg_buf));
	uint32_t len = sizeof(ndef_msg_buf);
	if (nfc_ble_pair_default_msg_encode(NFC_BLE_PAIR_MSG_FULL,
	                                    &oob_auth_key,
	                                    ndef_msg_buf,
	                                    &len) != NRF_SUCCESS) {
		return false;
	}

	nfcSetPayload((char *) ndef_msg_buf, len);
	return true;
}


bool NFCClass::start(void)
{
	if (!_initialized) return false;
	nfcStartEmulation();
	return true;
}


void NFCClass::stop(void)
{
	nfcStopEmulation();
}


void NFCClass::registerCallback(void(*function)(void))
{
	Callback = function;
}


void NFCClass::onService(void)
{
	if (Callback) {
		Callback();
	}
}


NFCClass NFC;
