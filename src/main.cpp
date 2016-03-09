#include <Arduino.h>
#include <ESP8266WiFi.h>

void setup() {
  pinMode(2, OUTPUT);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin("ssid", "pass");
  int WiFiCounter = 0;
  while (WiFi.status() != WL_CONNECTED && WiFiCounter < 30) {
    digitalWrite(2, LOW);
    delay(1000);
    digitalWrite(2, HIGH);
    delay(1000);
    WiFiCounter++;
  }
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(2, LOW);
  } else {
    digitalWrite(2, HIGH);
  }
}
