// WeatherData.h — the parsed weather snapshot the WeatherMode renders.
//
// Populated by WeatherClient from an Open-Meteo response (current + hourly + daily).
// Keeps the last good values so a failed fetch leaves the screen unchanged (error
// flag set); until the first success `valid` is false and the mode shows a neutral
// placeholder. Buffers are small and fixed so RAM stays bounded on the ESP8266.
#pragma once
#include <Arduino.h>
#include "config.h"

// One hourly sample (next-hours forecast).
struct WxHour {
  uint8_t hour;   // hour-of-day 0..23 (parsed from the "..Thh:.." time string)
  float   temp;   // temperature in the chosen unit
  uint8_t pop;    // precipitation probability, 0..100 %
  int16_t code;   // WMO weather code
};

// One daily sample.
struct WxDay {
  uint8_t wday;   // weekday index 0=Sun .. 6=Sat (computed from the date string)
  float   tmax;   // day's max temperature
  float   tmin;   // day's min temperature
  uint8_t popMax; // max precipitation probability, 0..100 %
  int16_t code;   // WMO weather code
};

struct WeatherData {
  bool     valid;     // at least one good fetch has landed
  bool     error;     // the most recent fetch failed
  uint32_t lastOkMs;  // millis() of the last good fetch

  // --- current conditions ---
  float    temp;      // temperature (chosen unit)
  float    feelsLike; // apparent temperature
  float    precip;    // precipitation (mm)
  float    wind;      // wind speed (chosen unit)
  uint8_t  humidity;  // relative humidity 0..100 %
  int16_t  code;      // WMO weather code
  bool     isDay;     // true = daytime (drives the sun/moon icon choice)

  // --- hourly (next hours) ---
  uint8_t  nHours;
  WxHour   hours[WX_HOURLY_POINTS];

  // --- daily ---
  uint8_t  nDays;
  WxDay    days[WX_DAILY_POINTS];

  void clear() {
    valid = false;
    error = false;
    lastOkMs = 0;
    temp = feelsLike = precip = wind = 0.0f;
    humidity = 0;
    code = -1;
    isDay = true;
    nHours = 0;
    nDays = 0;
    for (uint8_t i = 0; i < WX_HOURLY_POINTS; i++) { hours[i].hour = 0; hours[i].temp = 0.0f; hours[i].pop = 0; hours[i].code = -1; }
    for (uint8_t i = 0; i < WX_DAILY_POINTS;  i++) { days[i].wday = 0; days[i].tmax = days[i].tmin = 0.0f; days[i].popMax = 0; days[i].code = -1; }
  }
};

// WMO weather_code -> short German condition string (ASCII only: the fonts carry
// no umlauts, so "bewoelkt"/"Niesel"/"Gefr." stand in for the umlaut forms).
const char* weatherCodeDE(int code);
