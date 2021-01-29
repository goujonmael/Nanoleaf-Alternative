/*
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */



// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProLight.h"

#define WIFI_SSID         "Miguel"    
#define WIFI_PASS         "Banane26"
#define APP_KEY           "85f5bb93-8161-43e5-afcf-cf817fbac696"             // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "227078b3-3960-4099-820e-1ca86c04888e-d57f6ebc-b96a-4cc1-b58d-c84c6bad550d"          // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define LIGHT_ID          "600c68d5905d812f2410e4b2"           // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE         9600                      // Change baudrate to your need for serial log

#define BLUE_PIN          14                        // PIN for BLUE Mosfet  - change this to your need (D5 = GPIO14 on ESP8266)
#define RED_PIN           13                        // PIN for RED Mosfet   - change this to your need (D6 = GPIO12 on ESP8266)
#define GREEN_PIN         12                        // PIN for GREEN Mosfet - change this to your need (D7 = GPIO13 on ESP8266)
#define FADESPEED 5

struct {                                            // Stores current device state with following initial values:
  bool powerState = false;                          // initial state is off
  int brightness = 100;                             // initial brightness is set to 100
  struct {
    byte r = 255;                                   // color
    byte g = 255;                                   // is set 
    byte b = 255;                                   // to white
  } color;
} device_state; 

// setStripe: sets the mosfets values corresponding to values stored in device_state
void setStripe() {
  int rValue = map(device_state.color.r * device_state.brightness, 0, 255 * 100, 0, 1023);  // calculate red value and map between 0 and 1023 for analogWrite
  int gValue = map(device_state.color.g * device_state.brightness, 0, 255 * 100, 0, 1023);  // calculate green value and map between 0 and 1023 for analogWrite
  int bValue = map(device_state.color.b * device_state.brightness, 0, 255 * 100, 0, 1023);  // calculate blue value and map between 0 and 1023 for analogWrite

  if (device_state.powerState == false) {           // turn off?
    digitalWrite(RED_PIN,   LOW);                   // set
    digitalWrite(GREEN_PIN, LOW);                   // mosfets
    digitalWrite(BLUE_PIN,  LOW);                   // low
  } 
  else {
    if (device_state.color.r==255 && device_state.color.g==191 && device_state.color.b==204){
      fade();
    }
    else {
    analogWrite(RED_PIN,   rValue);                 // write red value to pin
    analogWrite(GREEN_PIN, gValue);                 // write green value to pin
    analogWrite(BLUE_PIN,  bValue);                 // write blue value to pin
  }}
}

bool onPowerState(const String &deviceId, bool &state) {
  device_state.powerState = state;                  // store the new power state
  setStripe();                                      // update the mosfets
  return true;
}

bool onBrightness(const String &deviceId, int &brightness) {
  device_state.brightness = brightness;             // store new brightness level
  setStripe();                                      // update the mosfets
  return true;
}

bool onAdjustBrightness(const String &deviceId, int &brightnessDelta) {
  device_state.brightness += brightnessDelta;       // calculate and store new absolute brightness
  brightnessDelta = device_state.brightness;        // return absolute brightness
  setStripe();                                      // update the mosfets
  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  device_state.color.r = r;                         // store new red value
  device_state.color.g = g;                         // store new green value
  device_state.color.b = b;                         // store new blue value
  setStripe();                                      // update the mosfets
  return true;
}

void setup() {
  pinMode(RED_PIN,   OUTPUT);                       // set red-mosfet pin as output
  pinMode(GREEN_PIN, OUTPUT);                       // set green-mosfet pin as output
  pinMode(BLUE_PIN,  OUTPUT);                       // set blue-mosfet pin as output

  Serial.begin(BAUD_RATE);                          // setup serial 

  WiFi.begin(WIFI_SSID, WIFI_PASS);                 // connect wifi

  SinricProLight &myLight = SinricPro[LIGHT_ID];    // create light device
  myLight.onPowerState(onPowerState);               // set PowerState callback
  myLight.onBrightness(onBrightness);               // set Brightness callback
  myLight.onAdjustBrightness(onAdjustBrightness);   // set AdjustBrightness callback
  myLight.onColor(onColor);                         // set Color callback

  SinricPro.begin(APP_KEY, APP_SECRET);             // start SinricPro 
}

void fade(){
  for (int i = 0; i < 10; i = i + 1) {
  int r, g, b;
 
  // fade from blue to violet
  for (r = 0; r < 256; r++) { 
    analogWrite(RED_PIN, r);
    delay(FADESPEED);
  } 
  // fade from violet to red
  for (b = 255; b > 0; b--) { 
    analogWrite(BLUE_PIN, b);
    delay(FADESPEED);
  } 
  // fade from red to yellow
  for (g = 0; g < 256; g++) { 
    analogWrite(GREEN_PIN, g);
    delay(FADESPEED);
  } 
  // fade from yellow to green
  for (r = 255; r > 0; r--) { 
    analogWrite(RED_PIN, r);
    delay(FADESPEED);
  } 
  // fade from green to teal
  for (b = 0; b < 256; b++) { 
    analogWrite(BLUE_PIN, b);
    delay(FADESPEED);
  } 
  // fade from teal to blue
  for (g = 255; g > 0; g--) { 
    analogWrite(GREEN_PIN, g);
    delay(FADESPEED);
  }
  } 
}

void loop() {
  SinricPro.handle();                               // handle SinricPro communication
}
