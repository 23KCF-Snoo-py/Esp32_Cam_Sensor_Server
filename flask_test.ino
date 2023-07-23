#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <DHTesp.h>

DHTesp dht;
#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid = "nas";
const char* password = "kali0912!@#";
const char* serverIP = "3.39.224.161";
const int serverPort = 1234;

const int soilMoisturePin = 12;
const int cdsPin = 13;

const long imageInterval = 30000;
unsigned long lastImageCaptureTime = 0;

int readSoilMoisture() {
  return analogRead(soilMoisturePin);
}

int readCDS() {
  return analogRead(cdsPin);
}

void captureAndSendImage() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to capture image");
    return;
  }

  HTTPClient http;
  http.begin("http://" + String(serverIP) + ":" + String(serverPort) + "/upload_image");
  http.addHeader("Content-Type", "image/jpeg");
  http.addHeader("Content-Length", String(fb->len));

  int httpResponseCode = http.POST(fb->buf, fb->len);
  if (httpResponseCode > 0) {
    Serial.println("Image uploaded successfully");
  } else {
    Serial.print("Image upload failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  dht.setup(15, DHTesp::DHT_MODEL_t::DHT22);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA; // 640x480
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed with error 0x%x", err);
    return;
  }
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastImageCaptureTime >= imageInterval) {
    lastImageCaptureTime = currentTime;
    captureAndSendImage();
  }

  float soilMoisture = readSoilMoisture();
  float cdsValue = readCDS();
  // float temp = dht.getTemperature();
  // float humi = dht.getHumidity();
  float temp = 0.00;
  float humi = 0.00; 

  String sensorData = "soil=" + String(soilMoisture) + "&cds=" + String(cdsValue) + "&Temp=" + String(temp) + "&humi=" + String(humi);
  HTTPClient http;
  Serial.println(sensorData);
  http.begin("http://" + String(serverIP) + ":" + String(serverPort) + "/upload_sensor_data");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(sensorData);

  if (httpResponseCode > 0) {
    Serial.println("Sensor data uploaded successfully");
  } else {
    Serial.print("Sensor data upload failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  delay(30000);
}
