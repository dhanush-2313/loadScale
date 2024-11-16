#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <HX711.h>
#include <esp_camera.h>

// Load cell pins
#define DOUT  21  // Data pin (DT)
#define CLK   22  // Clock pin (SCK)

// Replace with your WiFi credentials
const char* ssid = "your-ssid";
const char* password = "your-password";

// Backend URL to send data
const String serverUrl = "http://your-backend-url.com/upload";

// HX711 load cell initialization
HX711 scale;

void setup() {
  Serial.begin(115200);
  
  // Initialize the load cell
  scale.begin(DOUT, CLK);
  
  // Calibrate load cell (adjust this as needed for your specific load cell)
  scale.set_scale(2280.f);  // Calibrate this value with a known weight
  scale.tare();  // Set the initial weight to zero
  
  // Initialize WiFi connection
  connectToWiFi();
  
  // Initialize camera
  initCamera();
}

void loop() {
  // Read weight from load cell
  long weight = scale.get_units(10);  // Average over 10 readings
  
  // Capture image from the ESP32-CAM
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to capture image");
    return;
  }

  // Send the image and weight to the backend
  sendToBackend(fb, weight);
  
  // Release the camera frame buffer
  esp_camera_fb_return(fb);
  
  delay(5000);  // Wait for 5 seconds before taking another measurement
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Init the camera
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera initialization failed");
    return;
  }
}

void sendToBackend(camera_fb_t *fb, long weight) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);  // Server URL
    
    // Set the content type to multipart/form-data
    http.addHeader("Content-Type", "multipart/form-data");

    // Create a multipart payload
    String payload = "--boundary\r\n";
    payload += "Content-Disposition: form-data; name=\"weight\"\r\n\r\n";
    payload += String(weight) + "\r\n";  // Send weight
    
    payload += "--boundary\r\n";
    payload += "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\n";
    payload += "Content-Type: image/jpeg\r\n\r\n";
    
    // Send the image bytes
    int imageLength = fb->len;
    uint8_t* imageData = fb->buf;
    http.addHeader("Content-Length", String(payload.length() + imageLength + 2));
    
    // Send the request body
    WiFiClient *client = http.getStreamPtr();
    client->print(payload);  // send weight data first
    client->write(imageData, imageLength);  // then send the image
    client->print("\r\n--boundary--\r\n");  // End boundary
    
    // Send the POST request
    int httpResponseCode = http.POST("");  // POST body is already sent
    if (httpResponseCode > 0) {
      Serial.println("POST request sent successfully");
      Serial.println("Response code: " + String(httpResponseCode));
    } else {
      Serial.println("Error in sending POST request");
    }

    http.end();  // Close the connection
  } else {
    Serial.println("WiFi not connected");
  }
}

