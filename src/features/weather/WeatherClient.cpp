#include "WeatherClient.h"
#include "Platform.h"      // WiFiClient + HTTPClient (arch-correct headers)
#include <ArduinoJson.h>

static WeatherData g_wx;
static uint32_t    g_nextPollMs = 0;
static bool        g_inited = false;

// ---------------------------------------------------------------------------
void weatherInit(const Settings& s) {
  (void)s;
  g_wx.clear();
  g_nextPollMs = millis();
  g_inited = true;
}

void weatherForceRefresh() { g_nextPollMs = millis(); }

const WeatherData& weatherGet() { return g_wx; }

// ---- WMO weather_code -> short German condition (ASCII only) ----------------
const char* weatherCodeDE(int code) {
  switch (code) {
    case 0:  return "Klar";
    case 1:
    case 2:  return "Leicht bewoelkt";
    case 3:  return "Bewoelkt";
    case 45:
    case 48: return "Nebel";
    case 51:
    case 53:
    case 55:
    case 56:
    case 57: return "Niesel";
    case 61:
    case 63:
    case 65: return "Regen";
    case 66:
    case 67: return "Gefr. Regen";
    case 71:
    case 73:
    case 75:
    case 77: return "Schnee";
    case 80:
    case 81:
    case 82: return "Schauer";
    case 85:
    case 86: return "Schneeschauer";
    case 95:
    case 96:
    case 99: return "Gewitter";
    default: return "";
  }
}

// ---- parse: keep only the fields we render (bounds RAM on the ESP8266) ------
static bool applyWeatherDoc(WeatherData& d, JsonDocument& doc) {
  JsonObjectConst cur = doc["current"];
  if (!cur["temperature_2m"].is<float>() && !cur["temperature_2m"].is<int>())
    return false;   // no usable current reading -> treat as a failed fetch

  d.temp   = cur["temperature_2m"].as<float>();
  d.precip = cur["precipitation"] | 0.0f;
  d.code   = cur["weather_code"] | -1;

  d.nPts = 0;
  JsonArrayConst arr = doc["hourly"]["temperature_2m"];
  for (JsonVariantConst v : arr) {
    if (d.nPts >= WX_HOURLY_POINTS) break;
    d.trend[d.nPts++] = v.as<float>();
  }

  d.valid = true;
  d.error = false;
  d.lastOkMs = millis();
  return true;
}

static bool parseWeather(WeatherData& d, const String& body) {
  // Filter: only current.{temperature_2m,precipitation,weather_code} +
  // hourly.temperature_2m survive deserialization, so the doc stays tiny.
  JsonDocument filter;
  JsonObject fc = filter["current"].to<JsonObject>();
  fc["temperature_2m"] = true;
  fc["precipitation"]  = true;
  fc["weather_code"]   = true;
  filter["hourly"]["temperature_2m"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, body, DeserializationOption::Filter(filter))) return false;
  return applyWeatherDoc(d, doc);
}

// ---- one plain-HTTP GET + parse -------------------------------------------
static bool fetchWeather(const Settings& s) {
  const WeatherSettings& w = s.weather;

  char url[256];
  snprintf(url, sizeof(url),
           "http://" WX_API_HOST WX_API_PATH
           "?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,precipitation,weather_code"
           "&hourly=temperature_2m&forecast_hours=%d&timezone=auto"
           "&temperature_unit=%s",
           w.lat, w.lon, WX_HOURLY_POINTS, w.unitF ? "fahrenheit" : "celsius");

  WiFiClient client;   // plain HTTP: no TLS buffers, cheap on the ESP8266
  HTTPClient http;
  http.setTimeout(s.httpTimeout);
  http.setReuse(false);
  if (!http.begin(client, url)) return false;
  http.addHeader("Accept", "application/json");

  int code = http.GET();
  if (code != HTTP_CODE_OK) { http.end(); return false; }

  // Open-Meteo replies with Transfer-Encoding: chunked. HTTPClient::getString()
  // de-chunks into a String; feeding the raw getStream() to the JSON parser would
  // choke on the chunk-size lines. The body is tiny (~800 B), so this is cheap.
  String body = http.getString();
  http.end();
  return parseWeather(g_wx, body);
}

// ---------------------------------------------------------------------------
void weatherService(const Settings& s) {
  if (!g_inited) weatherInit(s);
  if ((int32_t)(millis() - g_nextPollMs) < 0) return;

  bool ok = fetchWeather(s);
  if (!ok) g_wx.error = true;   // keep the last good snapshot, flag the error

  // Reschedule: normal cadence on success, a faster retry after a failure.
  uint32_t waitSec = ok ? s.weather.refreshSec : WX_RETRY_SEC;
  g_nextPollMs = millis() + waitSec * 1000UL;
}
