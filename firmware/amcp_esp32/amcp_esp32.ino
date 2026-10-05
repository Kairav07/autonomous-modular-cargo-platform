/*
 * Autonomous Modular Cargo Platform
 * AMCP ESP32 Firmware
 *
 * This file will contain:
 * - IR line following
 * - Motor control
 * - Ultrasonic obstacle detection
 * - Junction detection
 * - Cargo navigation logic
 * - Serial telemetry
 */

#include <Arduino.h>


void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println("AMCP ESP32 Starting...");

}


void loop() {

  /*
   * Temporary telemetry for dashboard testing.
   *
   * Replace this with actual sensor data later.
   */

  Serial.println(
    "{\"line\":\"00100\","
    "\"action\":\"FORWARD\","
    "\"distance_cm\":50,"
    "\"obstacle\":false,"
    "\"junction\":false,"
    "\"path_detected\":true}"
  );

  delay(500);

}
