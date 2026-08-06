// WeatherClient.h — fetches current weather + a 12h trend from Open-Meteo.
//
// Open-Meteo is queried over PLAIN HTTP (no API key, no TLS): the payload is a
// tiny fixed-shape JSON (~400-900 bytes) that we stream-parse with an ArduinoJson
// filter so only the handful of fields we render ever hit RAM. The device GETs on
// its refresh schedule (and immediately after a settings change); a failed fetch
// keeps the last good snapshot and just flags the error.
#pragma once
#include "Settings.h"
#include "WeatherData.h"

void weatherInit(const Settings& s);
void weatherService(const Settings& s);   // call each loop; fetches on the schedule
void weatherForceRefresh();               // fetch again on the next service() call
const WeatherData& weatherGet();
