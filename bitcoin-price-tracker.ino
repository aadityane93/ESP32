#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Address for 128x32 display

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "ssid";
const char* password = "pass";

const String url = "http://api.coindesk.com/v1/bpi/currentprice/BTC.json";

HTTPClient http;
String lastPrice;

void setup() {
  Serial.begin(115200);

  // Setup I2C pins for ESP32-C3
  Wire.begin(9, 8);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("Connected to SSID: ");
  Serial.println(ssid);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected to ");
  display.println(ssid);
  display.display();
  delay(2000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Getting current data...");

    http.begin(url);
    int httpCode = http.GET();
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      StaticJsonDocument<768> doc;
      DeserializationError error = deserializeJson(doc, http.getString());

      if (error) {
        Serial.print(F("deserializeJson failed: "));
        Serial.println(error.f_str());
        delay(2500);
        return;
      }

      String BTCUSDPrice = doc["bpi"]["USD"]["rate"].as<String>();
      if (BTCUSDPrice == lastPrice) {
        Serial.print("Price hasn't changed (Current/Last): ");
        Serial.print(BTCUSDPrice);
        Serial.print(" : ");
        Serial.println(lastPrice);
        delay(1250);
        return;
      } else {
        lastPrice = BTCUSDPrice;
      }

      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.println("BTC/USD");
      display.setTextSize(2);
      display.println("$" + BTCUSDPrice);
      display.display();
    }
    http.end();
    delay(10000); // Fetch new data every 10 seconds
  }
}

void printCenter(const String buf, int x, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); // Calculate width of the new string
  display.setCursor((x - w / 2) + (128 / 2), y);
  display.print(buf);
}
