/*
   Inkplate5V2_Read_Temperature example for Soldered Inkplate 5 V2
   For this example you will need a USB-C cable and Inkplate 5 V2.
   Select "Soldered Inkplate5 V2" from Tools -> Board menu.
   Don't have "Soldered Inkplate5 V2" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   This example will show you how to read temperature from on-board
   temperature sensor which is part of TPSrow_height186 e-paper PMIC.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   15 April 2024 by Soldered

   In order to convert your images into a format compatible with Inkplate
   use the Soldered Image Converter available at:
   https://github.com/SolderedElectronics/Soldered-Image-Converter/releases
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE5V2
#error "Wrong board selection for this example, please select Soldered Inkplate5 V2 in the boards menu."
#endif

#include "Inkplate.h"   // Include Inkplate library to the sketch
#include "battSymbol.h" // Include .h file that contains byte array for battery symbol.

// It is in same folder as this sketch. You can even open it (read it) by clicking on tempSymbol.h tab in Arduino IDE
Inkplate display(INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1-bit mode (BW)

#include "HTTPClient.h" //Include library for HTTPClient
#include <ArduinoJson.h>
#include <WiFiManager.h>

#include "constants.h"
#include "Utils/utils.h"
#include "Utils/DisplayManager.h"

#include "Clock/TimeUtils.h"
#include "Weather/WeatherDisplay.h"
#include "PublicTransport/DeparturesDisplay.h"

#include "Fonts/AcariSansbd12.h"
#include "Fonts/AcariSans20.h"
#include "Fonts/AcariSansbd20.h"
#include "Fonts/AcariSansbd50.h"

static const FontCollection fonts = {
  AcariSans_Regular20pt8b,
  AcariSans_Bold20pt8b,
  AcariSans_Bold50pt7b,
  AcariSans_Bold12pt8b
};

// Time config
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const int jsonBufferSize = 8192;

// Trafiklab API details
const char *busSiteId = "7083";
const char *busTitle = "Buss fr\xe5n Sn\xe4ttringe";

const char *trainSiteId = "9528";
const char *trainTitle = "T\xe5g fr\xe5n Stuvsta";

const char *TrainType = "TRAIN";
const char *BusType = "BUS";

// Layout constants
const int leftXpos = 35;    // Left column start position
const int rightXpos = 430;  // right column start position

const int row_height = 50;

const int busLineWidth = 80;
const int trainLineWidth = 60;

const int refreshInterval = 20000;
int framesDrawn = 0;

void setup()
{
  Serial.begin(115200);
  WiFiManager wm;
  bool res = wm.autoConnect("Inkplate5V2 Home Dashboard");
  if (!res)
  {
    Serial.println("Failed to connect");
  }
  else
  {
    Serial.println("Connected to WiFi.");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  initializeDisplay(display, fonts);
}

void loop()
{
  clearAndPrepareDisplay(display);
  
  printLocalTime(display, fonts, leftXpos, 100);
  
  drawWeather(display, fonts, leftXpos, 180, row_height, framesDrawn);
  
  Serial.println("Bus information from API...");
  if (!drawDepartures(display, fonts, busSiteId, BusType, busTitle, rightXpos, 65, busLineWidth, true, row_height))
  {
    drawDepartures(display, fonts, busSiteId, BusType, busTitle, rightXpos, 65, busLineWidth, true, row_height);
  }

  if (!drawDepartures(display, fonts, trainSiteId, TrainType, trainTitle, rightXpos, 385, trainLineWidth, false, row_height))
  {
    drawDepartures(display, fonts, trainSiteId, TrainType, trainTitle, rightXpos, 385, trainLineWidth, false, row_height);
  }
  
  if (framesDrawn == 0)
  {
    display.display();
  }
  else
  {
    display.partialUpdate();
  }
  framesDrawn = (++framesDrawn % 10);
  
  Serial.printf("Waiting for next update in %d seconds...\n", refreshInterval / 1000);
  delay(refreshInterval);
}

