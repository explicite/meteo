#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <SI70xx.hpp>
#include <BH17xx.hpp>
#include <LPS331AP.hpp>

#define BAUD          0xE100
#define SDA           0x00 // GPIO0
#define SCL           0x02 // GPIO2
#define WAKEUP_PIN    0x0C // GPI16
#define SLEEP_TIME    0x96 // 60 seconds
#define WIFI_CON_TRY  0x1E
#define SI7021_ADDR   0x40
#define BH1750_ADDR   0x23
#define LPS331AP_ADDR 0x5d
#define SSID "node"
#define PASSWD "node"
#define DEBUG
#undef  DEBUG

ADC_MODE(ADC_VCC);
IPAddress server(000, 000, 000, 000);

SI70xx si7021(SI7021_ADDR);
BH17xx bh1750(BH1750_ADDR, BH1750);
LPS331AP lps331ap(LPS331AP_ADDR);
WiFiClient client;
String id = String(ESP.getChipId());
String vcc;
String temperatureSI7021;
String humiditySI7021;
String illuminanceBH1750;
String temperatureLPS331AP;
String pressureLPS331AP;
String body;

void send() {
  vcc = String(ESP.getVcc());
  temperatureSI7021 = String(si7021.getTemperature()/1000.0);
  humiditySI7021 = String(si7021.getHumidity()/1000.0);
  illuminanceBH1750 = String(bh1750.getIlluminance());
  temperatureLPS331AP = String(lps331ap.getTemperature());
  pressureLPS331AP = String(lps331ap.getPressure());

  body = String(
    "esp8266,sensor=esp8266,type=id value=" + id + "\n" +
    "esp8266,sensor=esp8266,type=vcc value=" + vcc + "\n" +
    "esp8266,sensor=si7021,type=humidity value=" + humiditySI7021 + "\n" +
    "esp8266,sensor=si7021,type=temperature value=" + temperatureSI7021 + "\n" +
    "esp8266,sensor=bh1750,type=illuminance value=" + illuminanceBH1750 + "\n" +
    "esp8266,sensor=lps331ap,type=temperature value=" + temperatureLPS331AP + "\n" +
    "esp8266,sensor=lps331ap,type=pressure value=" + pressureLPS331AP
  );

  #ifdef DEBUG
  Serial.println(body);
  #endif

  if (client.connect(server, 8086)) {
    client.println("POST /write?db=weather HTTP/1.1");
    client.println("Accept:*/*");
    client.println("Accept-Encoding:gzip, deflate");
    client.println("Host: 212.47.230.159:8086");
    client.println("User-Agent: esp8266");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println();
    client.println(body);
  }

  delay(5000);
}

void setup() {
  #ifdef DEBUG
  Serial.begin(BAUD);
  Serial.println("\nWAKE UP");
  #endif

  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWD);
  si7021.init(SDA, SCL);
  bh1750.init(SDA, SCL);
  lps331ap.init(SDA, SCL);
  lps331ap.enable();

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < WIFI_CON_TRY) {
    delay(2000);
    counter++;
  }

  send();

  ESP.deepSleep(SLEEP_TIME * 1000000);
}

void loop() {
  //Deep sleep
}
