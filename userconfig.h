/////////////////////////////////////////////////
/////////////////////////////////////////////////
// Here are all variables necessary to work
/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Replace your board Hostname
// This the name appear on WIFI
const char* hostname = "ClockLED";

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Replace with your network credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Password OTA must be in MD5 https://www.md5.fr/ default is admin = 21232f297a57a5a743894a0e4a801fc3
const char* OTA_PASSWORD = "21232f297a57a5a743894a0e4a801fc3";

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Define the pins for the NeoPixels
#define LEDCLOCK_PIN D6 // You can change this pin
#define LEDDOWNLIGHT_PIN D5 // You can change this pin

/////////////////////////////////////////////////

// How many NeoPixels are attached to the NodeMCU8266?
#define LEDCLOCK_COUNT 216 // You can change the amount of led
#define LEDDOWNLIGHT_COUNT 12

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Button for wifi
const int buttonPin = D7; // You can change this pin

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Define your NeoPixel colors
uint32_t clockMinuteColour = Adafruit_NeoPixel::Color(50, 50, 255); // Blue color
uint32_t clockHourColour = Adafruit_NeoPixel::Color(255, 0, 0); // Red color
// If needed here are other colors
// Color(255, 0, 0); // Red
// Color(0, 255, 0); // Green
// Color(0, 0, 255); // Bleu
// Color(255, 255, 0); // Jaune
// Color(128, 0, 128); // Violet
// Color(255, 165, 0); // Orange

/////////////////////////////////////////////////
/////////////////////////////////////////////////

int clockFaceBrightness = 0;

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Define your time zone
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Last sunday of march at 2h am (Central European Summer Time)
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 0};     // Last sunday of october at 3h am (Central European Time)
Timezone myTZ(CEST, CET);

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Define if the wifi should stop after each NTP Sync
// If value is true, the nodeMCU will stay connected
// If value is false the nodeMCU will disconnect after sync to NTP server every 2hours (Depend of the next variable)
bool stayConnected = true; // true always on, false to disconnect after sync

/////////////////////////////////////////////////
/////////////////////////////////////////////////

// Interval time to sync
unsigned long syncInterval = 7200000; // here 2 hours
// 1 minute = 60000
// 1 hour   = 3600000
/////////////////////////////////////////////////