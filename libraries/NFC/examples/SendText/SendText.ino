/*
  SendText.ino
  
  Written by Chiara Ruggeri (chiara@arduino.org)
  Modified for iLabs by P. Oldberg (pontus@ilabs.se)
  This code runs on the Connectivity 840 LTE board by iLabs
  
  This example for the Arduino Primo board shows how to use
  NFC library.
  It sets a text message specifying the language code, then
  starts the module, so that when a device with NFC is near
  to the board the message "Hello World!" will be sent.

  This example code is in the public domain.
  
*/

#include <bluefruit.h>
#include <NFC.h>


void setup() {
  // The NFC stack uses SoftDevice clock arbitration internally; the SD must
  // be enabled before NFC.start(). Bluefruit.begin() is the supported way
  // to bring it up on this BSP — call it even if you don't use BLE.
  Bluefruit.begin();

  NFC.begin();
  // set the NFC message as first parameter and the language code as second
  NFC.setTXTmessage("Hello World!", "en");
  // start the NFC module
  NFC.start();
}


void loop() {
}


