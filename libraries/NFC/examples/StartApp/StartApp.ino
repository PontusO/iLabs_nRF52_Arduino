/*
  StartApp.ino
  
  Written by Chiara Ruggeri (chiara@arduino.org)
  Modified for iLabs by P. Oldberg (pontus@ilabs.se)
  This code runs on the Connectivity 840 LTE board by iLabs
  
  This example for the Arduino Primo board shows how to use
  NFC library.
  It sets an app message specifying the package name (for Android)
  and the application ID (for Windows phone), then starts the
  module, so that when a device with NFC is near to the board
  it will try to open the application (if present) or will
  look for the app in the store. Finally it registers a callback
  function that will be called any time an NFC field is detected
  (it means that a device is near).

  This example code is in the public domain.
  
*/

#include <bluefruit.h>
#include <NFC.h>

// specify the package name for Windows and Android phones and insert the EOL character at the end '\0'
static const char android_package_name[] = {'n', 'o', '.', 'n', 'o', 'r', 'd', 'i', 'c', 's',
                                               'e', 'm', 'i', '.', 'a', 'n', 'd', 'r', 'o', 'i',
                                               'd', '.', 'n', 'r', 'f', 't', 'o', 'o', 'l', 'b',
                                               'o', 'x', '\0'};

static const char windows_application_id[] = {'{', 'e', '1', '2', 'd', '2', 'd', 'a', '7', '-',
                                                 '4', '8', '8', '5', '-', '4', '0', '0', 'f', '-',
                                                 'b', 'c', 'd', '4', '-', '6', 'c', 'b', 'd', '5',
                                                 'b', '8', 'c', 'f', '6', '2', 'c', '}', '\0'};

void setup() {
  // The NFC stack uses SoftDevice clock arbitration internally; the SD must
  // be enabled before NFC.start(). Bluefruit.begin() is the supported way
  // to bring it up on this BSP — call it even if you don't use BLE.
  Bluefruit.begin();

  Serial.begin(9600);
  NFC.begin();
  NFC.setAPPmessage(android_package_name, windows_application_id);
  NFC.start();
  NFC.registerCallback(myFunction);
}


void loop() {
}

void myFunction(){
  Serial.println("A user viewed the application");
}