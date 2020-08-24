/**
 * Simple server compliant with Mozilla's proposed WoT API
 * Originally based on the HelloServer example
 * Tested on ESP8266, ESP32, Arduino boards with WINC1500 modules (shields or
 * MKR1000)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define ARDUINOJSON_USE_LONG_LONG 1

#include <Arduino.h>
#include <Thing.h>
#include <WebThingAdapter.h>
#include "config.h"

#ifdef ESP32
#include <analogWrite.h>
#endif

ThingActionObject *action_generator(DynamicJsonDocument *);

WebThingAdapter *adapter;

const char *fountainTypes[] = {"OnOffSwitch", "EnergyMonitor", nullptr};
ThingDevice fountain(AIGOR_THING_ID, AIGOR_THING_NAME, fountainTypes);
ThingProperty fountainOn("on", "Whether the fountain is turned on", BOOLEAN, "OnOffProperty");
bool lastOn = false;

// TODO
//
// ThingProperty fountainBatteryVoltage("batteryVoltage", "Battery Voltage", NUMBER, "Voltage");
//
//  void updateVoltage() {
//    int ADC_VALUE = analogRead(relayPin);
//    Serial.println("voltage ="+ADC_VALUE);
//    ThingDataValue voltageValue = fountainBatteryVoltage.getValue();
//    voltageValue.number = (ADC_VALUE * 3.7 ) / (4095);
//    fountainBatteryVoltage.setValue(voltageValue);
//  }
//

void updateFountain() {
  bool on = fountainOn.getValue().boolean;
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  if(lastOn == on) {
    return;
  }
  Serial.print("fountain ");
  Serial.println(on);
  lastOn = on;
}

void setup(void) {
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);
  Serial.begin(115200);
  Serial.println("");
  Serial.printf("Connecting to \"%s\"...",AIGOR_WIFI_SSID);
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(AIGOR_WIFI_SSID, AIGOR_WIFI_PASS);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("OK");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  adapter = new WebThingAdapter("fountain", WiFi.localIP());
  fountain.description = "A web connected fountain";
  fountainOn.title = "On/Off";
  fountain.addProperty(&fountainOn);
  // fountain.addProperty(&fountainBatteryVoltage);
  adapter->addDevice(&fountain);
  adapter->begin();

  Serial.println("HTTP server started");
  Serial.println("http://" + WiFi.localIP().toString() + "/things/" + fountain.id);
  Serial.println("");

  // set initial values
  ThingPropertyValue initialOn = {.boolean = false};
  fountainOn.setValue(initialOn);
  (void)fountainOn.changedValueOrNull();

  digitalWrite(PIN_RELAY, LOW);
}

void loop(void) {
  adapter->update();
  updateFountain();
}
