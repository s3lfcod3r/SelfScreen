// WeatherData.h — the parsed weather snapshot the WeatherMode renders.
//
// Populated by WeatherClient from an Open-Meteo response. Keeps the last good
// values so a failed fetch leaves the screen unchanged (error flag set); until
// the first success `valid` is false and the mode shows a neutral placeholder.
#pragma once
#include <Arduino.h>
#include "config.h"

struct WeatherData {
  bool     valid;                    // at least one good fetch has landed
  bool     error;                    // the most recent fetch failed
  uint32_t lastOkMs;                 // millis() of the last good fetch
  float    temp;                     // current temperature (in the chosen unit)
  float    precip;                   // current precipitation (mm)
  int      code;                     // WMO weather code
  uint8_t  nPts;                     // number of valid trend points
  float    trend[WX_HOURLY_POINTS];  // next 12 hourly temperatures

  void clear() {
    valid = false;
    error = false;
    lastOkMs = 0;
    temp = 0.0f;
    precip = 0.0f;
    code = -1;
    nPts = 0;
    for (uint8_t i = 0; i < WX_HOURLY_POINTS; i++) trend[i] = 0.0f;
  }
};

// WMO weather_code -> short German condition string (ASCII only: the fonts carry
// no umlauts, so "bewoelkt"/"Niesel"/"Gefr." stand in for the umlaut forms).
const char* weatherCodeDE(int code);
