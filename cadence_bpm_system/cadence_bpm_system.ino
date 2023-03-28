
#include <WiFi.h>
#include <time.h>

const char* ssid = "";
const char* password = "";
WiFiClient client;  // TODO: WiFiClientSecure instead?

unsigned long prev_timestamp;
unsigned int loop_delay = 10000;  // 10 secs

void setup() {

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    prev_timestamp = millis();
}

void loop() {
  unsigned long curr_timestamp = millis();
  if(curr_timestamp - prev_timestamp > loop_delay) {




    prev_timestamp = millis();
  }
}
