#include <Arduino.h>
#include <ESP8266WiFi.h>

void setup() {
  pinMode(2, OUTPUT);
  int WiFiCounter = 0;
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin("ssid", "pass");
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
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
}
