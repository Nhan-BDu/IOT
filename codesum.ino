#include <Wire.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define MAX30100_ADDR 0x57  
#define MLX90614_ADDR 0x5A  
#define THRESHOLD 500    
#define FILTER_SIZE 4 // Kích thước bộ lọc trung bình

const char* ssid = "Wifi"; 
const char* password = "khongcomatkhau";
const char* mqtt_server = "rabbitmq-pub.education.wise-paas.com";  // URL của máy chủ MQTT của Datahub
const int mqtt_port = 1883;
// Thông tin tài khoản MQTT
const char* mqtt_username = "7LeD0ox8MZeY:VoebJJ454Ped";
const char* mqtt_password = "mD4jM9RIMSGYpygEzbja";

WiFiClient espClient;
PubSubClient client(espClient);

const int x_out = 35; 
const int y_out = 34;
const int z_out = 39;

unsigned long lastBeat = 0;
const int filterWindowSize = 5;
float irBuffer[filterWindowSize] = {0};
float redBuffer[filterWindowSize] = {0};
int bufferIndex = 0;

int status = 0;


void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  Wire.begin();  
  Serial.println("Starting I2C communication...");

  initMAX30100(); 
  Serial.println("Initialization complete.");
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnect() {
  // Thử kết nối lại nếu mất kết nối
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient", mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}
void send(String name, String data ){
  String payload = "{\"d\": {\"DeviceA\": {\"" + name + "\": \"" + data + "\", \"ts\": \"2024-10-24T07:13:32.429337Z\"}}}";
  String topic = "/wisepaas/scada/8bab95f7-be6d-46c1-b55b-cfbe9e089c6a/data";
  client.publish(topic.c_str(), payload.c_str());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  calculateHeartRateAndSpO2();
  readMLX90614();
  alocimetter();
  delay(1000); 
}

void initMAX30100() {
  Wire.beginTransmission(MAX30100_ADDR);
  Wire.write(0x06); // Mode Configuration
  Wire.write(0x03); // SpO2 mode
  Wire.endTransmission();

  Wire.beginTransmission(MAX30100_ADDR);
  Wire.write(0x07); // SpO2 Configuration
  Wire.write(0x2F); // Set resolution and sample rate
  Wire.endTransmission();

  Wire.beginTransmission(MAX30100_ADDR);
  Wire.write(0x09); // LED Configuration
  Wire.write(0x24); // Adjust LED brightness if needed
  Wire.endTransmission();

  Serial.println("MAX30100 initialized.");
}

void calculateHeartRateAndSpO2() {
  Wire.beginTransmission(MAX30100_ADDR);
  Wire.write(0x05); // FIFO Data register
  Wire.endTransmission(false);
  Wire.requestFrom(MAX30100_ADDR, 4);

  if (Wire.available() == 4) {
    uint8_t irHigh = Wire.read();
    uint8_t irLow = Wire.read();
    uint8_t redHigh = Wire.read();
    uint8_t redLow = Wire.read();

    int irValue = (irHigh << 8) | irLow;
    int redValue = (redHigh << 8) | redLow;

    // Kiểm tra xem tín hiệu IR có đủ lớn (cảm biến đang được che phủ)
    if (irValue < THRESHOLD) {
      Serial.println("No finger detected. Waiting...");
      return;  // Thoát hàm nếu không có ngón tay
    }

    // Lọc tín hiệu bằng bộ lọc trung bình trượt
    irBuffer[bufferIndex] = irValue;
    redBuffer[bufferIndex] = redValue;
    bufferIndex = (bufferIndex + 1) % filterWindowSize;

    float irFiltered = 0;
    float redFiltered = 0;
    for (int i = 0; i < filterWindowSize; i++) {
      irFiltered += irBuffer[i];
      redFiltered += redBuffer[i];
    }
    irFiltered /= filterWindowSize;
    redFiltered /= filterWindowSize;

    // Tính toán SpO2 dựa trên tín hiệu lọc
    float dcIR = irFiltered * 0.8;
    float acIR = irFiltered - dcIR;
    float dcRed = redFiltered * 0.8;
    float acRed = redFiltered - dcRed;
    float ratio = (acRed / dcRed) / (acIR / dcIR);
    float spO2 = 115 - 25 * ratio;

    // Hiển thị SpO2 nếu trong phạm vi hợp lệ
    if (spO2 > 0 && spO2 <= 100) {
      Serial.print("Estimated SpO2: ");
      Serial.print(spO2);
      Serial.println(" %");
      send("SpO2", String(spO2));
    } else {
      Serial.println("SpO2 out of range.");
    }

    // Phát hiện nhịp tim dựa trên ngưỡng tín hiệu AC
    unsigned long currentTime = millis();
    if (acIR > THRESHOLD) {
      unsigned long timeBetweenBeats = currentTime - lastBeat;
      if (timeBetweenBeats > 150) { // Đảm bảo không ghi đè khi có nhiễu
        int heartRate = 96000 / timeBetweenBeats;
        Serial.print("Heart Rate: ");
        Serial.print(heartRate);
        Serial.println(" bpm");
        lastBeat = currentTime;
        send("HeartRate", String(heartRate));
      }
    }
  } else {
    Serial.println("Failed to read from MAX30100");
  }
}




void readMLX90614() {
  Wire.beginTransmission(MLX90614_ADDR);
  Wire.write(0x07);
  Wire.endTransmission(false);

  Wire.requestFrom(MLX90614_ADDR, 3);

  if (Wire.available() == 3) {
    uint8_t lowByte = Wire.read();
    uint8_t highByte = Wire.read();
    uint8_t pec = Wire.read(); 

    int tempData = (highByte << 8) | lowByte;
    float temperature = tempData * 0.0202 - 273.15;

    Serial.print("Object Temperature (MLX90614): ");
    Serial.print(temperature);
    Serial.println(" °C");
    send("Temperature",String(temperature));
  } else {
    Serial.println("Failed to read from MLX90614");
  }
}
void kiemTraTuThe(double roll, double pitch) {
    static double lastRoll = roll;
    static double lastPitch = pitch;
    static unsigned long lastChangeTime = millis();
    static unsigned long lastMoveTime = millis();

    double fallThreshold = 130.0;     // Ngưỡng cho trạng thái ngã
    double noiseThreshold = 3.0;      // Ngưỡng nhiễu cho thay đổi nhỏ
    unsigned long moveTimeout = 2000; // Thời gian tối đa cho phép không thay đổi để xác định trạng thái dừng

    unsigned long currentTime = millis();

    // Kiểm tra trạng thái "Nằm"
    if (pitch > 150 && pitch < 210) {
        status = 0;  // Nằm
        lastMoveTime = currentTime;  // Cập nhật thời gian lần cuối nằm
    }
    // Kiểm tra trạng thái "Ngã"
    else if (abs(roll - lastRoll) > fallThreshold || abs(pitch - lastPitch) > fallThreshold) {
        status = 2;  // Ngã
        lastMoveTime = currentTime;  // Cập nhật thời gian lần cuối ngã
    }
    // Kiểm tra trạng thái "Hoạt động"
    else if (abs(roll - lastRoll) > noiseThreshold || abs(pitch - lastPitch) > noiseThreshold) {
        status = 1;  // Hoạt động
        lastChangeTime = currentTime;
        lastMoveTime = currentTime;  // Cập nhật thời gian lần cuối chuyển động
    }
    // Kiểm tra nếu không có thay đổi trong một khoảng thời gian để quay lại trạng thái "Nằm"
    else if (currentTime - lastMoveTime > moveTimeout) {
        status = 0;  // Trạng thái trở về Nằm
    }

    // Cập nhật giá trị roll và pitch cho lần đọc tiếp theo
    lastRoll = roll;
    lastPitch = pitch;
}

// Function to read accelerometer data
void alocimetter() {
    int x_adc_value = analogRead(x_out);
    int y_adc_value = analogRead(y_out);
    int z_adc_value = analogRead(z_out);
    
    double x_g_value = ((double)(x_adc_value * 5) / 1024 - 1.65) / 0.330;
    double y_g_value = ((double)(y_adc_value * 5) / 1024 - 1.65) / 0.330;
    double z_g_value = ((double)(z_adc_value * 5) / 1024 - 1.80) / 0.330;

    double roll = (atan2(y_g_value, z_g_value) * 180) / M_PI + 180;
    double pitch = (atan2(z_g_value, x_g_value) * 180) / M_PI + 180;

    kiemTraTuThe(roll, pitch);
    
    // Hiển thị trạng thái
    switch(status) {
        case 0:
            Serial.println("Nằm");
            send("Accelerometer","lying");
            break;
        case 1:
            Serial.println("Hoạt động");
            send("Accelerometer","moving");
            break;
        case 2:
            Serial.println("Ngã");
            send("Accelerometer","falling");
            break;
        default:
            Serial.println("Không xác định");
            break;
    }
}
