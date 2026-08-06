// Settings.h — persisted configuration (LittleFS /config.json)
//
// Layout is segmented per feature: shared device/network fields live at the top
// level, and each feature owns a nested settings slice (usage / clock).
// config.json mirrors this: { ..shared.., "usage":{...}, "clock":{...} }.
// The JSON reader also still accepts the old flat layout, so a device upgrading
// from the pre-segmentation firmware keeps its WiFi.
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

// One saved WiFi station network. The device keeps up to MAX_WIFI_NETS and
// joins the strongest visible one at boot (hidden SSIDs are tried last).
struct WifiCred {
  String ssid;
  String pass;
};

// ---- Claude usage feature slice -------------------------------------------
struct UsageSettings {
  String   usageUrl;      // daemon HTTP endpoint, e.g. http://192.168.1.10:8787/
  uint16_t pollSec;       // refresh period

  void setDefaults();
  void toJson(JsonObject o) const;
  void fromJson(JsonObjectConst o);
};

// ---- Clock face feature slice (MODE_CLOCK display options) ------------------
struct ClockFaceSettings {
  bool    hour24;       // true = 24h, false = 12h (with AM/PM)
  bool    showSeconds;  // append :SS to the time
  bool    showWeekday;  // show the weekday name (German, ASCII)
  uint8_t dateFormat;   // CLK_DATE_* (DMY / YMD / DM / OFF / DE_LONG)
  uint8_t timeColor;    // CLK_COL_* preset index — time
  uint8_t dateColor;    // CLK_COL_* preset index — date
  uint8_t weekdayColor; // CLK_COL_* preset index — weekday
  uint8_t lineColor;    // CLK_COL_* preset index — separator rule
  uint8_t timeSize;     // index into CLK_NUM_FONTS[]  (0..CLK_NUM_FONT_MAX)
  uint8_t weekdaySize;  // index into CLK_PROP_FONTS[] (0..CLK_PROP_FONT_MAX)
  uint8_t dateSize;     // index into CLK_PROP_FONTS[] (0..CLK_PROP_FONT_MAX)

  void setDefaults();
  void toJson(JsonObject o) const;
  void fromJson(JsonObjectConst o);
};

// ---- Clock / night mode slice (device-wide) --------------------------------
struct ClockSettings {
  String   tz;            // IANA display name, e.g. "Europe/Rome" (UI round-trip)
  String   tzPosix;       // POSIX TZ rule the device feeds SNTP
  bool     nightEnabled;  // dim/blank on a nightly schedule
  uint16_t nightStartMin; // minutes since local midnight (0..1439)
  uint16_t nightEndMin;
  uint8_t  nightLevel;    // 0..100, 0 = backlight off

  void setDefaults();
  void toJson(JsonObject o) const;
  void fromJson(JsonObjectConst o);
};

// ---- Top-level settings ----------------------------------------------------
struct Settings {
  // --- WiFi station networks (the device joins one of these) ---
  WifiCred wifi[MAX_WIFI_NETS];
  uint8_t  wifiCount;

  // --- Access point (config / fallback hotspot) ---
  String apSsid;
  String apPass;        // empty => open network
  String hostname;      // mDNS name => http://<hostname>.local

  // --- Active feature ---
  uint8_t mode;         // MODE_USAGE / MODE_CAROUSEL

  // --- Carousel (mode == MODE_CAROUSEL): dwell + which features rotate ---
  uint16_t carouselSec;
  bool carouselUsage;
  bool carouselClock;

  // --- Shared HTTP / display ---
  uint16_t httpTimeout; // ms
  uint8_t  brightness;        // 0..100 %
  bool     autoBrightness;    // use LDR on A0
  bool     backlightInverted; // active-low backlight
  uint8_t  rotation;          // 0..3 screen orientation

  // --- Feature slices ---
  UsageSettings     usage;
  ClockFaceSettings clockFace;
  ClockSettings     clock;

  void setDefaults();
};

// Persistence
bool settingsBegin();                       // mount LittleFS
bool loadSettings(Settings& s);             // false => defaults applied
bool saveSettings(const Settings& s);
void factoryReset(Settings& s);             // wipe file + defaults

// JSON <-> struct. `includeSecrets=false` masks passwords for the web API.
void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets);
void settingsApplyJson(Settings& s, JsonObjectConst root); // partial update allowed
