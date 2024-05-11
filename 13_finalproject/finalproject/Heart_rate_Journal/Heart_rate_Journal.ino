#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PulseSensorPlayground.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ChatGPT.hpp>

// WiFi and OLED display settings
const char* ssid = "Bernie";
const char* password = "21278tmac";
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// PulseSensor settings
const int PulseWire = 15;  // PulseSensor PURPLE WIRE connected to GPIO 15 (D15)
const int LED = 2;  // GPIO 2, commonly used for onboard LED in ESP32
int Threshold = 550;

// Global objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
PulseSensorPlayground pulseSensor;
WiFiClientSecure client;
ChatGPT<WiFiClientSecure> chat_gpt(&client, "v1", "insert api);

unsigned long lastUpdate = 0;  // Time since last display update
const long updateInterval = 200;  // Update interval in milliseconds

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  display.println("Connecting to WiFi...");
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  display.println("Connected!");
  display.display();
  client.setInsecure();

  pulseSensor.analogInput(PulseWire);
  pulseSensor.setThreshold(Threshold);
  if (!pulseSensor.begin()) {
    display.println("Failed to start pulse sensor");
    display.display();
    while (1);
  }
}

void loop() {
  if (pulseSensor.sawStartOfBeat()) {
    int heartRate = pulseSensor.getBeatsPerMinute();

    if (millis() - lastUpdate > updateInterval) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Heart Rate: ");
      display.print(heartRate);
      display.println(" BPM");
      display.display();
      lastUpdate = millis();

      String prompt;
      if (heartRate < 60) {
        prompt = "Generate a calming journal prompt based on low heart rate.";
      } else if (heartRate >= 60 && heartRate < 80) {
        prompt = "Create a reflective journal prompt for a heart rate in a relaxed state.";
      } else if (heartRate >= 80 && heartRate < 100) {
        prompt = "Generate a journal prompt for moderate activity and stress levels.";
      } else if (heartRate >= 100 && heartRate < 120) {
        prompt = "Develop a prompt to explore feelings during increased physical or emotional stress.";
      } else {
        prompt = "Provide breathing exercises to help reduce heart rate and stress.";
      }

      String response;
      if (chat_gpt.simple_message("gpt-3.5-turbo-0301", "user", prompt, response)) {
        if (response.length() > 110) {
          response = response.substring(0, 110);  // Truncate to 110 characters
        }
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(response);
        display.display();
      } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Failed to get response:");
        display.println(response);
        display.display();
      }
    }
  }
}
