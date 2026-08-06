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

// ---- small parse helpers ---------------------------------------------------
// Hour-of-day from an ISO time string "YYYY-MM-DDThh:mm" -> 0..23 (else 0).
static uint8_t hourFromISO(const char* t) {
  if (!t) return 0;
  const char* p = strchr(t, 'T');
  if (!p || !p[1] || !p[2]) return 0;
  int h = (p[1] - '0') * 10 + (p[2] - '0');
  return (h >= 0 && h < 24) ? (uint8_t)h : 0;
}

// Weekday (0=Sun..6=Sat) from a date string "YYYY-MM-DD" via Zeller's congruence.
// Independent of the device clock, so the daily strip labels stay correct even
// before NTP has synced. Returns 0 on a malformed string.
static uint8_t weekdayFromISO(const char* t) {
  if (!t || strlen(t) < 10) return 0;
  int y = (t[0]-'0')*1000 + (t[1]-'0')*100 + (t[2]-'0')*10 + (t[3]-'0');
  int m = (t[5]-'0')*10 + (t[6]-'0');
  int d = (t[8]-'0')*10 + (t[9]-'0');
  if (m < 1 || m > 12 || d < 1 || d > 31) return 0;
  if (m < 3) { m += 12; y -= 1; }
  int K = y % 100, J = y / 100;
  int h = (d + (13 * (m + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;  // 0=Sat..6=Fri
  return (uint8_t)((h + 6) % 7);   // -> 0=Sun..6=Sat (tm_wday convention)
}

// ---- parse: keep only the fields we render (bounds RAM on the ESP8266) ------
static bool applyWeatherDoc(WeatherData& d, JsonDocument& doc) {
  JsonObjectConst cur = doc["current"];
  if (!cur["temperature_2m"].is<float>() && !cur["temperature_2m"].is<int>())
    return false;   // no usable current reading -> treat as a failed fetch

  d.temp      = cur["temperature_2m"].as<float>();
  d.feelsLike = cur["apparent_temperature"] | d.temp;
  d.humidity  = (uint8_t)constrain((int)(cur["relative_humidity_2m"] | 0), 0, 100);
  d.precip    = cur["precipitation"] | 0.0f;
  d.wind      = cur["wind_speed_10m"] | 0.0f;
  d.code      = cur["weather_code"] | -1;
  d.isDay     = ((int)(cur["is_day"] | 1)) != 0;

  // hourly: zip the parallel arrays by index.
  d.nHours = 0;
  JsonObjectConst hr = doc["hourly"];
  JsonArrayConst  ht = hr["time"];
  JsonArrayConst  hT = hr["temperature_2m"];
  JsonArrayConst  hP = hr["precipitation_probability"];
  JsonArrayConst  hC = hr["weather_code"];
  for (size_t i = 0; i < hT.size() && d.nHours < WX_HOURLY_POINTS; i++) {
    WxHour& o = d.hours[d.nHours];
    o.hour = hourFromISO(ht[i] | (const char*)nullptr);
    o.temp = hT[i].as<float>();
    o.pop  = (uint8_t)constrain((int)(hP[i] | 0), 0, 100);
    o.code = hC[i] | -1;
    d.nHours++;
  }

  // daily.
  d.nDays = 0;
  JsonObjectConst dy = doc["daily"];
  JsonArrayConst  dt = dy["time"];
  JsonArrayConst  dW = dy["weather_code"];
  JsonArrayConst  dMx = dy["temperature_2m_max"];
  JsonArrayConst  dMn = dy["temperature_2m_min"];
  JsonArrayConst  dP = dy["precipitation_probability_max"];
  for (size_t i = 0; i < dMx.size() && d.nDays < WX_DAILY_POINTS; i++) {
    WxDay& o = d.days[d.nDays];
    o.wday   = weekdayFromISO(dt[i] | (const char*)nullptr);
    o.tmax   = dMx[i].as<float>();
    o.tmin   = dMn[i] | o.tmax;
    o.popMax = (uint8_t)constrain((int)(dP[i] | 0), 0, 100);
    o.code   = dW[i] | -1;
    d.nDays++;
  }

  d.valid = true;
  d.error = false;
  d.lastOkMs = millis();
  return true;
}

static bool parseWeather(WeatherData& d, const String& body) {
  // Filter: only the handful of fields we render survive deserialization, so the
  // doc stays small even though the response carries three forecast blocks.
  JsonDocument filter;
  JsonObject fc = filter["current"].to<JsonObject>();
  fc["temperature_2m"]       = true;
  fc["apparent_temperature"] = true;
  fc["relative_humidity_2m"] = true;
  fc["precipitation"]        = true;
  fc["weather_code"]         = true;
  fc["wind_speed_10m"]       = true;
  fc["is_day"]               = true;
  JsonObject fh = filter["hourly"].to<JsonObject>();
  fh["time"]                     = true;
  fh["temperature_2m"]           = true;
  fh["precipitation_probability"] = true;
  fh["weather_code"]             = true;
  JsonObject fd = filter["daily"].to<JsonObject>();
  fd["time"]                          = true;
  fd["weather_code"]                  = true;
  fd["temperature_2m_max"]            = true;
  fd["temperature_2m_min"]            = true;
  fd["precipitation_probability_max"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, body, DeserializationOption::Filter(filter))) return false;
  return applyWeatherDoc(d, doc);
}

// ---- one plain-HTTP GET + parse -------------------------------------------
static const char* windUnitStr(uint8_t u) {
  switch (u) {
    case WX_WIND_MPH: return "mph";
    case WX_WIND_MS:  return "ms";
    default:          return "kmh";
  }
}

static bool fetchWeather(const Settings& s) {
  const WeatherSettings& w = s.weather;

  char url[440];
  snprintf(url, sizeof(url),
           "http://" WX_API_HOST WX_API_PATH
           "?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,apparent_temperature,relative_humidity_2m,"
           "precipitation,weather_code,wind_speed_10m,is_day"
           "&hourly=temperature_2m,precipitation_probability,weather_code"
           "&daily=weather_code,temperature_2m_max,temperature_2m_min,"
           "precipitation_probability_max"
           "&forecast_hours=%d&forecast_days=%d&timezone=auto"
           "&temperature_unit=%s&wind_speed_unit=%s",
           w.lat, w.lon, WX_HOURLY_POINTS, WX_DAILY_POINTS,
           w.unitF ? "fahrenheit" : "celsius", windUnitStr(w.windUnit));

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
  // choke on the chunk-size lines. The body is small (~1.5 KB), so this is cheap.
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
