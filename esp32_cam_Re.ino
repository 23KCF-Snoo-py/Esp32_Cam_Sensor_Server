#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "esp_camera.h"

#define DHTPIN 4          // DHT22 data pin
#define DHTTYPE DHT22     // DHT22 sensor type
#define MOISTUREPIN 5     // Soil moisture sensor data pin
#define CDSPIN 6          // CDS sensor data pin
#define IMAGE_INTERVAL 1800000  // Image capture interval (30 minutes)
#define SENSOR_INTERVAL 60000   // Sensor data update interval (1 minute)

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

camera_config_t cameraConfig;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize sensors
  dht.begin();

  // Initialize camera configuration
  cameraConfig.ledc_channel = LEDC_CHANNEL_0;
  cameraConfig.ledc_timer = LEDC_TIMER_0;
  cameraConfig.pin_d0 = Y2_GPIO_NUM;
  cameraConfig.pin_d1 = Y3_GPIO_NUM;
  cameraConfig.pin_d2 = Y4_GPIO_NUM;
  cameraConfig.pin_d3 = Y5_GPIO_NUM;
  cameraConfig.pin_d4 = Y6_GPIO_NUM;
  cameraConfig.pin_d5 = Y7_GPIO_NUM;
  cameraConfig.pin_d6 = Y8_GPIO_NUM;
  cameraConfig.pin_d7 = Y9_GPIO_NUM;
  cameraConfig.pin_xclk = XCLK_GPIO_NUM;
  cameraConfig.pin_pclk = PCLK_GPIO_NUM;
  cameraConfig.pin_vsync = VSYNC_GPIO_NUM;
  cameraConfig.pin_href = HREF_GPIO_NUM;
  cameraConfig.pin_sscb_sda = SIOD_GPIO_NUM;
  cameraConfig.pin_sscb_scl = SIOC_GPIO_NUM;
  cameraConfig.pin_pwdn = PWDN_GPIO_NUM;
  cameraConfig.pin_reset = RESET_GPIO_NUM;
  cameraConfig.xclk_freq_hz = 20000000;
  cameraConfig.pixel_format = PIXFORMAT_JPEG;
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&cameraConfig);
  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed with error 0x%x", err);
    return;
  }

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><body>";
    html += "<h1>ESP32-CAM Sensor Data</h1>";
    html += "<p>Temperature: " + String(readTemperature()) + " &#8451;</p>";
    html += "<p>Humidity: " + String(readHumidity()) + " %</p>";
    html += "<p>Soil Moisture: " + String(readSoilMoisture()) + " %</p>";
    html += "<p>CDS Sensor: " + String(readCds()) + "</p>";
    html += "<img src=\"/image.jpg\" alt=\"ESP32-CAM Image\" width=\"640\" height=\"480\">";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/image.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    captureImage();
    File imageFile = SPIFFS.open("/image.jpg", "r");
    if (imageFile) {
      request->send(imageFile, "image/jpeg");
      imageFile.close();
    } else {
      request->send(404);
    }
  });

  server.begin();

  // Start image capture timer
  setIntervalCapture();
}

void loop() {
  static unsigned long previousSensorMillis = 0;
  unsigned long currentMillis = millis();

  // Check if it's time to update sensor data
  if (currentMillis - previousSensorMillis >= SENSOR_INTERVAL) {
    previousSensorMillis = currentMillis;
    // Update sensor data
    // Replace the following line with your code to update the sensor data if necessary
  }

  server.handleClient();
}

float readTemperature() {
  return dht.readTemperature();
}

float readHumidity() {
  return dht.readHumidity();
}

int readSoilMoisture() {
  return analogRead(MOISTUREPIN);
}

int readCds() {
  return analogRead(CDSPIN);
}

void captureImage() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  fs::FS &fs = SPIFFS;
  File file = fs.open("/image.jpg", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  
  file.write(fb->buf, fb->len);
  file.close();

  esp_camera_fb_return(fb);
}

void setIntervalCapture() {
  static unsigned long previousImageMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousImageMillis >= IMAGE_INTERVAL) {
    previousImageMillis = currentMillis;
    captureImage();
  }
}
