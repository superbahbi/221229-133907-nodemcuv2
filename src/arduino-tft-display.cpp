#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "../lib/config.h"

void print_text(byte x_pos, byte y_pos, char *text, byte text_size, uint16_t color);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup(void)
{
    Serial.begin(9600);

    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

    tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
    tft.setRotation(2);           // set display orientation
}

void loop()
{
    tft.fillScreen(ST77XX_WHITE);
    print_text(20, 25, "PISOT", 3, ST77XX_BLACK);
    print_text(20, 50, "SI!", 3, ST77XX_BLACK);
    print_text(20, 75, "LAU!", 3, ST77XX_BLACK);
    delay(5000);
}

void print_text(byte x_pos, byte y_pos, char *text, byte text_size, uint16_t color)
{
    tft.setCursor(x_pos, y_pos);
    tft.setTextSize(text_size);
    tft.setTextColor(color);
    tft.setTextWrap(true);
    tft.print(text);
}