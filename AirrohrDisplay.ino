/**
   AirrohrDisplay.ino

    Created by: passantedimezzo

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <LiquidCrystal_I2C.h>

ESP8266WiFiMulti WiFiMulti;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int status = WL_IDLE_STATUS;     // the Wifi radio's status


#define TEMPERATURE 1
byte temperature[8] = //icon for thermometer
{
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

#define HUMIDITY 2
byte humidity[8] = //icon for water droplet
{
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110,
};

#define MICRO 3
byte mu[8] = //icon for µ
{
  B00000,
  B00000,
  B10001,
  B10001,
  B10011,
  B11101,
  B10000,
  B10000
};

#define CUBE 4
byte cube[8] = //icon for ³
{
  B11110,
  B00010,
  B00100,
  B00010,
  B10010,
  B01100,
  B00000,
  B00000
};


const int interruptPin = 0; //GPIO 0 (Flash Button)
const int LED = 2; //On board blue LED
int interrupt = false;

void handleInterrupt() {
  Serial.println("Interrupt Detected");
  interrupt = true;
}

char mac[20] = "000000000000"; // macaddress

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  digitalWrite(2, HIGH); // Blue LED off

  lcd.begin(16, 2);
  lcd.init();
  lcd.createChar(TEMPERATURE, temperature);
  lcd.createChar(HUMIDITY, humidity);
  lcd.createChar(MICRO, mu);
  lcd.createChar(CUBE, cube);
  lcd.backlight();

  pinMode(LED, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

  lcd.setCursor(0, 0);
  lcd.print("MAC:");


  //sscanf (string2char(WiFi.macAddress()), "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]", &mac1, &mac2, &mac3, &mac4, &mac5, &mac6);
  sscanf (WiFi.macAddress().c_str(), "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]", &mac, &mac[2], &mac[4], &mac[6], &mac[8], &mac[10]);
  lcd.print(mac);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("AccessPoint1", "mypasswd");
  WiFiMulti.addAP("AccessPoint2", "mypasswd");
  
  WiFiMulti.run();

  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect");

    // Connect to WPA/WPA2 network. 
    WiFiMulti.run();

    // wait 10 seconds for connection:
    delay(10000);
  }

  // print your WiFi shield's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(15000);

}

void pingsensor(const char* sds011url, const char* dht22url, const char* desc, int loops) {

  float pm10 = -1000.0;
  float pm25 = -1000.0;
  float temperature = -1000.0;
  float humidity = -1000.0;

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");

    http.begin(sds011url);

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        const char *strbuffer = payload.c_str();

        char* idx = 0;
        if (idx = strstr(strbuffer, "\"value\":\"")) {
          Serial.println("Found the  word at index: ");
          idx = idx + 9;
          sscanf (idx, "%f", &pm10);
          Serial.println(pm10);
        }

        if ((idx != 0) && (idx = strstr(idx, "\"value\":\""))) {
          Serial.println("Found the  word at index: ");
          idx = idx + 9;

          sscanf (idx, "%f", &pm25);
          Serial.println(pm25);
        }
      }

    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("No connection ...\n");
    //WiFi.disconnect(); WiFi.reconnect();
  }

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");

    http.begin(dht22url);

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        const char *strbuffer = payload.c_str();

        char* idx = 0;
        if (idx = strstr(strbuffer, "\"value\":\"")) {
          Serial.println("Found the  word at index: ");
          idx = idx + 9;
          sscanf (idx, "%f", &temperature);
          Serial.println(temperature);
        }

        if ((idx != 0) && (idx = strstr(idx, "\"value\":\""))) {
          Serial.println("Found the  word at index: ");
          idx = idx + 9;

          sscanf (idx, "%f", &humidity);
          Serial.println(humidity);
        }
      }

    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();


 
  } else {
    Serial.printf("No connection ...\n");
    //WiFi.disconnect();
    //WiFiMulti.run();
    //WiFi.disconnect(); WiFi.reconnect();
  }

   for (int n = 0; n < loops; n++) {

      if (interrupt) {
        about(); interrupt = false;
      }


      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(desc);
      lcd.setCursor(0, 1);
      lcd.print("PM10: ");
      if (pm10 > -1000.0) lcd.print((int) (pm10 + 0.5f)); else  {
        lcd.print("--");
        n = loops;
      }
      lcd.print(" "); lcd.write(MICRO); lcd.print("g/m"); lcd.write(CUBE);    //  µg/m³
      delay(5000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(desc);
      lcd.setCursor(0, 1);
      lcd.print("PM2.5: ");
      if (pm25 > -1000.0) lcd.print((int) (pm25 + 0.5f)); else lcd.print("--");
      lcd.print(" "); lcd.write(MICRO); lcd.print("g/m"); lcd.write(CUBE);    //  µg/m³
      delay(5000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(desc);
      lcd.setCursor(0, 1);
      lcd.print(" "); lcd.write(TEMPERATURE); lcd.print(" ");
      if (temperature > -1000.0)  {
        if (temperature < 0) lcd.print((int) (temperature - 0.5f));
        else lcd.print((int) (temperature + 0.5f));
      } else {
        lcd.print("--");
      }
      lcd.print((char)223); lcd.print("C  ");
      lcd.write(HUMIDITY); lcd.print(" ");
      if (humidity > -1000.0) lcd.print((int) (humidity + 0.5f)); else lcd.print("--");
      lcd.print("%");
      delay(5000);

    }


}


void about() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("#airrohrdisplay");

  lcd.setCursor(0, 1);
  lcd.print("@passantedimezzo");
  delay(5000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("bologna.maps.");
  lcd.setCursor(0, 1);
  lcd.print("luftdaten.info/");
  delay(5000);
}

void loop() {
  Serial.println("WiFiMulti.run()");
  WiFiMulti.run();

  about();

  pingsensor("http://api.luftdaten.info/v1/sensor/3151/", "http://api.luftdaten.info/v1/sensor/3152/", "Liceo Sabin", 2);

  pingsensor("http://api.luftdaten.info/v1/sensor/1495/", "http://api.luftdaten.info/v1/sensor/1496/", "Arcoveggio", 1);

  pingsensor("http://api.luftdaten.info/v1/sensor/3457/", "http://api.luftdaten.info/v1/sensor/3458/", "Crocecoperta", 1);

  pingsensor("http://api.luftdaten.info/v1/sensor/5191/", "http://api.luftdaten.info/v1/sensor/5192/", "Aeroporto", 1);

  pingsensor("http://api.luftdaten.info/v1/sensor/9294/", "http://api.luftdaten.info/v1/sensor/9295/", "Saragozza", 1);

  pingsensor("http://api.luftdaten.info/v1/sensor/4371/", "http://api.luftdaten.info/v1/sensor/4372/", "Modena", 1);

  pingsensor("http://api.luftdaten.info/v1/sensor/9050/", "http://api.luftdaten.info/v1/sensor/9051/", "Piacenza", 1);
}



