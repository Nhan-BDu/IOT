// Định nghĩa chân kết nối của cảm biến LM35
int sensorPin = A0;  // Chân analog để đọc dữ liệu từ cảm biến
float temperature;   // Biến lưu nhiệt độ

void setup() {
  // Khởi động Serial Monitor
  Serial.begin(9600);
}

void loop() {
  // Đọc giá trị từ chân analog
  int sensorValue = analogRead(sensorPin);

  // Tính toán nhiệt độ từ giá trị cảm biến
  temperature = (sensorValue * 5.0 * 100) / 1023;

  // Kiểm tra nếu nhiệt độ nằm trong khoảng từ 35 đến 40 độ C
  if (temperature >= 35 && temperature <= 40) {
    // Hiển thị kết quả nhiệt độ trên Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
  }

  // Dừng một chút trước khi đo lại
  delay(100);
}
