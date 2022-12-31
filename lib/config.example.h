const char *ssid = "";
const char *password = "";

#define TFT_CS D2
#define TFT_RST D2
#define TFT_DC D4

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

int timezone = -8; // Pacific Standard Time Los Angeles UTC-8
int dst = 0;

String city = "";                 // Paso Robles
String countryCode = "";          // us
String openWeatherMapApiKey = ""; // 2es0e95fdbd771510ae60e7b45a9aa4e
String units = "";                // imperial