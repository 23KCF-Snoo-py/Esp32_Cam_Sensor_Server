#include <WiFi.h>
#include <esp_camera.h>
#include <HTTPClient.h>
#include <DHTesp.h>
DHTesp dht;

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* serverIP = "your_server_ip";
const int serverPort = 1234;

const int soilMoisturePin = 12;
const int cdsPin = 13;

const long imageInterval = 1800000;
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
  config.pin_d0 = 32;
  config.pin_d1 = 35;
  config.pin_d2 = 34;
  config.pin_d3 = 5;
  config.pin_d4 = 39;
  config.pin_d5 = 18;
  config.pin_d6 = 36;
  config.pin_d7 = 19;
  config.pin_xclk = 27;
  config.pin_pclk = 21;
  config.pin_vsync = 22;
  config.pin_href = 26;
  config.pin_sscb_sda = 25;
  config.pin_sscb_scl = 23;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
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

  int soilMoisture = readSoilMoisture();
  int cdsValue = readCDS();
  float temp = dht.getTemperature();
  float humi = dht.getHumidity();

  String sensorData = "soil=" + String(soilMoisture) + "&cds=" + String(cdsValue) + "&Temp=" + String(temp) + "&humi=" + String(humi);
  HTTPClient http;
  http.begin("http://" + String(serverIP) + ":" + String(serverPort) + "/upload_sensor_data");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(sensorData);

  if (httpResponseCode > 0) {
    Serial.println("Sensor data uploaded successfully");
  } 
  else {
    Serial.print("Sensor data upload failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  delay(60000);
}
