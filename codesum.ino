#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MLX90614.h>
#include <math.h>
#define REPORTING_PERIOD_MS 1000


Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseOximeter pox;
uint32_t tsLastReport = 0;
int sensorPin = A0;  // Chân analog để đọc dữ liệu từ cảm biến
int status = 0;
float o2 = 0;
float hr = 0;
float temp = 0;

// #define WIFI_SSID "#" 
// #define WIFI_PASSWORD "#"
const int x_out = A1; 
const int y_out = A2;
const int z_out = A3;

void onBeatDetected() {
    Serial.print("");
}

void setup() {
  Serial.begin(9600);
  pox.begin();
  mlx.begin();  
  if (!pox.begin()) {
    // Serial.println("Failed to initialize pulse oximeter! Please check your connections.");
    while (1); // Halt the program
  }
  // Set the callback for beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  Serial.println("Pulse oximeter initialized successfully!");
  //pox.setOnBeatDetectedCallback(onBeatDetected);
  float heartRate = pox.getHeartRate();
  float spO2 = pox.getSpO2();
}

void heart_rate() {
  pox.update();
  if (millis() - tsLastReport > 1000) {
    tsLastReport = millis();
    float heartRate = pox.getHeartRate();
    float spO2 = pox.getSpO2();
    if (heartRate > 0) {  
      hr = heartRate;
      o2 = spO2;
    } 
  }
}

// Hàm kiểm tra tư thế
void kiemTraTuThe(double roll, double pitch) {
    if (roll > 150 && roll < 210) {
        status = 1;
        delay(10);
    }  else if (roll > 330 || roll < 30) {
        status = 1;
        delay(10);
    } else if (roll > 60 && roll < 120) {
        status = 1;
        delay(10);
    } else if (pitch > 60 && pitch < 120) {
        status = 1;
        delay(10);
    }
    if (pitch > 150 && pitch < 210) {
        status = 0;
        delay(10);
    } 
}  


// Hàm đo gia tốc
void alocimetter() {
    int x_adc_value, y_adc_value, z_adc_value; 
    double x_g_value, y_g_value, z_g_value;
    double roll, pitch;

    x_adc_value = analogRead(x_out);
    y_adc_value = analogRead(y_out);
    z_adc_value = analogRead(z_out);

    // Serial.print("X = ");
    // Serial.print(x_adc_value);
    // Serial.print("\t\t");
    // Serial.print("Y = ");
    // Serial.print(y_adc_value);
    // Serial.print("\t\t");
    // Serial.print("Z = ");
    // Serial.print(z_adc_value);
    // Serial.print("\t\t");

    x_g_value = ( ((double)(x_adc_value * 5) / 1024) - 1.65 ) / 0.330;
    y_g_value = ( ((double)(y_adc_value * 5) / 1024) - 1.65 ) / 0.330;
    z_g_value = ( ((double)(z_adc_value * 5) / 1024) - 1.80 ) / 0.330;

    roll = (atan2(y_g_value, z_g_value) * 180) / 3.14 + 180;
    pitch = (atan2(z_g_value, x_g_value) * 180) / 3.14 + 180;

    // Serial.print("Roll = ");
    // Serial.print(roll);
    // Serial.print("\t");
    // Serial.print("Pitch = ");
    // Serial.print(pitch);
    // Serial.print("\n\n");

    kiemTraTuThe(roll, pitch);
}

void loop() {
    // int check = 1;
    // if (check == 1){
    //   heart_rate();
    //   check = 2;
    // }
    // else if (check == 2){
    //   alocimetter();
    //   check = 1;
    // }
    
    // Serial.println("kkkk");
    // delay(1000);
    alocimetter();
    heart_rate();
    temp = mlx.readObjectTempC();
    Serial.println(temp);
    if (o2 != 0 && hr != 0 && temp != 0 && status != 0){
      Serial.print(o2);
      Serial.print(" ");
      Serial.print(hr);
      Serial.print(" ");
      Serial.print(temp);
      Serial.print(" ");
      Serial.print(status);
      Serial.println();
      o2 = 0 ;
      hr = 0;
      temp = 0;
      status = 0;
    }
}

