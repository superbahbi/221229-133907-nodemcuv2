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

void print_text(byte x_pos, byte y_pos, char *text, byte text_size, uint16_t color, byte position);
char *intMonthToText(int num);
void oledDisplayCenter(String text);
void print_labels();
void draw_background();
void print_hour(uint8_t hours, uint16_t color);
void print_min(uint8_t minutes, uint16_t color);
void print_ampm(uint8_t seconds, uint16_t color);
String httpGETRequest(const char *serverName);
void print_weather_data(int x_pos, int y_pos, int color, String data, char *units);

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
    Serial.println("");
    draw_background();
    print_labels();

    delay(1000);
    Serial.println("Timer set to 60 seconds (timerDelay variable), it will take 60 seconds before publishing the first reading.");
}

void loop()
{
    if ((millis() - lastTime) > timerDelay)
    {
        // Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED)
        {
            jsonBuffer = httpGETRequest(serverPath.c_str());
            // Serial.println(jsonBuffer);
            weather_data = JSON.parse(jsonBuffer);

            // JSON.typeof(jsonVar) can be used to get the type of the var
            if (JSON.typeof(weather_data) == "undefined")
            {
                Serial.println("Parsing input failed!");
                return;
            }

            Serial.print("Pressure: ");
            Serial.println(weather_data["main"]["pressure"]);
            Serial.print("Wind Speed: ");
            Serial.println(weather_data["wind"]["speed"]);

            // Inside temp data
            String temp = JSON.stringify(int(weather_data["main"]["temp"]));
            print_weather_data(55, 55, ST7735_CYAN, temp, "F");

            // Humid data
            String humid = JSON.stringify(weather_data["main"]["humidity"]);
            print_weather_data(95, 70, ST7735_CYAN, humid, "%");

            // Longitude data
            String lon = JSON.stringify(weather_data["coord"]["lon"]);
            print_weather_data(33, 85, ST7735_CYAN, lon, "");

            // Latitude data
            String lat = JSON.stringify(weather_data["coord"]["lat"]);
            print_weather_data(33, 100, ST7735_CYAN, lat, "");
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
        time_t t = time(nullptr);
        struct tm tm = *localtime(&t);

        tft.invertDisplay(false);

        tft.fillRect(15, 20, 100, 15, ST7735_BLACK);

        print_hour(tm.tm_hour, ST7735_BLUE);
        print_min(tm.tm_min, ST7735_BLUE);
        print_ampm(tm.tm_hour, ST7735_BLUE);

        tft.setTextColor(ST7735_BLUE);
        tft.setTextSize(1);
        tft.setCursor(15, 40);
        char fullYear[50];
        strcpy(fullYear, intMonthToText(tm.tm_mon + 1));
        strcat(fullYear, " ");
        strcat(fullYear, String(tm.tm_mday).c_str());
        strcat(fullYear, ", ");
        strcat(fullYear, String(tm.tm_year + 1900).c_str());
        tft.println(fullYear);

        lastTime = millis();
    }
}

void print_labels()
{
    tft.setTextSize(1);

    // System label
    tft.setTextColor(ST7735_RED);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds("LUNAR LANDER", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 5);
    tft.setTextSize(1);
    tft.println("LUNAR LANDER");

    tft.setTextSize(1);
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

    // Wifi label
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(85, 85);
    tft.println("WIFI");

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

    // Header background
    tft.fillRect(0, 0, 128, 15, ST7735_BLACK);
    // Border
    tft.drawLine(0, 0, 0, tft.height(), ST7735_WHITE);
    tft.drawLine(0, 0, tft.height(), 0, ST7735_WHITE);
    tft.drawLine(tft.width() - 1, 0, tft.width() - 1, tft.height(), ST7735_WHITE);
    tft.drawLine(0, tft.height() - 1, tft.width(), tft.height() - 1, ST7735_WHITE);

    tft.drawLine(80, 50, 80, 110, ST7735_WHITE);
    tft.drawLine(30, 80, 30, 110, ST7735_WHITE);
    tft.drawLine(0, 50, 128, 50, ST7735_WHITE);
    tft.drawLine(0, 65, 80, 65, ST7735_WHITE);
    tft.drawLine(0, 80, 128, 80, ST7735_WHITE);
    tft.drawLine(0, 110, 128, 110, ST7735_WHITE);
    tft.drawLine(30, 95, 138, 95, ST7735_WHITE);
}

void print_ampm(uint8_t h, uint16_t color)
{

    tft.setTextColor(color);
    tft.setTextSize(2);
    tft.setCursor(85, 20);
    if (h < 13)
    {
        tft.println("AM");
    }
    else
    {
        tft.println("PM");
    }
}

void print_min(uint8_t m, uint16_t color)
{

    tft.setTextColor(color);
    tft.setTextSize(2);
    // print colon followed by minutes
    tft.setCursor(45, 20);
    tft.println(":");

    if (m > 9)
    {
        tft.setCursor(55, 20);
        tft.println(m);
    }

    else
    {
        tft.setCursor(55, 20);
        tft.println("0");
        tft.setCursor(67, 20);
        tft.println(m);
    }
}

void print_hour(uint8_t h, uint16_t color)
{
    if (h > 12)
    {
        h = h - 12;
    }

    tft.setTextColor(color);
    tft.setTextSize(2);
    // print hour

    if (h > 9)
    {
        tft.setCursor(25, 20);
        tft.println(h);
    }
    else
    {
        tft.setCursor(20, 20);
        tft.println("0");
        tft.setCursor(35, 20);
        tft.println(h);
    }
}

char *intMonthToText(int num)
{
    switch (num)
    {
    case 1:
        return "Janaury";
        break;
    case 2:
        return "February";
        break;
    case 3:
        return "March";
        break;
    case 4:
        return "April";
        break;
    case 5:
        return "May";
        break;
    case 6:
        return "June";
        break;
    case 7:
        return "July";
        break;
    case 8:
        return "August";
        break;
    case 9:
        return "September";
        break;
    case 10:
        return "October";
        break;
    case 11:
        return "November";
        break;
    case 12:
        return "December";
        break;
    default:
        return "Error";
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
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
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

void print_weather_data(int x_pos, int y_pos, int color, String data, char *units)
{
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(x_pos, y_pos);
    tft.print(data);
    tft.print(units);
}