/*
 * Modified by Lepikouze

 * 3D printed smart shelving with a giant hidden digital clock in the front edges of the shelves - DIY Machines

==========

More info and build instructions: https://www.youtube.com/watch?v=8E0SeycTzHw

3D printed parts can be downloaded from here: https://www.thingiverse.com/thing:4207524

You will need to install this librairies in the library manager:
  - the Adafruit Neopixel library
  - NTPClient
  - WiFiUdp
  - Timezone
  - ArduinoOTA

You have to install the Additional Boards ESP8266
Go to File > Preferences > Additional Boards Manager URLs

http://arduino.esp8266.com/stable/package_esp8266com_index.json

==========
*/

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <ESP8266WiFi.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h> 
#include <ArduinoOTA.h>
#include <Arduino.h>
#include "userconfig.h"

/////////////////////////////////////////////////
// Don't forget to replace var in userconfig.h
/////////////////////////////////////////////////

int buttonState = LOW;
int lastButtonState = LOW;

// Declare our NeoPixel objects:
Adafruit_NeoPixel stripClock(LEDCLOCK_COUNT, LEDCLOCK_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripDownlighter(LEDDOWNLIGHT_COUNT, LEDDOWNLIGHT_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

//Smoothing of the readings from the light sensor so it is not too twitchy
const int numReadings = 12;
int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
long total = 0;             // the running total
long average = 0;           // the average

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

/////////////////////////////////////////////////
/////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);

  // Initialize push button pin as input
  pinMode(buttonPin, INPUT_PULLUP); // Configurer la broche du bouton-poussoir en entrée avec résistance de pull-up interne

  // Set the hostname
  WiFi.hostname(hostname);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // show IP address
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  stripClock.begin();           // Initialize NeoPixel stripClock object (REQUIRED)
  stripClock.show();            // Turn OFF all pixels ASAP
  stripClock.setBrightness(100); // Set initial BRIGHTNESS (max = 255)

  stripDownlighter.begin();           // Initialize NeoPixel stripDownlighter object (REQUIRED)
  stripDownlighter.show();            // Turn OFF all pixels ASAP
  stripDownlighter.setBrightness(50); // Set BRIGHTNESS (max = 255)

  //smoothing
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  // Initialize the NTPClient
  timeClient.begin();
  timeClient.setTimeOffset(0); // Set the time zone offset (in seconds)

  // Initialize a ArduinoOTA
  ArduinoOTA.setHostname("ClockLED");

  // Paswword in md5 https://www.md5.fr/ default admin
  ArduinoOTA.setPasswordHash("OTA_PASSWORD");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("ArduinoOTA Ready");
  
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////

void loop() {

  // Update OTA
  ArduinoOTA.handle();

  unsigned long currentTime = millis();
  static unsigned long previousSyncTime = 0;

  // Calculate the date and time of the next synchronization
  unsigned long nextSyncTime = previousSyncTime + syncInterval;
  time_t utcNextSyncTime = timeClient.getEpochTime() + (nextSyncTime - previousSyncTime) / 1000;
  struct tm nextSyncTimeStruct;
  gmtime_r(&utcNextSyncTime, &nextSyncTimeStruct);

  // Calculate the time remaining before the next synchronization
  unsigned long timeRemaining = nextSyncTime - currentTime;
  unsigned long secondsRemaining = timeRemaining / 1000;
  unsigned long minutesRemaining = secondsRemaining / 60;
  unsigned long hoursRemaining = minutesRemaining / 60;

  // Display the time remaining before the next sync
  Serial.print("Temps restant avant la prochaine synchronisation : ");
  Serial.print(hoursRemaining);
  Serial.print(" heures, ");
  Serial.print(minutesRemaining % 60);
  Serial.print(" minutes, ");
  Serial.print(secondsRemaining % 60);
  Serial.println(" secondes");

  if (currentTime - previousSyncTime >= syncInterval) {
    if (WiFi.status() != WL_CONNECTED) {
      // If wifi is not ready
      Serial.println("WiFi connected");
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Reconnecting to WiFi...");
      }
      Serial.print("Connected to WiFi. IP address: ");
      Serial.println(WiFi.localIP());
    }
  if (!stayConnected) {
    // Si stayConnected is false
    // It's time to sync the time
    synchronizeTime();
    previousSyncTime = currentTime;

    delay(5000);
    WiFi.disconnect();
    Serial.println("WiFi disconnected");
  }
  else {
    // It's time to sync the time
    synchronizeTime();
    previousSyncTime = currentTime;
    }
  }
  // Update the time from the NTP server
  timeClient.update();

  // Display the time
  displayTheTime();

  //Record a reading from the light sensor and add it to the array
  readings[readIndex] = analogRead(A0);
  Serial.print("Light sensor value added to array = ");
  Serial.println(readings[readIndex]);
  readIndex = readIndex + 1;

  // if we're at the end of the array move the index back around...
  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  //now work out the sum of all the values in the array
  int sumBrightness = 0;
  for (int i = 0; i < numReadings; i++) {
    sumBrightness += readings[i];
  }
  Serial.print("Sum of the brightness array = ");
  Serial.println(sumBrightness);

  // and calculate the average:
  int lightSensorValue = sumBrightness / numReadings;
  Serial.print("Average light sensor value = ");
  Serial.println(lightSensorValue);

  //set the brightness based on ambiant light levels
  clockFaceBrightness = map(lightSensorValue, 50, 1000, 200, 1);
  stripClock.setBrightness(clockFaceBrightness);
  Serial.print("Mapped brightness value = ");
  Serial.println(clockFaceBrightness);

  //set 0 if clockFaceBrightness is -1 or -2 (correct bug)
  if (clockFaceBrightness == -1 || clockFaceBrightness == -2 || clockFaceBrightness == -3) {
      clockFaceBrightness = 0;
  } 
  stripClock.setBrightness(clockFaceBrightness); // Set brightness value of the LEDs
  Serial.print("Status brightness value = ");
  Serial.println(clockFaceBrightness);

  stripClock.show();

  //Start or shutoff Downlighter
  if (clockFaceBrightness > 125)
    {
      stripDownlighter.setBrightness(0);
      Serial.println("LED DECO OFF");
    }
     else 
    {
      if (clockFaceBrightness < 10)
      {
        stripDownlighter.setBrightness(0);
        Serial.println("LED DECO OFF");
      }  
     else
      {
        stripDownlighter.setBrightness(100);
        Serial.println("LED DECO ON"); 
      }
    }

   //(red * 65536) + (green * 256) + blue ->for 32-bit merged colour value so 16777215 equals white
  stripDownlighter.fill(16777215, 0, LEDDOWNLIGHT_COUNT);
  stripDownlighter.show();

  delay(5000);   //this 5 second delay to slow things down during testing

  // Lire l'état du bouton-poussoir
  int buttonState = digitalRead(buttonPin);

  // Si le bouton est enfoncé (l'état est LOW), mettez stayConnected à true
  if (buttonState == LOW) {
    stayConnected = true;
  }

}
/////////////////////////////////////////////////
/////////////////////////////////////////////////

void synchronizeTime() {
  // Update time from NTP server
  timeClient.forceUpdate();
  Serial.println("Heure synchronisée : " + timeClient.getFormattedTime());
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////

void displayTheTime() {
  stripClock.clear(); //clear the clock face 

  time_t utcTime = timeClient.getEpochTime();
  time_t localTime = myTZ.toLocal(utcTime);
  struct tm localTimeStruct;
  gmtime_r(&localTime, &localTimeStruct);

  int firstMinuteDigit = localTimeStruct.tm_min % 10; //work out the value of the first digit and then display it
  displayNumber(firstMinuteDigit, 0, clockMinuteColour);

  int secondMinuteDigit = localTimeStruct.tm_min / 10; //work out the value for the second digit and then display it
  displayNumber(secondMinuteDigit, 63, clockMinuteColour);



  int firstHourDigit = localTimeStruct.tm_hour; //work out the value for the third digit and then display it
  if (firstHourDigit > 12){
    firstHourDigit = firstHourDigit - 12;
  }
 
 // Comment out the following three lines if you want midnight to be shown as 12:00 instead of 0:00
 //  if (firstHourDigit == 0){
 //    firstHourDigit = 12;
 //  }
 
  firstHourDigit = firstHourDigit % 10;
  displayNumber(firstHourDigit, 126, clockHourColour);


  int secondHourDigit = localTimeStruct.tm_hour; //work out the value for the fourth digit and then display it

 // Comment out the following three lines if you want midnight to be shwon as 12:00 instead of 0:00
 //  if (secondHourDigit == 0){
 //    secondHourDigit = 12;
 //  }
 
 if (secondHourDigit > 12){
    secondHourDigit = secondHourDigit - 12;
  }
    if (secondHourDigit > 9){
      stripClock.fill(clockHourColour,189, 18); 
    }

  // Prints the current time, including seconds and date, to the console
  Serial.print("Heure actuelle : ");
  Serial.print(localTimeStruct.tm_hour);
  Serial.print(":");
  Serial.print(localTimeStruct.tm_min);
  Serial.print(":");
  Serial.print(localTimeStruct.tm_sec);
  Serial.print(" - Date : ");
  Serial.print(localTimeStruct.tm_mday);
  Serial.print("/");
  Serial.print(localTimeStruct.tm_mon + 1); // Note that tm_mon starts at 0
  Serial.print("/");
  Serial.print(localTimeStruct.tm_year + 1900); // Add 1900 to get the current year
  Serial.println();
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////

void displayNumber(int digitToDisplay, int offsetBy, int colourToUse) {
  switch (digitToDisplay) {
    case 0:
      digitZero(offsetBy, colourToUse);
      break;
    case 1:
      digitOne(offsetBy, colourToUse);
      break;
    case 2:
      digitTwo(offsetBy, colourToUse);
      break;
    case 3:
      digitThree(offsetBy, colourToUse);
      break;
    case 4:
      digitFour(offsetBy, colourToUse);
      break;
    case 5:
      digitFive(offsetBy, colourToUse);
      break;
    case 6:
      digitSix(offsetBy, colourToUse);
      break;
    case 7:
      digitSeven(offsetBy, colourToUse);
      break;
    case 8:
      digitEight(offsetBy, colourToUse);
      break;
    case 9:
      digitNine(offsetBy, colourToUse);
      break;
    default:
      break;
  }
}