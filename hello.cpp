#include <WiFi.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <Wire.h>              
#include <LiquidCrystal_I2C.h>

#define DOUT 16 
#define CLK 17  

const char* ssid = "";
const char* password = "";

const String serverUrl = "";

HX711 scale;

float calibrationFactor = 502; 

LiquidCrystal_I2C lcd(0x3f, 16, 2);  

void setup() {
  Serial.begin(115200);

  scale.begin(DOUT, CLK);
  scale.set_scale(calibrationFactor);  
  scale.tare();                        

  Serial.println("Load cell initialized. Tare complete.");
  Serial.println("Place a known weight to calibrate, or use the current settings.");

  connectToWiFi();

  lcd.init();       
  lcd.backlight();  
  lcd.clear();     
}

void loop() {
  float weight = scale.get_units(10);
  if (weight <= 5) weight = 0;  

  Serial.println(weight);

  lcd.setCursor(0, 0);       
  lcd.print("Wt:        ");  

  lcd.setCursor(0, 0);   
  lcd.print("Wt: ");     
  lcd.print(weight, 2);  
  lcd.print(" g");      

  if (weight != 0) {
    sendToBackend(weight);
  }

  delay(1000); 
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
    String payload = "{\"weight\": " + String(weight, 2) + "}";
    http.POST(payload);
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
