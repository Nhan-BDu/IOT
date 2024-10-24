#include <SPI.h>

volatile byte receivedHeartRate;
volatile byte receivedSpO2;

void setup() {
  Serial.begin(9600);
  
  // Khởi tạo SPI với ESP8266 làm Slave
  pinMode(15, INPUT_PULLUP);  // SS ở chân D8
  SPI.begin();
  attachInterrupt(digitalPinToInterrupt(15), receiveDataFromMaster, FALLING);  // Gắn ngắt cho chân SS
}

void receiveDataFromMaster() {
  // Nhận dữ liệu từ Arduino Uno
  receivedHeartRate = SPI.transfer(0x00);  // Nhận nhịp tim
  receivedSpO2 = SPI.transfer(0x00);       // Nhận SpO2
  
  // In dữ liệu ra Serial để kiểm tra
  Serial.print("Heart rate received: ");
  Serial.println(receivedHeartRate);
  
  Serial.print("SpO2 received: ");
  Serial.println(receivedSpO2);
}

void loop() {
  // Hàm loop trống vì dữ liệu được nhận qua ngắt SPI
}
