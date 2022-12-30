#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <time.h>

#include "../lib/config.h"
#include "../lib/size.h"
enum position
{
    left,
    center,
    right,
    top
};

void print_text(byte x_pos, byte y_pos, char *text, byte text_size, uint16_t color, byte position);
char *intMonthToText(int num);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

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
    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

    tft.initR(INITR_144GREENTAB);
    tft.setRotation(2);

    configTime(timezone * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov");
    while (!time(nullptr))
    {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("");
}

void loop()
{
    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);

    tft.fillScreen(ST77XX_WHITE);
    char fullyear[50];

    // Add month to fullyear string
    strcpy(fullyear, intMonthToText(tm.tm_mon + 1));

    // Add day to fullyear string
    char day[2];
    itoa(tm.tm_mday, day, 10);
    strcat(fullyear, " ");
    strcat(fullyear, day);

    // Add year to fullyear string
    char year[4];
    itoa(tm.tm_year + 1900, year, 10);
    strcat(fullyear, ", ");
    strcat(fullyear, year);

    // Print fullyear string
    // print_text(20, 5, fullyear, size::small, ST77XX_BLACK, position::center);

    char hour[4];
    itoa(tm.tm_hour, hour, 10);
    char minute[4];
    itoa(tm.tm_min, minute, 10);
    char second[4];
    itoa(tm.tm_sec, second, 10);
    char time[50];
    strcpy(time, hour);
    strcat(time, ":");
    strcat(time, minute);
    print_text(0, 15, time, size::medium, ST77XX_BLACK, position::top);
    print_text(0, 15, time, size::medium, ST77XX_BLACK, position::center);
    print_text(20, 50, "TEST", size::medium, ST77XX_BLACK, position::left);
    print_text(20, 75, "TEST", size::small, ST77XX_BLACK, position::right);

    delay(timerDelay);
}

void print_text(byte x_pos, byte y_pos, char *text, byte text_size, uint16_t color, byte position)
{
    if (position == position::center)
    {
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds(text, x_pos, y_pos, &x1, &y1, &w, &h);
        Serial.print("x1: ");
        Serial.print(x1);
        Serial.print(" y1: ");
        Serial.print(y1);
        Serial.print(" w: ");
        Serial.print(w);
        Serial.print(" h: ");
        Serial.println(h);

        x_pos = (128 - w / 2);
    }
    else if (position == position::right)
    {
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds(text, x_pos, y_pos, &x1, &y1, &w, &h);
        x_pos = (tft.width() - w);
    }
    else if (position == position::left)
    {
        x_pos = 0;
    }
    tft.setCursor(x_pos, y_pos);
    tft.setTextSize(text_size);
    tft.setTextColor(color);
    tft.setTextWrap(true);
    tft.print(text);
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