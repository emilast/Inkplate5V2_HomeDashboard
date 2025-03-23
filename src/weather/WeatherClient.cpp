#include "WeatherClient.h"
#include <WiFi.h>          
#include <HTTPClient.h>     
#include "WeatherData.h"     

#include <ArduinoJson.h>     

#include <Inkplate.h>


// Function to get weather description based on the weather code
String WeatherClient::getWeatherDescription(int code) {
switch (code) {
        case 1: return String("Klart v\xE4""der"); // Clear sky
        case 2: return String("N\xE4""stan klart"); // Nearly clear sky
        case 3: return String("V\xE4""xlande molnighet"); // Variable cloudiness
        case 4: return String("Halvklart"); // Halfclear sky
        case 5: return String("Molnigt"); // Cloudy sky
        case 6: return String("Mulet"); // Overcast
        case 7: return String("Dimma"); // Fog
        case 8: return String("L\xE4""tta regnskurar"); // Light rain showers
        case 9: return String("M\xE5""ttliga regnskurar"); // Moderate rain showers
        case 10: return String("Kraftiga regnskurar"); // Heavy rain showers
        case 11: return String("\xC5""skv\xE4""der"); // Thunderstorm
        case 12: return String("L\xE4""tta bl\xF6""tsn\xF6""skurar"); // Light sleet showers
        case 13: return String("M\xE5""ttliga bl\xF6""tsn\xF6""skurar"); // Moderate sleet showers
        case 14: return String("Kraftiga bl\xF6""tsn\xF6""skurar"); // Heavy sleet showers
        case 15: return String("L\xE4""tta sn\xF6""byar"); // Light snow showers
        case 16: return String("M\xE5""ttliga sn\xF6""byar"); // Moderate snow showers
        case 17: return String("Kraftiga sn\xF6""byar"); // Heavy snow showers
        case 18: return String("L\xE4""tt regn"); // Light rain
        case 19: return String("M\xE5""ttligt regn"); // Moderate rain
        case 20: return String("Kraftigt regn"); // Heavy rain
        case 21: return String("\xC5""ska"); // Thunder
        case 22: return String("L\xE4""tt bl\xF6""tsn\xF6"); // Light sleet
        case 23: return String("M\xE5""ttlig bl\xF6""tsn\xF6"); // Moderate sleet
        case 24: return String("Kraftig bl\xF6""tsn\xF6"); // Heavy sleet
        case 25: return String("L\xE4""tt sn\xF6""fall"); // Light snowfall
        case 26: return String("M\xE5""ttligt sn\xF6""fall"); // Moderate snowfall
        case 27: return String("Kraftigt sn\xF6""fall"); // Heavy snowfall
        default: return String("Ok\xE4""nd v\xE4""dertyp: " + code); // Unknown weather type
    }
}

String WeatherClient::extractSun(String dateTime) {
  int tIndex = dateTime.indexOf('T');
  if (tIndex != -1 && tIndex + 5 < dateTime.length()) {
      return dateTime.substring(tIndex + 1, tIndex + 6);  // e.g., "06:11"
  }
  return "??:??";
}

// Function to get the current day of the week and return the next 7 days dynamically
String WeatherClient::getDayName(int dayIndex) {
  // Array of days, starting from Sunday (index 0)
  String daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  
  // Get the current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
      return "Error";  // Return error if time can't be fetched
  }

  // Get the current day index (0 = Sunday, 1 = Monday, etc.)
  int currentDay = timeinfo.tm_wday;

  // Calculate the day of the week for the given index
  int dayOfWeekIndex = (currentDay + dayIndex) % 7;

  return daysOfWeek[dayOfWeekIndex];
}


int getCurrentHour() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
      return -1;  // Return -1 if time is not available (error)
  }

  return timeinfo.tm_hour;
}


float windChill(float tempC, float windSpeedKph) {
    return 13.12 + 0.6215 * tempC - 11.37 * pow(windSpeedKph, 0.16) + 0.3965 * tempC * pow(windSpeedKph, 0.16);
}

float heatIndex(float tempC, float humidity) {
    float T = tempC;
    float R = humidity;

    return -8.784695 +
           1.61139411 * T +
           2.338549 * R +
          -0.14611605 * T * R +
          -0.012308094 * pow(T, 2) +
          -0.016424828 * pow(R, 2) +
           0.002211732 * pow(T, 2) * R +
           0.00072546 * T * pow(R, 2) +
          -0.000003582 * pow(T, 2) * pow(R, 2);
}

float feelsLikeC(float tempC, float humidity, float windSpeedMps) {
    float windKph = windSpeedMps * 3.6;

    if (tempC <= 10.0 && windKph > 4.8) {
        return windChill(tempC, windKph);
    } else if (tempC >= 27.0 && humidity >= 40.0) {
        return heatIndex(tempC, humidity);
    } else {
        return tempC;  // no correction
    }
}


// Function to fetch weather data from SMHI API
void WeatherClient::fetchWeatherData(WeatherData* weatherData, const float* latitude, const float* longitude) {
    String url = "https://opendata-download-metfcst.smhi.se/api/category/pmp3g/version/2/geotype/point/lon/" + 
                  String(*longitude, 6) + "/lat/" + String(*latitude, 6) + "/data.json";

    Serial.println("Fetching weather data from: " + url);

    HTTPClient http;
    http.useHTTP10(true); // Disable chunked transfer encoding for compatibility, https://arduinojson.org/v7/how-to/use-arduinojson-with-httpclient/#solution-1-use-http-10-recommended
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        JsonDocument doc;
        deserializeJson(doc, http.getStream());

        JsonArray timeSeries = doc["timeSeries"];
        int currentHour = getCurrentHour();
        int dataCount = timeSeries.size();

        Serial.printf("Found %d time series entries\n", dataCount);
        Serial.printf("Current hour: %d\n", currentHour);

        int forecastIndex = -1;
        time_t now;
        time(&now);
        struct tm *timeinfo = localtime(&now);

        char targetTime[20];
        sprintf(targetTime, "%04d-%02d-%02dT%02d:00:00Z", 
                timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour);

        // Find current hour forecast entry
        // for (int i = 0; i < dataCount; ++i) {
        //     Serial.printf("Checking timeSeries[%d]: %s (target: %s)\n", i, timeSeries[i]["validTime"].as<const char*>(), targetTime);
        //     if (strcmp(timeSeries[i]["validTime"].as<const char*>(), targetTime) == 0) {
        //         forecastIndex = i;
        //         break;
        //     }
        // }

        // if (forecastIndex == -1) {
        //     Serial.println("Could not find forecast for current hour");
        //     return;
        // }
        forecastIndex = 0; // Current hour is always first in SMHI API data

        // Extract current weather parameters
        JsonArray parameters = timeSeries[forecastIndex]["parameters"];

        for (JsonObject param : parameters) {
            String name = param["name"].as<String>();
            float value = param["values"][0].as<float>();

            if (name == "t") weatherData->currentTemp = value;
            else if (name == "ws") weatherData->windSpeed = value;
            else if (name == "pmax") weatherData->precipitation = value;
            else if (name == "Wsymb2") {
                weatherData->weatherCode = (int)value;
                weatherData->weatherDescription = getWeatherDescription(weatherData->weatherCode);
            }
        }


        // Set dummy values or handle sunrise/sunset manually via another API if needed
        weatherData->sunrise = "06:00";
        weatherData->sunset = "18:00";
        weatherData->uvIndex = -1;  // Not provided
        weatherData->feelsLike = feelsLikeC(weatherData->currentTemp, weatherData->precipitation, weatherData->windSpeed);
        weatherData->isDay = true;

        // Fill in forecast graph values (next GRAPH_HOURS hours)
        for (int i = 0; i < GRAPH_HOURS && (forecastIndex + i) < dataCount; i++) {
            JsonArray forecastParams = timeSeries[forecastIndex + i]["parameters"];
            for (JsonObject param : forecastParams) {
                String name = param["name"].as<String>();
                float value = param["values"][0].as<float>();

                if (name == "t") weatherData->hourlyTemps[i] = value;
                else if (name == "pmin") weatherData->hourlyPrecipMin[i] = value;
                else if (name == "pmax") weatherData->hourlyPrecip[i] = value;
            }

            String timeStr = timeSeries[forecastIndex + i]["validTime"].as<String>();
            weatherData->hourlyTimes[i] = extractSun(timeStr);
        }

        // Note: Daily values not directly supported. You may need to compute these from hourly data.

    } else {
        Serial.println("Error on HTTP request");
    }

    http.end();
}
