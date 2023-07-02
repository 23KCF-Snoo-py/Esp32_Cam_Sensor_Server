#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 4          // DHT22 data pin
#define DHTTYPE DHT22     // DHT22 sensor type
#define MOISTUREPIN 5     // Soil moisture sensor data pin
#define CDSPIN 6          // CDS sensor data pin
#define IMAGE_INTERVAL 600000  // Image capture interval (10 minutes)

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

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
    // Capture image here and serve it
    // Replace the following line with your code to capture and serve the image
    // For example, you can use the "camera_fb_get()" function provided by the ESP32-CAM library
    // Make sure to set the appropriate resolution and format based on your requirements
    // Save the captured image as "/image.jpg" in the ESP32-CAM filesystem
    // Then, read the image file and send it as a response using "request->send" method

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
  // Capture image and save it as "/image.jpg" in the ESP32-CAM filesystem
  // Replace the following line with your code to capture and save the image
  // For example, you can use the "camera_fb_get()" function provided by the ESP32-CAM library
}

void setIntervalCapture() {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= IMAGE_INTERVAL) {
    previousMillis = currentMillis;
    captureImage();
  }
}
