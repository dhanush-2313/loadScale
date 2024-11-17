#include <WiFi.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <Wire.h>               // Include Wire library for I2C communication
#include <LiquidCrystal_I2C.h>  // Include LiquidCrystal_I2C library

// Load cell pins
#define DOUT 16  // Data pin (DT)
#define CLK 17   // Clock pin (SCK)

// Replace with your WiFi credentials
const char* ssid = "";
const char* password = "";

const String serverUrl = "";

// HX711 load cell initialization
HX711 scale;

// Calibration factor (adjust this value after calibration)
float calibrationFactor = 502;  // Replace with your calibrated factor

// LCD Initialization (Assuming address 0x3f, 16x2 LCD)
LiquidCrystal_I2C lcd(0x3f, 16, 2);  // 16 columns and 2 rows LCD

void setup() {
  Serial.begin(115200);

  // Initialize the load cell
  scale.begin(DOUT, CLK);
  scale.set_scale(calibrationFactor);  // Set initial calibration factor
  scale.tare();                        // Set the initial weight to zero

  Serial.println("Load cell initialized. Tare complete.");
  Serial.println("Place a known weight to calibrate, or use the current settings.");

  // Initialize WiFi connection
  connectToWiFi();

  // Initialize LCD
  lcd.init();       // Initialize the LCD
  lcd.backlight();  // Turn on the backlight
  lcd.clear();      // Clear any old text from the LCD
}

void loop() {
  float weight = scale.get_units(10);
  if (weight <= 5) weight = 0;  // Zero out small weight readings

  Serial.println(weight);

  // Clear the previous weight on the LCD line
  lcd.setCursor(0, 0);       // Set cursor to the first row, first column
  lcd.print("Wt:        ");  // Clear the line by printing extra spaces

  // Print the new weight on the LCD
  lcd.setCursor(0, 0);   // Set cursor to first row, first column
  lcd.print("Wt: ");     // Print label "Wt: "
  lcd.print(weight, 2);  // Print the weight with 2 decimal places
  lcd.print(" g");       // Print " g" to denote grams

  // Send the weight to the backend
  if (weight != 0) {
    sendToBackend(weight);
  }

  delay(1000);  // Add a small delay to reduce flickering on the LCD
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);  // Small delay to check for connection
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void sendToBackend(float weight) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    String payload = "{\"weight\": " + String(weight, 2) + "}";
    http.POST(payload);
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
