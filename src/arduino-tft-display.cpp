#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <time.h>
#include <Arduino_JSON.h>

#include "../lib/config.h"
#include "../lib/size.h"

String intMonthToText(int num);
void oledDisplayCenter(String text);
void print_labels();
void draw_background();
void print_hour(uint8_t hours, uint16_t color);
void print_min(uint8_t minutes, uint16_t color);
void print_ampm(uint8_t seconds, uint16_t color);
String httpGETRequest(const char *serverName);
void print_time(int x_pos, int y_pos, int color, int size, int hour, int min, int sec);
void print_date(int x_pos, int y_pos, int color, int size, int month, int day, int year);
void print_data(int x_pos, int y_pos, int color, int size, String data, char *units);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

String jsonBuffer;
JSONVar weather_data;
String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=" + units;

void setup(void)
{
    Serial.begin(9600);

    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    tft.initR(INITR_144GREENTAB);
    tft.fillScreen(ST7735_BLACK);
    tft.setRotation(2);

    configTime(timezone * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov");
    while (!time(nullptr))
    {
        Serial.print(".");
        delay(1000);
    }

    draw_background();
    print_labels();

    delay(1000);
    Serial.println("Timer set to 60 seconds (timerDelay variable), it will take 60 seconds before publishing the first reading.");
}

void loop()
{
    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);

    if ((millis() - lastTime) > timerDelay)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            jsonBuffer = httpGETRequest(serverPath.c_str());
            weather_data = JSON.parse(jsonBuffer);

            if (JSON.typeof(weather_data) == "undefined")
            {
                Serial.println("Parsing input failed!");
                return;
            }

            // Inside temp data
            String temp = JSON.stringify(int(weather_data["main"]["temp"]));
            print_data(55, 55, ST7735_CYAN, size::small, temp, "F");

            // Humid data
            String humid = JSON.stringify(weather_data["main"]["humidity"]);
            print_data(95, 70, ST7735_CYAN, size::small, humid, "%");

            // Longitude data
            String lon = JSON.stringify(weather_data["coord"]["lon"]);
            print_data(30, 85, ST7735_CYAN, size::small, lon, "");

            // Latitude data
            String lat = JSON.stringify(weather_data["coord"]["lat"]);
            print_data(30, 100, ST7735_CYAN, size::small, lat, "");
            lastTime = millis();
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
    }

    // Time data
    print_time(20, 20, ST7735_BLUE, size::medium, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // Date data
    print_date(15, 38, ST7735_BLUE, size::small, tm.tm_mon, tm.tm_mday, tm.tm_year);

    // Wifi data
    String wifi = JSON.stringify(WiFi.RSSI());
    print_data(85, 85, ST7735_CYAN, size::small, wifi, "dBm");
    delay(1000);
}

void print_labels()
{
    tft.setTextSize(1);

    // System label
    tft.setTextColor(ST7735_RED);
    tft.setCursor(28, 5);
    tft.println("LUNAR LANDER");
    // Inside temp label
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(5, 55);
    tft.print("OUTSIDE: ");

    // Outside temp label
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(5, 70);
    tft.println("INSIDE: ??F");

    // Humidity label
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(90, 55);
    tft.println("HUMID");

    // Battery label
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(85, 100);
    tft.println("BATT");
    // Volt label
    tft.setTextColor(ST7735_CYAN);

    for (int i = 0; i < 10; i++)
    {
        tft.fillRect(2 + (i * 8), 112, 6, 14, ST7735_CYAN);
    }
    tft.setCursor(85, 115);
    tft.println("VOLT");
}

void draw_background()
{
    // Border
    tft.drawLine(0, 0, 0, tft.height(), ST7735_WHITE);
    tft.drawLine(0, 0, tft.height(), 0, ST7735_WHITE);
    tft.drawLine(tft.width() - 1, 0, tft.width() - 1, tft.height(), ST7735_WHITE);
    tft.drawLine(0, tft.height() - 1, tft.width(), tft.height() - 1, ST7735_WHITE);

    tft.drawLine(80, 50, 80, 110, ST7735_WHITE);
    tft.drawLine(26, 80, 26, 110, ST7735_WHITE);
    tft.drawLine(0, 50, 128, 50, ST7735_WHITE);
    tft.drawLine(0, 65, 80, 65, ST7735_WHITE);
    tft.drawLine(0, 80, 128, 80, ST7735_WHITE);
    tft.drawLine(0, 110, 128, 110, ST7735_WHITE);
    tft.drawLine(26, 95, 138, 95, ST7735_WHITE);
}

String intMonthToText(int num)
{
    switch (num)
    {
    case 1:
        return "Jan";
        break;
    case 2:
        return "Feb";
        break;
    case 3:
        return "Mar";
        break;
    case 4:
        return "Apr";
        break;
    case 5:
        return "May";
        break;
    case 6:
        return "Jun";
        break;
    case 7:
        return "Jul";
        break;
    case 8:
        return "Aug";
        break;
    case 9:
        return "Sep";
        break;
    case 10:
        return "Oct";
        break;
    case 11:
        return "Nov";
        break;
    case 12:
        return "Dec";
        break;
    default:
        return "Err";
        break;
    }
}

String httpGETRequest(const char *serverName)
{
    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0)
    {
        payload = http.getString();
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}

void print_time(int x_pos, int y_pos, int color, int size, int hour, int min, int sec)
{
    int16_t x1, y1;
    uint16_t w, h;
    String ampm = "AM";
    String addZeroHour = "";
    String addZeroMin = "";
    String fullTime;
    if (hour > 12)
    {
        hour = hour - 12;
        ampm = "PM";
    }

    if (hour < 10)
    {
        addZeroHour = "0";
    }

    if (min < 10)
    {
        addZeroMin = "0";
    }

    fullTime = addZeroHour + String(hour) + ":" + addZeroMin + String(min) + " " + ampm;
    tft.setTextSize(size);
    tft.getTextBounds(fullTime, x_pos, y_pos, &x1, &y1, &w, &h);
    print_data((tft.width() - w) / 2, y_pos, color, size, fullTime, "");
}

void print_date(int x_pos, int y_pos, int color, int size, int month, int day, int year)
{

    int16_t x1, y1;
    uint16_t w, h;
    String fullYear = intMonthToText(month + 1) + " " + String(day) + ", " + String(year + 1900);
    tft.setTextSize(size);
    tft.getTextBounds(fullYear.c_str(), x_pos, y_pos, &x1, &y1, &w, &h);
    print_data((tft.width() - w) / 2, y_pos, color, size, fullYear, "");
}

void print_data(int x_pos, int y_pos, int color, int size, String data, char *units)
{
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(data.c_str(), x_pos, y_pos, &x1, &y1, &w, &h);
    tft.fillRect(x1, y1, w, h, ST7735_BLACK);
    tft.setTextSize(size);
    tft.setTextColor(color);
    tft.setCursor(x_pos, y_pos);
    tft.print(data);
    tft.print(units);
}