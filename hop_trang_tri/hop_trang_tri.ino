

#include <Wire.h>
#include <ESP8266WiFi.h>                 // thu vien mang cho esp8266 
#include <WiFiClient.h>                  // 
#include <ESP8266WebServer.h>            // webserver 
#include <ESP8266mDNS.h>               // thu vien dinh nghia ten mien
#include <ESP8266HTTPUpdateServer.h>  // thu vien update
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h> // http web access library
#include <Arduino_JSON.h>

#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#include"image.h"
#include"ota.h"
#include"led.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include"app.h"
#include"weather.h"
#include <string>

#define JSON_BUFF_DIMENSION 2500
String openWeatherMapApiKey = "00d873c2b4b19180d7f85b2b214b5599";

// Replace with your country code and city
String city = "Hanoi";
String countryCode = "VN";


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String jsonBuffer;

WiFiClient client;
//Week Days
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

String addr;
unsigned long resetTime;

int temp;
int hum ;
int wind ;
String country;
String city1;
String weather;
int wea = 0;
long pres;

String httpGETRequest(const char* serverName) {
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    //    Serial.print("HTTP Response code: ");
    //    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void loadWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";
    jsonBuffer = httpGETRequest(serverPath.c_str());
    //Serial.println(jsonBuffer);
    JSONVar myObject = JSON.parse(jsonBuffer);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      //Serial.println("Parsing input failed!");
      return;
    }
    int tem  ;
    long tem1;
    tem =  myObject["main"]["temp"];
    if (tem != 0) {
      temp = tem;
    }

    tem1 =  myObject["main"]["pressure"];
    if ( tem1 != 0) {
      pres = tem1;
    }

    tem = myObject["main"]["humidity"];
    if (tem != 0) {
      hum  = tem;
    }
    tem =  myObject["wind"]["speed"];
    if (tem != 0) {
      wind = tem;
    }
    if (JSON.stringify(myObject["sys"]["country"]) != NULL) {
      country = JSON.stringify(myObject["sys"]["country"]);
    }
    if (JSON.stringify(myObject["name"]) != NULL) {
      city1 = JSON.stringify(myObject["name"]);
    }

    if (JSON.stringify(myObject["weather"][0]["description"]) != NULL) {
      weather = JSON.stringify(myObject["weather"][0]["description"]);

      if (weather.indexOf(String("cloud")) != -1) {
        wea = 1; // clould
      } else {
        wea = 2;
      }

    }
  }
  else {
    //Serial.println("WiFi Disconnected");
  }

}

void setup() {
  Serial.begin(115200);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  Serial.println();
  Serial.println("Booting programs...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  pixels.clear(); // Set all pixel colors to 'off'
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {

    pixels.setPixelColor(retries % NUMPIXELS, pixels.Color(0, 150, 0));
    retries++;
    pixels.show();   // Send the updated pixel colors to the hardware.
    delay(1000);
    if (retries >= NUMPIXELS) {
      pixels.clear();
      for (int i = 0 ; i < NUMPIXELS ; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        pixels.show();   // Send the updated pixel colors to the hardware.
      }
      break;
    }
    Serial.print(".");
  }
  //  pixels.clear();
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");

  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  MDNS.begin(host);     // multicast DNS
  MDNS.addService("http", "tcp", 80);

  httpUpdater.setup(&webServer, updatePath, updateUsername, updatePassword); //
  webServer.on("/", [] {
    String s = MainPage;
    webServer.send(200, "text/html", s);
  });
  webServer.begin();
  Serial.println("Web Server is started!");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  addr = WiFi.localIP().toString();
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  //=========Chương trình Chính=====//
  pinMode(BUT, INPUT);
  resetTime = millis();
  //  while(1);
  //============End=================//
}

void testdrawbitmap(const uint8_t bitmap[], int16_t LOGO_HEIGHT, int16_t LOGO_WIDTH ) {
  display.clearDisplay();
  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    bitmap, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  //  delay(100);

}

void testdrawbitmapcur(int x, int y, const uint8_t bitmap[], int16_t LOGO_HEIGHT, int16_t LOGO_WIDTH ) {
  //  display.clearDisplay();
  display.drawBitmap(
    y,  // width
    x,  // height
    bitmap, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  //  delay(100);

}


void getTime(time_t *epoch) {
  timeClient.update();
  *epoch = timeClient.getEpochTime();
}
String formattedTime;
int currentHour;
int currentMinute;
int currentSecond;
String weekDay;
struct tm *ptm;
int monthDay;
int currentMonth ;
String currentMonthName;
int currentYear;
time_t epoch;
time_t epochTemp;

String cTime;
String cDate;
uint8_t count = 0;

uint8_t tempClick = 0;

uint8_t state = 0;
uint8_t oldState = 0;

unsigned long curTime;
unsigned long clickTime;
unsigned long stateTime;
unsigned long lastTime;
unsigned long longClickTime;
uint8_t longClick;

uint8_t clickCnt = 0;

uint8_t state_led = 0;
uint8_t r, g, b;

uint8_t config_enable = 0;

uint8_t cur = 0;
uint8_t oldcur = 0;

unsigned long weatherTime = 0;

void loop() {


  //
  //  testdrawbitmap(logo_bmp,67,45);
  //  testdrawbitmap(logo_bmp1,60,120);
  //  testdrawbitmap(logo_bmp2,60,120);
  curTime = millis();
  // trigger click
  if (digitalRead(BUT) == 1 ) {
    tempClick = 10;
    if (longClick == 0 ) {
      longClickTime = curTime;
    }

    longClick = 10;
  }

  // check long click
  if (longClick > 0 && millis() - longClickTime > 3000 && digitalRead(BUT) == 1) {
    Serial.println("Long click");
    ESP.reset();
  }


  if (tempClick > 0 && digitalRead(BUT) == 0) {
    tempClick = 0;
    if (clickCnt == 0) {
      oldState = state;
    }

    clickTime = millis();
    longClick = 0 ;
    clickCnt++;
  }

  // clear or check button event
  if ((millis() - clickTime > 600 && digitalRead(BUT) == 0)) {
    switch (clickCnt) {
      case 1:
        if (config_enable == 0) {
          display.clearDisplay();
          state = (oldState + 1) % 4;
        } else {
          state_led = 1;
        }

        break;

      case 2:
        config_enable = (config_enable + 1) % 2;
        break;

      case 3:
        state_led = 0; // off led
        break;

      default:
        break;
    }
    clickCnt = 0;
  }


  switch (state) {
    case SHOW_TIME:
      if (curTime - stateTime > 1000) {
        Serial.println("Show time");
        if (millis() - lastTime > 36000 || lastTime == 0 ) {
          getTime(&epoch);
          lastTime = millis();
        } else {
          epochTemp = epoch + ((millis() - lastTime) / 1000);
          //          lastTime = millis();
        }
        formattedTime = timeClient.getFormattedTime();
        currentHour = timeClient.getHours();
        currentMinute = timeClient.getMinutes();
        currentSecond = timeClient.getSeconds();

        weekDay = weekDays[timeClient.getDay()];
        ptm = gmtime (&epochTemp);
        monthDay = ptm->tm_mday;
        currentMonth = ptm->tm_mon + 1;
        currentMonthName = months[currentMonth - 1];
        currentYear = ptm->tm_year + 1900;

        cTime =  String(currentHour)  + ":" + String(currentMinute) + ":" + String(currentSecond);
        cDate =  String( monthDay)  + "-" + String(currentMonth) + "-" + String(currentYear);
        Serial.println(cTime);
        Serial.println(cDate);
        display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(10, 15);            // Start at top-left corner
        display.println(cTime);
        display.setTextSize(1);             // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(30, 45);            // Start at top-left corner
        display.println(cDate);
        display.display();
        stateTime = curTime;
      }
      break;

    case SHOW_WEATHER:
      if (curTime - stateTime > 500) {
        if (curTime - weatherTime > 3600000 || weatherTime == 0 ||  temp ==0) {
          loadWeather();
          display.clearDisplay();
          weatherTime = curTime;
        }



        Serial.println ("SHOW_WEATHER ");
        //        testdrawbitmapcur( 0 , oldcur, cleari, 32, 32);
        for (int i = 0 ; i < 32 ; i++) {
          for (int j = 0 ; j < 128; j++) {
            display.drawPixel(j, i, 0);

            // Show the display buffer on the screen. You MUST call display() after
            // drawing commands to make them visible on screen!

          }
        }
        display.display();
        if (wea == 1) {
          testdrawbitmapcur( 0 , cur, sunny, 32, 32);
        }
        else {
          testdrawbitmapcur( 0 , cur, rainy, 32, 32);
        }
        oldcur = cur;
        cur = (cur + 5) % 128;
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(10, 42);
        display.print(temp);
        display.println("*C");
        display.setTextSize(1);
        display.setCursor(74, 33);
        display.print(wind);
        display.println("m/s");

        display.setCursor(74, 45);
        display.print(hum);
        display.println("%");
        display.setCursor(74, 55);
        display.print(pres);
        display.println("hPA");
        display.display();
        stateTime = curTime;
      }
      break;

    case SHOW_IMAGE:
      if (curTime - stateTime > 500) {
        Serial.println ("SHOW_IMAGE ");
        display.clearDisplay();
        testdrawbitmap(logo_bmp, 67, 45);
        stateTime = curTime;
      }
      break;

    case SHOW_ADDR:
      if (curTime - stateTime > 1000) {
        Serial.println ("SHOW_ADDR ");
        display.clearDisplay();
        display.setTextSize(1);             // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(30, 20);            // Start at top-left corner
        display.println(addr);

        display.display();

        stateTime = curTime;
      }
      break;

    default:
      state =  SHOW_TIME;
      break;
  }

  switch (state_led) {
    case LED_OFF:
      r = 0; g = 0; b = 0;
      break;

    case LED_CHANGE:
      if (curTime - stateTime > 50) {

        r = random(255);
        Serial.print("R ");
        Serial.print(r);
        g = random(255);
        b = random(255);
        Serial.print(" - G ");
        Serial.print(g);
        Serial.print(" - B ");
        Serial.println(g);
        state_led = 5;
      }
      break;

    case LED_EFFECT:
      break;

    default:
      break;
  }

  if (config_enable == 1) {
    display.setTextSize(1);             // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(100, 55);            // Start at top-left corner
    display.println("EN");
    display.display();
  }
  //        pixels.clear();
  for (int i = 0 ; i < NUMPIXELS ; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
    pixels.show();   // Send the updated pixel colors to the hardware.
  }

  if ((millis() - resetTime) / 1000 >= 86400) {
    ESP.reset();
  }
  MDNS.update();
  webServer.handleClient();
}
