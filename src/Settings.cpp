#include "Settings.h"
#include "Platform.h"   // platformChipId() for the unique default hostname
#include <LittleFS.h>

static const char* CONFIG_PATH = "/config.json";

// ===========================================================================
// Usage slice
// ===========================================================================
void UsageSettings::setDefaults() {
  usageUrl = "";
  pollSec = DEFAULT_POLL_SEC;
}

void UsageSettings::toJson(JsonObject o) const {
  o["usageUrl"] = usageUrl;
  o["pollSec"]  = pollSec;
}

void UsageSettings::fromJson(JsonObjectConst o) {
  if (o["usageUrl"].is<const char*>()) usageUrl = o["usageUrl"].as<String>();
  if (o["pollSec"].is<int>())          pollSec = max(10, (int)o["pollSec"]);
}

// ===========================================================================
// Clock face slice (MODE_CLOCK display options)
// ===========================================================================
void ClockFaceSettings::setDefaults() {
  hour24       = DEFAULT_CLK_24H;
  showSeconds  = DEFAULT_CLK_SECONDS;
  showWeekday  = DEFAULT_CLK_WEEKDAY;
  dateFormat   = DEFAULT_CLK_DATEFMT;
  timeColor    = DEFAULT_CLK_TIMECOLOR;
  dateColor    = DEFAULT_CLK_DATECOLOR;
  weekdayColor = DEFAULT_CLK_WEEKDAYCOLOR;
  lineColor    = DEFAULT_CLK_LINECOLOR;
  timeSize     = DEFAULT_CLK_TIMESIZE;
  weekdaySize  = DEFAULT_CLK_WEEKDAYSIZE;
  dateSize     = DEFAULT_CLK_DATESIZE;
}

void ClockFaceSettings::toJson(JsonObject o) const {
  o["hour24"]       = hour24;
  o["showSeconds"]  = showSeconds;
  o["showWeekday"]  = showWeekday;
  o["dateFormat"]   = dateFormat;
  o["timeColor"]    = timeColor;
  o["dateColor"]    = dateColor;
  o["weekdayColor"] = weekdayColor;
  o["lineColor"]    = lineColor;
  o["timeSize"]     = timeSize;
  o["weekdaySize"]  = weekdaySize;
  o["dateSize"]     = dateSize;
}

void ClockFaceSettings::fromJson(JsonObjectConst o) {
  if (o["hour24"].is<bool>())       hour24 = o["hour24"];
  if (o["showSeconds"].is<bool>())  showSeconds = o["showSeconds"];
  if (o["showWeekday"].is<bool>())  showWeekday = o["showWeekday"];
  if (o["dateFormat"].is<int>())    dateFormat = (uint8_t)constrain((int)o["dateFormat"], 0, CLK_DATE_MAX);
  if (o["timeColor"].is<int>())     timeColor = (uint8_t)constrain((int)o["timeColor"], 0, CLK_COL_MAX);
  if (o["dateColor"].is<int>())     dateColor = (uint8_t)constrain((int)o["dateColor"], 0, CLK_COL_MAX);
  if (o["weekdayColor"].is<int>())  weekdayColor = (uint8_t)constrain((int)o["weekdayColor"], 0, CLK_COL_MAX);
  if (o["lineColor"].is<int>())     lineColor = (uint8_t)constrain((int)o["lineColor"], 0, CLK_COL_MAX);
  if (o["timeSize"].is<int>())      timeSize = (uint8_t)constrain((int)o["timeSize"], 0, CLK_NUM_FONT_MAX);
  if (o["weekdaySize"].is<int>())   weekdaySize = (uint8_t)constrain((int)o["weekdaySize"], 0, CLK_PROP_FONT_MAX);
  if (o["dateSize"].is<int>())      dateSize = (uint8_t)constrain((int)o["dateSize"], 0, CLK_PROP_FONT_MAX);
  // Back-compat: pre-slider firmware stored a single bool "bigSize". If that is
  // all the config carries, keep the current look (defaults already applied) —
  // small was the proportional font, which no longer maps cleanly, so we simply
  // do not crash and leave the size sliders at their defaults.
}

// ===========================================================================
// Weather slice (MODE_WEATHER display options)
// ===========================================================================
void WeatherSettings::setDefaults() {
  lat          = DEFAULT_WX_LAT;
  lon          = DEFAULT_WX_LON;
  unitF        = DEFAULT_WX_UNITF;
  windUnit     = DEFAULT_WX_WINDUNIT;
  precipMode   = DEFAULT_WX_PRECIPMODE;
  iconColorMode= DEFAULT_WX_ICONCOLORMODE;
  refreshSec   = DEFAULT_WX_REFRESH;

  showTemp     = DEFAULT_WX_SHOWTEMP;
  showBigIcon  = DEFAULT_WX_SHOWBIGICON;
  showCond     = DEFAULT_WX_SHOWCOND;
  showFeels    = DEFAULT_WX_SHOWFEELS;
  showHum      = DEFAULT_WX_SHOWHUM;
  showWind     = DEFAULT_WX_SHOWWIND;
  showPrecip   = DEFAULT_WX_SHOWPRECIP;
  showHourly   = DEFAULT_WX_SHOWHOURLY;
  showDaily    = DEFAULT_WX_SHOWDAILY;
  showTrend    = DEFAULT_WX_SHOWTREND;

  hourlyCount  = DEFAULT_WX_HOURLYCOUNT;
  hourlyStep   = DEFAULT_WX_HOURLYSTEP;
  hrHour       = DEFAULT_WX_HR_HOUR;
  hrIcon       = DEFAULT_WX_HR_ICON;
  hrTemp       = DEFAULT_WX_HR_TEMP;
  hrPop        = DEFAULT_WX_HR_POP;

  dailyCount   = DEFAULT_WX_DAILYCOUNT;
  dyDay        = DEFAULT_WX_DY_DAY;
  dyIcon       = DEFAULT_WX_DY_ICON;
  dyTemps      = DEFAULT_WX_DY_TEMPS;
  dyPop        = DEFAULT_WX_DY_POP;

  trendLabels  = DEFAULT_WX_TRENDLABELS;

  tempSize     = DEFAULT_WX_TEMPSIZE;
  condSize     = DEFAULT_WX_CONDSIZE;
  detailSize   = DEFAULT_WX_DETAILSIZE;
  hourlySize   = DEFAULT_WX_HOURLYSIZE;
  dailySize    = DEFAULT_WX_DAILYSIZE;

  tempColor    = DEFAULT_WX_TEMPCOLOR;
  bigIconColor = DEFAULT_WX_BIGICONCOLOR;
  condColor    = DEFAULT_WX_CONDCOLOR;
  detailColor  = DEFAULT_WX_DETAILCOLOR;
  hourlyColor  = DEFAULT_WX_HOURLYCOLOR;
  hourlyPop    = DEFAULT_WX_HOURLYPOP;
  dailyColor   = DEFAULT_WX_DAILYCOLOR;
  dailyPop     = DEFAULT_WX_DAILYPOP;
  trendColor   = DEFAULT_WX_TRENDCOLOR;
}

void WeatherSettings::toJson(JsonObject o) const {
  o["lat"]          = lat;
  o["lon"]          = lon;
  o["unitF"]        = unitF;
  o["windUnit"]     = windUnit;
  o["precipMode"]   = precipMode;
  o["iconColorMode"]= iconColorMode;
  o["refreshSec"]   = refreshSec;

  o["showTemp"]     = showTemp;
  o["showBigIcon"]  = showBigIcon;
  o["showCond"]     = showCond;
  o["showFeels"]    = showFeels;
  o["showHum"]      = showHum;
  o["showWind"]     = showWind;
  o["showPrecip"]   = showPrecip;
  o["showHourly"]   = showHourly;
  o["showDaily"]    = showDaily;
  o["showTrend"]    = showTrend;

  o["hourlyCount"]  = hourlyCount;
  o["hourlyStep"]   = hourlyStep;
  o["hrHour"]       = hrHour;
  o["hrIcon"]       = hrIcon;
  o["hrTemp"]       = hrTemp;
  o["hrPop"]        = hrPop;

  o["dailyCount"]   = dailyCount;
  o["dyDay"]        = dyDay;
  o["dyIcon"]       = dyIcon;
  o["dyTemps"]      = dyTemps;
  o["dyPop"]        = dyPop;

  o["trendLabels"]  = trendLabels;

  o["tempSize"]     = tempSize;
  o["condSize"]     = condSize;
  o["detailSize"]   = detailSize;
  o["hourlySize"]   = hourlySize;
  o["dailySize"]    = dailySize;

  o["tempColor"]    = tempColor;
  o["bigIconColor"] = bigIconColor;
  o["condColor"]    = condColor;
  o["detailColor"]  = detailColor;
  o["hourlyColor"]  = hourlyColor;
  o["hourlyPop"]    = hourlyPop;
  o["dailyColor"]   = dailyColor;
  o["dailyPop"]     = dailyPop;
  o["trendColor"]   = trendColor;
}

void WeatherSettings::fromJson(JsonObjectConst o) {
  if (o["lat"].is<float>())        lat = constrain((float)o["lat"], -90.0f, 90.0f);
  if (o["lon"].is<float>())        lon = constrain((float)o["lon"], -180.0f, 180.0f);
  if (o["unitF"].is<bool>())       unitF = o["unitF"];
  if (o["windUnit"].is<int>())     windUnit = (uint8_t)constrain((int)o["windUnit"], 0, WX_WIND_MAX);
  // precip display mode (mm/%/both). Migrate the old boolean precipPct if present
  // and no new key exists: false -> mm, true -> %.
  if (o["precipMode"].is<int>())      precipMode = (uint8_t)constrain((int)o["precipMode"], 0, WX_PRECIP_MAX);
  else if (o["precipPct"].is<bool>()) precipMode = o["precipPct"] ? WX_PRECIP_PCT : WX_PRECIP_MM;
  if (o["iconColorMode"].is<int>()) iconColorMode = (uint8_t)constrain((int)o["iconColorMode"], 0, WX_ICONCOL_MAX);
  if (o["refreshSec"].is<int>())   refreshSec = (uint16_t)constrain((int)o["refreshSec"], WX_REFRESH_MIN, WX_REFRESH_MAX);

  if (o["showTemp"].is<bool>())    showTemp = o["showTemp"];
  if (o["showBigIcon"].is<bool>()) showBigIcon = o["showBigIcon"];
  if (o["showCond"].is<bool>())    showCond = o["showCond"];
  if (o["showFeels"].is<bool>())   showFeels = o["showFeels"];
  if (o["showHum"].is<bool>())     showHum = o["showHum"];
  if (o["showWind"].is<bool>())    showWind = o["showWind"];
  if (o["showPrecip"].is<bool>())  showPrecip = o["showPrecip"];
  if (o["showHourly"].is<bool>())  showHourly = o["showHourly"];
  if (o["showDaily"].is<bool>())   showDaily = o["showDaily"];
  if (o["showTrend"].is<bool>())   showTrend = o["showTrend"];

  if (o["hourlyCount"].is<int>())  hourlyCount = (uint8_t)constrain((int)o["hourlyCount"], WX_HOURLY_COUNT_MIN, WX_HOURLY_COUNT_MAX);
  if (o["hourlyStep"].is<int>())   hourlyStep = (uint8_t)constrain((int)o["hourlyStep"], WX_HOURLY_STEP_MIN, WX_HOURLY_STEP_MAX);
  if (o["hrHour"].is<bool>())      hrHour = o["hrHour"];
  if (o["hrIcon"].is<bool>())      hrIcon = o["hrIcon"];
  if (o["hrTemp"].is<bool>())      hrTemp = o["hrTemp"];
  if (o["hrPop"].is<bool>())       hrPop = o["hrPop"];

  if (o["dailyCount"].is<int>())   dailyCount = (uint8_t)constrain((int)o["dailyCount"], WX_DAILY_COUNT_MIN, WX_DAILY_COUNT_MAX);
  if (o["dyDay"].is<bool>())       dyDay = o["dyDay"];
  if (o["dyIcon"].is<bool>())      dyIcon = o["dyIcon"];
  if (o["dyTemps"].is<bool>())     dyTemps = o["dyTemps"];
  if (o["dyPop"].is<bool>())       dyPop = o["dyPop"];

  if (o["trendLabels"].is<bool>()) trendLabels = o["trendLabels"];

  if (o["tempSize"].is<int>())     tempSize = (uint8_t)constrain((int)o["tempSize"], 0, CLK_NUM_FONT_MAX);
  if (o["condSize"].is<int>())     condSize = (uint8_t)constrain((int)o["condSize"], 0, CLK_PROP_FONT_MAX);
  if (o["detailSize"].is<int>())   detailSize = (uint8_t)constrain((int)o["detailSize"], 0, CLK_PROP_FONT_MAX);
  if (o["hourlySize"].is<int>())   hourlySize = (uint8_t)constrain((int)o["hourlySize"], 0, CLK_PROP_FONT_MAX);
  if (o["dailySize"].is<int>())    dailySize = (uint8_t)constrain((int)o["dailySize"], 0, CLK_PROP_FONT_MAX);

  if (o["tempColor"].is<int>())    tempColor = (uint8_t)constrain((int)o["tempColor"], 0, WX_COL_MAX);
  if (o["bigIconColor"].is<int>()) bigIconColor = (uint8_t)constrain((int)o["bigIconColor"], 0, WX_COL_MAX);
  if (o["condColor"].is<int>())    condColor = (uint8_t)constrain((int)o["condColor"], 0, WX_COL_MAX);
  if (o["detailColor"].is<int>())  detailColor = (uint8_t)constrain((int)o["detailColor"], 0, WX_COL_MAX);
  if (o["hourlyColor"].is<int>())  hourlyColor = (uint8_t)constrain((int)o["hourlyColor"], 0, WX_COL_MAX);
  if (o["hourlyPop"].is<int>())    hourlyPop = (uint8_t)constrain((int)o["hourlyPop"], 0, WX_COL_MAX);
  if (o["dailyColor"].is<int>())   dailyColor = (uint8_t)constrain((int)o["dailyColor"], 0, WX_COL_MAX);
  if (o["dailyPop"].is<int>())     dailyPop = (uint8_t)constrain((int)o["dailyPop"], 0, WX_COL_MAX);
  if (o["trendColor"].is<int>())   trendColor = (uint8_t)constrain((int)o["trendColor"], 0, WX_COL_MAX);
}

// ===========================================================================
// Clock / night mode slice
// ===========================================================================
static uint16_t hhmmToMin(const char* s, uint16_t fallback) {
  if (!s || !s[0]) return fallback;
  int h = 0, m = 0;
  if (sscanf(s, "%d:%d", &h, &m) != 2) return fallback;
  if (h < 0 || h > 23 || m < 0 || m > 59) return fallback;
  return (uint16_t)(h * 60 + m);
}
static String minToHhmm(uint16_t v) {
  if (v > 1439) v = 0;
  char b[6];
  snprintf(b, sizeof(b), "%02u:%02u", (unsigned)(v / 60), (unsigned)(v % 60));
  return String(b);
}

void ClockSettings::setDefaults() {
  tz            = DEFAULT_TZ_NAME;
  tzPosix       = DEFAULT_TZ_POSIX;
  nightEnabled  = DEFAULT_NIGHT_ENABLED;
  nightStartMin = DEFAULT_NIGHT_START_MIN;
  nightEndMin   = DEFAULT_NIGHT_END_MIN;
  nightLevel    = DEFAULT_NIGHT_LEVEL;
}

void ClockSettings::toJson(JsonObject o) const {
  o["tz"]           = tz;
  o["tzPosix"]      = tzPosix;
  o["nightEnabled"] = nightEnabled;
  o["nightStart"]   = minToHhmm(nightStartMin);
  o["nightEnd"]     = minToHhmm(nightEndMin);
  o["nightLevel"]   = nightLevel;
}

void ClockSettings::fromJson(JsonObjectConst o) {
  if (o["tz"].is<const char*>())          tz = o["tz"].as<String>();
  if (o["tzPosix"].is<const char*>())     tzPosix = o["tzPosix"].as<String>();
  if (o["nightEnabled"].is<bool>())       nightEnabled = o["nightEnabled"];
  if (o["nightStart"].is<const char*>())  nightStartMin = hhmmToMin(o["nightStart"], nightStartMin);
  if (o["nightEnd"].is<const char*>())    nightEndMin   = hhmmToMin(o["nightEnd"], nightEndMin);
  if (o["nightLevel"].is<int>())          nightLevel = constrain((int)o["nightLevel"], 0, 100);
}

// ===========================================================================
// Top-level settings
// ===========================================================================
void Settings::setDefaults() {
  wifiCount = 0;
  for (uint8_t i = 0; i < MAX_WIFI_NETS; i++) {
    wifi[i].ssid = "";
    wifi[i].pass = "";
  }
  apSsid  = DEFAULT_AP_SSID;
  apPass  = DEFAULT_AP_PASS;
  // Unique per device so several devices on one network don't collide on
  // mDNS out of the box. A hostname saved in config.json overrides this.
  hostname = String(DEFAULT_HOSTNAME) + "-" + String(platformChipId() & 0xFFFF, HEX);

  mode = DEFAULT_MODE;
  carouselSec = DEFAULT_CAROUSEL_SEC;
  carouselUsage = true;
  carouselClock = true;
  carouselWeather = true;
  carouselOrderClock   = 1;
  carouselOrderWeather = 2;
  carouselOrderUsage   = 3;
  httpTimeout = DEFAULT_HTTP_TIMEOUT;

  brightness = DEFAULT_BRIGHTNESS;
  autoBrightness = false;
  backlightInverted = TFT_BL_DEFAULT_INVERTED;
  rotation = 0;

  usage.setDefaults();
  clockFace.setDefaults();
  weather.setDefaults();
  clock.setDefaults();
}

// ---------------------------------------------------------------------------
bool settingsBegin() {
  if (LittleFS.begin()) return true;
  // First boot on a fresh chip: format then mount.
  if (LittleFS.format() && LittleFS.begin()) return true;
  return false;
}

bool loadSettings(Settings& s) {
  s.setDefaults();
  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  settingsApplyJson(s, doc.as<JsonObjectConst>());
  return true;
}

bool saveSettings(const Settings& s) {
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  settingsToJson(s, root, /*includeSecrets=*/true);

  File f = LittleFS.open(CONFIG_PATH, "w");
  if (!f) return false;
  bool ok = serializeJson(doc, f) > 0;
  f.close();
  return ok;
}

void factoryReset(Settings& s) {
  LittleFS.remove(CONFIG_PATH);
  s.setDefaults();
}

// ---------------------------------------------------------------------------
void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets) {
  root["hostname"]   = s.hostname;

  // WiFi networks. Passwords only reach the config file, never the web API.
  JsonArray wf = root["wifi"].to<JsonArray>();
  for (uint8_t i = 0; i < s.wifiCount; i++) {
    JsonObject e = wf.add<JsonObject>();
    e["ssid"]    = s.wifi[i].ssid;
    e["passSet"] = s.wifi[i].pass.length() > 0;
    if (includeSecrets) e["pass"] = s.wifi[i].pass;
  }
  // Legacy mirror of the primary network, kept for one release so a firmware
  // downgrade still finds its WiFi in config.json.
  root["staSsid"]    = s.wifiCount ? s.wifi[0].ssid : "";
  root["staPassSet"] = s.wifiCount && s.wifi[0].pass.length() > 0;
  root["apSsid"]     = s.apSsid;
  root["apPassSet"]  = s.apPass.length() > 0;
  if (includeSecrets) {
    root["staPass"]  = s.wifiCount ? s.wifi[0].pass : "";
    root["apPass"]   = s.apPass;
  }

  // Mode + shared HTTP/display
  root["mode"]              = (s.mode == MODE_CAROUSEL) ? "carousel"
                            : (s.mode == MODE_CLOCK)    ? "clock"
                            : (s.mode == MODE_WEATHER)  ? "weather" : "usage";
  root["carouselSec"]         = s.carouselSec;
  root["carouselUsage"]       = s.carouselUsage;
  root["carouselClock"]       = s.carouselClock;
  root["carouselWeather"]     = s.carouselWeather;
  root["carouselOrderClock"]   = s.carouselOrderClock;
  root["carouselOrderWeather"] = s.carouselOrderWeather;
  root["carouselOrderUsage"]   = s.carouselOrderUsage;
  root["httpTimeout"]       = s.httpTimeout;
  root["brightness"]        = s.brightness;
  root["autoBrightness"]    = s.autoBrightness;
  root["backlightInverted"] = s.backlightInverted;
  root["rotation"]          = s.rotation;

  // Feature slices
  s.usage.toJson(root["usage"].to<JsonObject>());
  s.clockFace.toJson(root["clockFace"].to<JsonObject>());
  s.weather.toJson(root["weather"].to<JsonObject>());
  s.clock.toJson(root["clock"].to<JsonObject>());
}

// Apply only the keys that are present (partial update friendly). Accepts both
// the nested layout and the legacy flat layout (feature keys at the top level).
void settingsApplyJson(Settings& s, JsonObjectConst root) {
  if (root["hostname"].is<const char*>()) s.hostname = root["hostname"].as<String>();

  if (root["wifi"].is<JsonArrayConst>()) {
    // The list is authoritative when present (order = try priority, missing
    // row = deletion). A blank password keeps the stored one, matched by SSID
    // so rows survive reordering.
    WifiCred old[MAX_WIFI_NETS];
    uint8_t oldCount = s.wifiCount;
    for (uint8_t i = 0; i < oldCount; i++) old[i] = s.wifi[i];

    s.wifiCount = 0;
    for (JsonObjectConst e : root["wifi"].as<JsonArrayConst>()) {
      if (s.wifiCount >= MAX_WIFI_NETS) break;
      const char* ssid = e["ssid"] | "";
      if (!ssid[0]) continue;                // skip blank rows
      WifiCred& dst = s.wifi[s.wifiCount];
      dst.ssid = ssid;
      const char* pass = e["pass"] | "";
      dst.pass = pass;
      if (!pass[0])
        for (uint8_t i = 0; i < oldCount; i++)
          if (old[i].ssid == dst.ssid) { dst.pass = old[i].pass; break; }
      s.wifiCount++;
    }
  } else if (root["staSsid"].is<const char*>()) {
    // Legacy single-network layout (pre-2.4 config.json or an old cached web
    // page): it becomes/updates the primary network, extras stay untouched.
    String ssid = root["staSsid"].as<String>();
    if (ssid.length()) {
      s.wifi[0].ssid = ssid;
      if (root["staPass"].is<const char*>()) {
        String p = root["staPass"].as<String>();
        if (p.length() > 0) s.wifi[0].pass = p;   // blank = keep
      }
      if (s.wifiCount < 1) s.wifiCount = 1;
    }
  }
  if (root["apSsid"].is<const char*>()) s.apSsid = root["apSsid"].as<String>();
  // AP password: apply as-is when present (empty allowed => open AP).
  if (root["apPass"].is<const char*>()) s.apPass = root["apPass"].as<String>();

  if (root["mode"].is<const char*>()) {
    String m = root["mode"].as<String>();
    // Retired modes (stocks/radar) fall back to usage; carousel/clock/weather are valid.
    s.mode = m.equalsIgnoreCase("carousel") ? MODE_CAROUSEL
           : m.equalsIgnoreCase("clock")    ? MODE_CLOCK
           : m.equalsIgnoreCase("weather")  ? MODE_WEATHER : MODE_USAGE;
  }
  if (root["carouselSec"].is<int>())      s.carouselSec = constrain((int)root["carouselSec"], 5, 3600);
  if (root["carouselUsage"].is<bool>())   s.carouselUsage = root["carouselUsage"];
  if (root["carouselClock"].is<bool>())   s.carouselClock = root["carouselClock"];
  if (root["carouselWeather"].is<bool>()) s.carouselWeather = root["carouselWeather"];
  // Rotation order (1..3). Absent keys keep the defaults set above (graceful
  // migration from a config.json written before this field existed).
  if (root["carouselOrderClock"].is<int>())   s.carouselOrderClock   = (uint8_t)constrain((int)root["carouselOrderClock"],   1, 3);
  if (root["carouselOrderWeather"].is<int>()) s.carouselOrderWeather = (uint8_t)constrain((int)root["carouselOrderWeather"], 1, 3);
  if (root["carouselOrderUsage"].is<int>())   s.carouselOrderUsage   = (uint8_t)constrain((int)root["carouselOrderUsage"],   1, 3);

  if (root["httpTimeout"].is<int>())        s.httpTimeout = constrain((int)root["httpTimeout"], 1000, 20000);
  if (root["brightness"].is<int>())         s.brightness = constrain((int)root["brightness"], 0, 100);
  if (root["autoBrightness"].is<bool>())    s.autoBrightness = root["autoBrightness"];
  if (root["backlightInverted"].is<bool>()) s.backlightInverted = root["backlightInverted"];
  if (root["rotation"].is<int>())           s.rotation = (uint8_t)(((int)root["rotation"]) & 3);

  // Feature slices: prefer the nested object; fall back to the top level so a
  // legacy flat config.json (or a legacy POST) still applies.
  JsonObjectConst u = root["usage"].is<JsonObjectConst>() ? root["usage"].as<JsonObjectConst>() : root;
  s.usage.fromJson(u);
  if (root["clockFace"].is<JsonObjectConst>()) s.clockFace.fromJson(root["clockFace"].as<JsonObjectConst>());
  if (root["weather"].is<JsonObjectConst>()) s.weather.fromJson(root["weather"].as<JsonObjectConst>());
  if (root["clock"].is<JsonObjectConst>()) s.clock.fromJson(root["clock"].as<JsonObjectConst>());
}
