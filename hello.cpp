#include <WiFi.h>
#include <HTTPClient.h>
#include <HX711.h>

// Load cell pins
#define DOUT 21  // Data pin (DT)
#define CLK  22  // Clock pin (SCK)

// Replace with your WiFi credentials
const char* ssid = "Airtel_Airtel-Madhu";
const char* password = "MadhuPrakash";

const String serverUrl = "http://192.168.1.8:3000";

// HX711 load cell initialization
HX711 scale;

// Calibration factor (adjust this value after calibration)
float calibrationFactor = 2280.f;  // Replace with your calibrated factor

void setup() {
  Serial.begin(115200);
  
  // Initialize the load cell
  scale.begin(DOUT, CLK);
  scale.set_scale(calibrationFactor);  // Set initial calibration factor
  scale.tare();  // Set the initial weight to zero
  
  Serial.println("Load cell initialized. Tare complete.");
  Serial.println("Place a known weight to calibrate, or use the current settings.");
  
  // Initialize WiFi connection
  connectToWiFi();
}

void loop() {
  // Read weight from load cell
  float weight = scale.get_units(10);
    if(weight<=0) weight=0;
    // Average over 10 readings
  Serial.println("Weight: " + String(weight*6.5) + " g");

  // Send the weight to the backend
  sendToBackend(weight*6.5);
  
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

void sendToBackend(float weight) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    String payload = "{\"weight\": " + String(weight, 2) + "}";  // Send weight with 2 decimal points
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("POST request sent successfully");
      Serial.println("Response code: " + String(httpResponseCode));
      String response = http.getString(); // Read the server's response
      Serial.println("Response from server: " + response);
    } else {
      Serial.println("Error in sending POST request");
      Serial.println("HTTP error code: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

