#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <Arduino.h>

#define GRAPH_HOURS 24

class WeatherData {
public:
    // Member variables
    float currentTemp;
    float feelsLike;
    String sunrise;
    String sunset;
    float uvIndex;
    float precipitation;
    float windSpeed;
    float dailyMinTemp[7];
    float dailyMaxTemp[7];
    String dailyNames[7];
    String weatherDescription;
    int weatherCode;
    int dailyWeatherCodes[7];
    bool isDay;
    float hourlyTemps[GRAPH_HOURS];
    float hourlyPrecipMin[GRAPH_HOURS];
    float hourlyPrecip[GRAPH_HOURS];
    String hourlyTimes[GRAPH_HOURS];
};

#endif
