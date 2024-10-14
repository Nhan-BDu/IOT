#include <math.h>

const int x_out = A1; /* kết nối chân x_out của module với A1 của board UNO */
const int y_out = A2; /* kết nối chân y_out của module với A2 của board UNO */
const int z_out = A3; /* kết nối chân z_out của module với A3 của board UNO */

void setup() {
  Serial.begin(9600); 
}

void loop() {
  int x_adc_value, y_adc_value, z_adc_value; 
  double x_g_value, y_g_value, z_g_value;
  double roll, pitch;

  // Đọc giá trị ADC từ các trục X, Y và Z
  x_adc_value = analogRead(x_out); /* Giá trị số từ chân x_out */
  y_adc_value = analogRead(y_out); /* Giá trị số từ chân y_out */
  z_adc_value = analogRead(z_out); /* Giá trị số từ chân z_out */
  
  //In giá trị thô ra màn hình Serial
  Serial.print("X = ");
  Serial.print(x_adc_value);
  Serial.print("\t\t");
  Serial.print("Y = ");
  Serial.print(y_adc_value);
  Serial.print("\t\t");
  Serial.print("Z = ");
  Serial.print(z_adc_value);
  Serial.print("\t\t");

  // Chuyển đổi giá trị ADC sang đơn vị gia tốc G
  x_g_value = ( ( ( (double)(x_adc_value * 5)/1024) - 1.65 ) / 0.330 ); /* Gia tốc theo trục X */
  y_g_value = ( ( ( (double)(y_adc_value * 5)/1024) - 1.65 ) / 0.330 ); /* Gia tốc theo trục Y */
  z_g_value = ( ( ( (double)(z_adc_value * 5)/1024) - 1.80 ) / 0.330 ); /* Gia tốc theo trục Z */

  // Tính toán Roll và Pitch
  roll = ( (atan2(y_g_value, z_g_value) * 180) / 3.14 ) + 180; /* Công thức tính Roll */
  pitch = ( (atan2(z_g_value, x_g_value) * 180) / 3.14 ) + 180; /* Công thức tính Pitch */

  //In giá trị Roll và Pitch ra màn hình Serial
  Serial.print("Roll = ");
  Serial.print(roll);
  Serial.print("\t");
  Serial.print("Pitch = ");
  Serial.print(pitch);
  Serial.print("\n\n");

  // Kiểm tra và thông báo tư thế của người dùng
  kiemTraTuThe(roll, pitch);

  delay(1000); // Đợi 1 giây trước khi lặp lại
}

// Hàm kiểm tra tư thế người dùng dựa trên giá trị Roll và Pitch
void kiemTraTuThe(double roll, double pitch) {
  // Kiểm tra Roll để xác định tư thế
  if (roll > 150 && roll < 210) {
    Serial.println("Người dùng đang nằm sấp.");
  } else if (roll > 330 || roll < 30) {
    Serial.println("Người dùng đang nằm ngửa.");
  } else if (roll > 60 && roll < 120) {
    Serial.println("Người dùng đang nằm nghiêng.");
  }

  // Kiểm tra Pitch để xác định tư thế
  if (pitch > 150 && pitch < 210) {
    Serial.println("Người dùng đang đứng hoặc ngồi thẳng.");
  } else if (pitch > 60 && pitch < 120) {
    Serial.println("Người dùng đang nằm.");
  }
}
