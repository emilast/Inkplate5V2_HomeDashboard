#include "DeparturesDisplay.h"
#include "../Utils/DisplayManager.h"
#include <HTTPClient.h>

const char *apiUrl = "https://transport.integration.sl.se/v1/sites/";

const int rightMargin = 25; // right margin on screen

void parseAndDisplayDepartures(Inkplate &display, const FontCollection &fonts, JsonDocument doc, const char *transportType, String title, int xpos, int startYpos, int lineBadgeWidth, bool useInformalTime, int row_height)
{
  int maxHeight = 320;
  int ypos = startYpos;
  const int rightPos = E_INK_WIDTH - rightMargin;

  // Clear area
  const int headerHeight = fonts.boldTextFont.yAdvance; // The header is printed with startYpos as baseline, so the etxt will apear above it
  display.fillRect(
    xpos,
    startYpos - headerHeight,
    // +3 to adjust for round errors etc when drawing fonts
    rightPos - xpos + 3,
    maxHeight + headerHeight,
    _WHITE);

  drawHeader(display, fonts.boldTextFont, title, xpos, ypos);

  JsonArray departures = doc["departures"];
  int rows = 0;

  // If no departures found, display a message
  if (departures.size() == 0)
  {
    ypos += row_height;
    display.setCursor(xpos, ypos);
    display.setFont(&fonts.normalTextFont);
    display.println("Inga avg\xe5ngar.");
    return;
  }

  for (JsonObject departure : departures)
  {
    const char *destination = departure["destination"];
    const int direction = departure["direction_code"];
    const char *displayTime = departure["display"]; // "17 min", "Now"
    const char *scheduledArrival = departure["scheduled"];
    const char *expectedArrival = departure["expected"];
    const char *line = departure["line"]["designation"];
    const char *transport_mode = departure["line"]["transport_mode"];
    const char *state = departure["journey"]["state"];

    if (strcmp(transport_mode, transportType) != 0)
    {
      continue; // Skip this departure if transport_mode is not matching the transportType
    }

    Serial.printf("Departure for %s: Line %s, %s, direction = %d\n", destination, line, displayTime, direction);

    char isoDestination[strlen(destination) + 1];
    utf8ToIso88591(destination, isoDestination);

    // Example usage
    int hour, minute;
    extractHourAndMinute(expectedArrival, hour, minute);

    int scheduledHour, scheduledMinute;
    extractHourAndMinute(scheduledArrival, scheduledHour, scheduledMinute);

    display.setCursor(xpos, ypos);

    char scheduledTimeBuffer[10];
    snprintf(scheduledTimeBuffer, sizeof(scheduledTimeBuffer), "%02d:%02d", scheduledHour, scheduledMinute);

    char timeBuffer[100];

    if (useInformalTime)
    {
      snprintf(timeBuffer, sizeof(timeBuffer), "%s", displayTime);
    }
    else
    {
      snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", hour, minute);
    }

    char buffer[100];

    const int marginToOriginalTime = 25;

    ypos += row_height;

    if (strcmp(state, "CANCELLED") == 0)
    {
      // Cancelled departure
      drawInvertedString(display, fonts.normalTextFont, line, xpos, ypos, lineBadgeWidth);

      display.printf(" %s", isoDestination);

      int w = drawRightString(display, fonts.normalTextFont, scheduledTimeBuffer, rightPos, ypos, true);
      drawRightString(display, fonts.normalTextFont, "INST\xc4LLD", rightPos - w - marginToOriginalTime, ypos, false);
      display.println();
    }
    else if ((hour != scheduledHour || minute != scheduledMinute) && (abs((hour * 60 + minute) - (scheduledHour * 60 + scheduledMinute)) > 1))
    {
      // Delayed departure (only if delay is more than 1 minute)
      drawInvertedString(display, fonts.normalTextFont, line, xpos, ypos, lineBadgeWidth);

      display.printf(" %s", isoDestination);

      int w = drawRightString(display, fonts.normalTextFont, scheduledTimeBuffer, rightPos, ypos, true);

      drawRightString(display, fonts.normalTextFont, timeBuffer, rightPos - w - marginToOriginalTime, ypos, false);
      display.println();
    }
    else
    {
      // On-time departure
      drawInvertedString(display, fonts.normalTextFont, line, xpos, ypos, lineBadgeWidth);

      display.printf(" %s", isoDestination);
      drawRightString(display, fonts.normalTextFont, timeBuffer, E_INK_WIDTH - rightMargin, ypos, false);

      display.println();
    }


    GFXfont deviationFont = fonts.smallTextFont;

    // Show deviation messages
    JsonArray deviations = departure["deviations"];
    for (JsonObject deviation : deviations)
    {
      const char *deviationMessage = deviation["message"];
      char isoDeviationMessage[strlen(deviationMessage) + 1];
      utf8ToIso88591(deviationMessage, isoDeviationMessage);

      display.setFont(&deviationFont);

      ypos += deviationFont.yAdvance;
      display.setCursor(xpos, ypos);
      display.printf("-");
      int linesDrawn = drawWrappedText(display, fonts.smallTextFont, isoDeviationMessage, xpos + 20, ypos, E_INK_WIDTH - xpos - 20 - rightMargin);

      ypos += (linesDrawn - 1) * deviationFont.yAdvance;
      rows += linesDrawn;
    }

    // Limit the number of rows
    if ( (ypos-startYpos) >= maxHeight)
    {
      break;
    }
  }
}

bool drawDepartures(Inkplate &display, const FontCollection &fonts, const char *siteId, const char *transportType, String title, int xpos, int startYpos, int lineBadgeWidth, bool useInformalTime, int row_height)
{
  bool success = false;
  HTTPClient http;
  http.useHTTP10(false);
  String requestUrl = String(apiUrl) + siteId + "/departures?transport=" + String(transportType) + "&nocache=" + String(random(1000, 9999));
  http.begin(requestUrl);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    JsonDocument doc;
    deserializeJson(doc, http.getStream());
    parseAndDisplayDepartures(display, fonts, doc, transportType, title, xpos, startYpos, lineBadgeWidth, useInformalTime, row_height);
    success = true;
  }
  else
  {
    // Show error message
    String response = http.getString();
    Serial.println("Error on HTTP request: " + String(httpCode) + " " + requestUrl + " " + response);
    display.setCursor(xpos, startYpos);
    display.setFont(&fonts.normalTextFont);
    display.print("Error on API call (");
    display.print(httpCode);
    display.println("):");

    startYpos += row_height;
    display.setCursor(xpos, startYpos);
    display.setFont(&fonts.smallTextFont);

    if (response.length() > 0) {
      String shortResponse = response.substring(0, 200);
      drawWrappedText(display, fonts.smallTextFont, shortResponse.c_str(), xpos, startYpos, E_INK_WIDTH - xpos - rightMargin);
    } else {
      display.print("(Empty response)");
    }
  }
  http.end();
  return success;
}
