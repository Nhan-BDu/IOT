#include <SPI.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS 1000

PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat detected!");
}
void setup() {
  Serial.begin(9600);
  SPI.begin();  // Khởi động SPI
  
  // pox.begin();
  // if (!pox.begin()) {
  //   Serial.println("Failed to initialize pulse oximeter! Please check your connections.");
  //   while (1); // Halt the program
  // }

  // // Set the callback for beat detection
  // pox.setOnBeatDetectedCallback(onBeatDetected);
}

void heart_rate() {
    pox.update();
    if (millis() - tsLastReport > 1000) {
      tsLastReport = millis();
      float heartRate = pox.getHeartRate();
      float spO2 = pox.getSpO2();
      if (heartRate > 0) {  
          Serial.print("Heart rate: ");
          Serial.print(heartRate);
          Serial.print(" bpm / SpO2: ");
          Serial.print(spO2);
          Serial.println("%");

          // Truyền dữ liệu qua SPI
          sendDataToESP(heartRate, spO2);
      } else {
          Serial.println("Heart rate data not available.");
      }
    }
}

void sendDataToESP(float heartRate, float spO2) {
  // Chuyển các giá trị sang byte để truyền qua SPI
  byte heartRateByte = (byte)heartRate;
  byte spO2Byte = (byte)spO2;

  // Kéo chân SS xuống LOW để bắt đầu giao tiếp
  digitalWrite(10, LOW);

  // Truyền dữ liệu heart rate và SpO2
  SPI.transfer(heartRateByte);
  SPI.transfer(spO2Byte);

  // Kết thúc giao tiếp
  digitalWrite(10, HIGH);
}

void loop() {
    // heart_rate();  // Gọi hàm đo nhịp tim và truyền dữ liệu
    sendDataToESP(1.1,1.2);
    Serial.println("send");
    delay(500);
    
}
