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
#include "../lib/colors.h"

String intMonthToText(int num);
void oledDisplayCenter(String text);
void print_labels();
void draw_background();
void print_hour(uint8_t hours, uint16_t color);
void print_min(uint8_t minutes, uint16_t color);
void print_ampm(uint8_t seconds, uint16_t color);
String httpGETRequest(const char *serverName);
void print_time(int x_pos, int y_pos, int color, int bgcolor, int size, int hour, int min, int sec);
void print_date(int x_pos, int y_pos, int color, int bgcolor, int size, int month, int day, int year);
void print_data(int x_pos, int y_pos, int color, int bgcolor, int size, String data, char *units);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

String jsonBuffer;
JSONVar weather_data;
String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=" + units;
int i = 0;
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
    tft.fillScreen(BLACK);
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
    if (i > 9)
    {
        i = 0;
        tft.fillRect(2, 112, 78, 14, BLACK);
    }
    if ((millis() - lastTime) > timerDelay) // loop thru every 60 seconds
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

            // Outside temp data
            String out_temp = JSON.stringify(int(weather_data["main"]["temp"]));
            print_data(55, 55, WHITE, FIREBRICK, size::small, out_temp, "F");

            // Outside temp data
            String in_temp = JSON.stringify(int("99"));
            print_data(70, 55, WHITE, FIREBRICK, size::small, in_temp, "F");

            // Humid data
            String humid = JSON.stringify(weather_data["main"]["humidity"]);
            print_data(95, 70, WHITE, PURPLE, size::small, humid, "%");

            // Sattelites data
            String sattelite = "#27";
            print_data(5, 100, WHITE, MAGENTA, size::small, sattelite, "");

            // Longitude data
            String lon = JSON.stringify(weather_data["coord"]["lon"]);
            print_data(30, 85, WHITE, BLUE, size::small, lon, "");

            // Latitude data
            String lat = JSON.stringify(weather_data["coord"]["lat"]);
            print_data(30, 100, WHITE, BLUE, size::small, lat, "");
            lastTime = millis();
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
    }
    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);
    // Time data
    print_time(20, 20, WHITE, BLACK, size::medium, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // Date data
    print_date(15, 38, WHITE, BLACK, size::small, tm.tm_mon, tm.tm_mday, tm.tm_year);

    // Wifi data
    String wifi = JSON.stringify(WiFi.RSSI());
    print_data(85, 85, WHITE, BROWN, size::small, wifi, "dBm");

    // Battery loading test animation
    tft.fillRect(2 + (i * 8), 112, 6, 14, FORESTGREEN);
    i++;
    // Time label blinking animation
    tft.drawChar(40, 20, ':', WHITE, BLACK, 2);
    delay(500);
    tft.drawChar(40, 20, ':', BLACK, BLACK, 2);
    delay(500);
}

void print_labels()
{
    tft.setTextSize(1);
    // System label
    tft.setTextColor(WHITE);
    tft.setCursor(28, 5);
    tft.println("LUNAR LANDER");

    // Inside temp label
    tft.setTextColor(WHITE);
    tft.setCursor(5, 55);
    tft.print("OUTSIDE: ");

    // Outside temp label
    tft.setTextColor(WHITE);
    tft.setCursor(5, 70);
    tft.println("INSIDE: ??F");

    // Humidity label
    tft.setTextColor(WHITE);
    tft.setCursor(90, 55);
    tft.println("HUMID");

    // Satellite label
    tft.setTextColor(WHITE);
    tft.setCursor(5, 85);
    tft.println("SAT");
    // Battery label
    tft.setTextColor(WHITE);
    tft.setCursor(85, 100);
    tft.println("BATT");

    // Volt label
    tft.setTextColor(WHITE);
    tft.setCursor(85, 115);
    tft.println("VOLT");
}

void draw_background()
{
    tft.fillRect(0, 0, 128, 15, GOLD);           // System
    tft.fillRect(0, 50, 80, 15, FIREBRICK);      // Outside temp
    tft.fillRect(0, 65, 80, 15, DARKGOLDENROD);  // Inside temp
    tft.fillRect(80, 50, 48, 30, PURPLE);        // Humid
    tft.fillRect(80, 80, 48, 15, BROWN);         // Wifi
    tft.fillRect(80, 95, 48, 15, LIMEGREEN);     // Batt
    tft.fillRect(80, 110, 48, 16, NAVY);         // Volt
    tft.fillRect(0, 80, 26, 30, ST7735_MAGENTA); // ???
    tft.fillRect(26, 80, 54, 15, BLUE);          // Longitude
    tft.fillRect(26, 95, 54, 15, BLUE);          // Latitude
    // Border
    tft.drawLine(0, 0, 0, tft.height(), WHITE);
    tft.drawLine(0, 0, tft.height(), 0, WHITE);
    tft.drawLine(tft.width() - 1, 0, tft.width() - 1, tft.height(), WHITE);
    tft.drawLine(0, tft.height() - 1, tft.width(), tft.height() - 1, WHITE);

    tft.drawLine(80, 50, 80, 128, WHITE);
    tft.drawLine(26, 80, 26, 110, WHITE);

    tft.drawLine(0, 15, 128, 15, WHITE);
    tft.drawLine(0, 50, 128, 50, WHITE);
    tft.drawLine(0, 65, 80, 65, WHITE);
    tft.drawLine(0, 80, 128, 80, WHITE);
    tft.drawLine(0, 110, 128, 110, WHITE);
    tft.drawLine(26, 95, 128, 95, WHITE);
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

void print_time(int x_pos, int y_pos, int color, int bgcolor, int size, int hour, int min, int sec)
{
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

    tft.setTextSize(size);
    print_data(16, 20, color, bgcolor, size, addZeroHour + String(hour), "");
    print_data(52, 20, color, bgcolor, size, addZeroMin + String(min), "");
    print_data(84, 20, color, bgcolor, size, ampm, "");
}

void print_date(int x_pos, int y_pos, int color, int bgcolor, int size, int month, int day, int year)
{

    int16_t x1, y1;
    uint16_t w, h;
    String fullYear = intMonthToText(month + 1) + " " + String(day) + ", " + String(year + 1900);
    tft.setTextSize(size);
    tft.getTextBounds(fullYear.c_str(), x_pos, y_pos, &x1, &y1, &w, &h);
    print_data((tft.width() - w) / 2, y_pos, color, bgcolor, size, fullYear, "");
}

void print_data(int x_pos, int y_pos, int color, int bgcolor, int size, String data, char *units)
{
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(data.c_str(), x_pos, y_pos, &x1, &y1, &w, &h);
    tft.fillRect(x1, y1, w - 1, h - 1, bgcolor);
    tft.setTextSize(size);
    tft.setTextColor(color);
    tft.setCursor(x_pos, y_pos);
    tft.print(data);
    tft.print(units);
}