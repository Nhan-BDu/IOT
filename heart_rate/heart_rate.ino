#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat detected!");
}

void setup() {
  Serial.begin(115200);
  Serial.print("Initializing pulse oximeter...");

  // Initialize the Pulse Oximeter
  if (!pox.begin()) {
    Serial.println("Failed to initialize pulse oximeter! Please check your connections.");
    while (1); // Halt the program
  }

  // Set the callback for beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  Serial.println("Pulse oximeter initialized successfully!");
}

void loop() {
  pox.update();
  
  // Print heart rate and SpO2 values every second
  if (millis() - tsLastReport > 1000) {
    tsLastReport = millis();

    float heartRate = pox.getHeartRate();
    float spO2 = pox.getSpO2();

    if (heartRate > 0) {
      Serial.print("Heart rate: ");
      Serial.print(heartRate);
      Serial.print(" bpm / SpO2: ");
      Serial.print(spO2);
      Serial.println(" %");
    } else {
      Serial.println("Heart rate data not available.");
    }
  }
}
