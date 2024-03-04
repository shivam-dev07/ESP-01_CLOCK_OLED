

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <U8x8lib.h>

// ========== user settings ==========

const char          *ssid          = "Raju";             // your wifi name
const char          *pw            = "thekingsuv1901";             // your wifi password
const char          *ntpServer     = "pool.ntp.org"; // change it to local NTP server if needed
const unsigned long updateDelay    = 900;            // NTP update interval (unit: seconds, default: 15 min)
const float          timezoneOffset = 5.5;              // timezone offset (unit: hours)

#define SCL_PIN 2  // SCL pin of OLED. Default: D1 (ESP8266) or D22 (ESP32)
#define SDA_PIN 0  // SDA pin of OLED. Default: D2 (ESP8266) or D21 (ESP32)

// ====================================

const String  weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
unsigned long lastCheckTime = 0;
unsigned int  second_prev = 0;
bool          colon_switch = false;

// NTPClient: https://github.com/arduino-libraries/NTPClient
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);

// U8X8 Display constructors: https://github.com/olikraus/u8g2/wiki/u8x8setupcpp
// U8X8 Fonts: https://github.com/olikraus/u8g2/wiki/u8x8reference
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(SCL_PIN, SDA_PIN, U8X8_PIN_NONE);

void setup() {

  // serial baud rate: 115200
  Serial.begin(115200);

  // initialize OLED
  u8x8.begin();
  u8x8.setBusClock(400000);
  u8x8.setFont(u8x8_font_7x14B_1x2_f);
  u8x8.drawString(0, 0, "Connecting");
  u8x8.drawString(0, 3, "  to WiFi...");

  // connect to wifi
  Serial.println("Connecting to WiFi...");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected. IP:");
  Serial.println(WiFi.localIP());

  // clear OLED
  u8x8.clear();

  // setup NTPClient
  timeClient.setUpdateInterval(updateDelay * 1000);
  timeClient.setTimeOffset(timezoneOffset * 60 * 60);
  timeClient.begin();
  while (!timeClient.update()) timeClient.forceUpdate();
  lastCheckTime = millis();

}

void loop() {

  // check wifi status and NTP updates
  if (millis() - lastCheckTime >= updateDelay * 1000) {
    // reconnect wifi if needed
    if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
    // update NTP (and force update if needed)
    if (!timeClient.update()) timeClient.forceUpdate();
    lastCheckTime = millis();
  }

  // extract second from board's internal clock
  unsigned int second = timeClient.getSeconds();

  // when the second number changes, extract the rest of values
  // then re-draw the clock texts
  if (second != second_prev) {

    colon_switch = !colon_switch;

    time_t rawtime = timeClient.getEpochTime();
    struct tm * ti;
    ti = localtime(&rawtime);
    unsigned int year = ti->tm_year + 1900;
    unsigned int month = ti->tm_mon + 1;
    unsigned int day = ti->tm_mday;
    unsigned int hour = timeClient.getHours();
    unsigned int minute = timeClient.getMinutes();

    String fYear = String(year);
    String fDate = (month < 10 ? "0" : "") + String(month) + "/" + (day < 10 ? "0" : "") + String(day);
    String fTime = (hour < 10 ? "0" : "") + String(hour) + (colon_switch ? ":" : " ") + (minute < 10 ? "0" : "") + String(minute);
    String weekDay = weekDays[timeClient.getDay()];

    Serial.println("Board time: " + fYear + "/" + fDate + " (" + weekDay + ") " + timeClient.getFormattedTime());

    u8x8.setFont(u8x8_font_lucasarts_scumm_subtitle_o_2x2_f);
    u8x8.drawString(1, 0, strcpy(new char[fDate.length() + 1], fDate.c_str()));
    u8x8.setFont(u8x8_font_pxplusibmcga_f);
    u8x8.drawString(12, 0, strcpy(new char[fYear.length() + 1], fYear.c_str()));
    u8x8.setFont(u8x8_font_victoriamedium8_r);
    u8x8.drawString(12, 1, strcpy(new char[weekDay.length() + 1], weekDay.c_str()));
    u8x8.setFont(u8x8_font_inb33_3x6_f);
    u8x8.drawString(1, 2, strcpy(new char[fTime.length() + 1], fTime.c_str()));

  }

  second_prev = second;

  delay(200);

}