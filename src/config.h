// config.h — compile-time constants for SelfScreen (based on smalltv-mod)
//
// Hardware: three board variants, all a 1.54" 240x240 ST7789 IPS panel:
//   - Original GeekMagic SmallTV: ESP-12F (ESP8266)      [board_esp8266.h]
//   - Knockoff SmallTV:           ESP32-C2 / ESP8684      [board_esp32c2.h]
//   - NMMiner NM-TV-154:          classic ESP32 (WROOM-32E) [board_esp32.h]
// The board-specific pin map + panel quirks live in the board headers, selected
// below by the build-time target macro. Everything else here is shared.
#pragma once

// ---------------------------------------------------------------------------
// Firmware identity
// ---------------------------------------------------------------------------
#define FW_NAME     "SelfScreen"
#define FW_VERSION  "2.8.2"

// Project / update references (shown in the web UI; used by the GitHub self-update)
#define REPO_URL      "https://github.com/s3lfcod3r/SelfScreen"
#define REPO_OWNER    "s3lfcod3r"
#define REPO_NAME     "SelfScreen"
// Release asset the GitHub self-updater pulls — one app image per target.
#if defined(SMALLTV_ESP32C2)
  #define UPDATE_ASSET "selfscreen-firmware-c2.bin"
#elif defined(SMALLTV_ESP32_PRO)
  #define UPDATE_ASSET "selfscreen-firmware-esp32-pro.bin"
#elif defined(SMALLTV_ESP32)
  #define UPDATE_ASSET "selfscreen-firmware-esp32.bin"
#else
  #define UPDATE_ASSET "selfscreen-firmware.bin"
#endif
#define GH_API_HOST   "api.github.com"
#define DAEMON_URL    "https://github.com/giovi321/clawdmeter-daemon"

// ---------------------------------------------------------------------------
// Display wiring + panel quirks — board-specific, pulled from the right header.
// Provides TFT_SCLK/MOSI/DC/RST/CS/BL, TFT_BGR, TFT_BL_DEFAULT_INVERTED,
// HAS_LDR/LDR_PIN/ADC_MAX. Both panels are 1.54" 240x240 ST7789 IPS.
// ---------------------------------------------------------------------------
#if defined(SMALLTV_ESP32C2)
  #include "board_esp32c2.h"
#elif defined(SMALLTV_ESP32_PRO)
  #include "board_esp32_pro.h"
#elif defined(SMALLTV_ESP32)
  #include "board_esp32.h"
#else
  #include "board_esp8266.h"
#endif

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// ---------------------------------------------------------------------------
// Limits (bound RAM usage on the ESP8266)
// ---------------------------------------------------------------------------
#define MAX_SYMBOLS       8    // max tickers in the rotation
#define MAX_SYMBOL_LEN   24    // e.g. "BTC-USD", cash.ch key "147478611-246-333"
#define MAX_WIFI_NETS     4    // saved WiFi networks; strongest visible wins at boot
#define MAX_NAME_LEN     20    // friendly name shown on screen
#define MAX_SPARK_POINTS 60    // sparkline samples kept per symbol
#define MAX_URL_LEN     200    // webhook base URL

// ---------------------------------------------------------------------------
// Display mode — what the device shows
//   0 = stock / crypto ticker (per-symbol source, see SRC_* below)
//   1 = Claude usage meter (mascot + 5h/7d usage bars, fed by the daemon/)
//   2 = plane radar
//   3 = carousel: rotate through the ticked features on a timer
// ---------------------------------------------------------------------------
#define MODE_STOCKS    0    // retired (ticker feature removed); kept for config back-compat
#define MODE_USAGE     1
#define MODE_RADAR     2    // retired (radar feature removed); kept for config back-compat
#define MODE_CAROUSEL  3
#define MODE_CLOCK     4    // time + date face (fed by SNTP, see Clock.*)
#define MODE_WEATHER   5    // current weather + 12h trend (Open-Meteo, see features/weather)
#define DEFAULT_MODE MODE_CLOCK
#define DEFAULT_CAROUSEL_SEC 30      // per-mode dwell in carousel

// ---------------------------------------------------------------------------
// Compile-time feature toggles. All shipping features are on by default; a lean
// build drops one by setting e.g. -D WITH_RADAR=0 in a PlatformIO env, which
// omits that feature's module from the registry and its web UI section.
// (WITH_RADAR ships off until the radar module lands.)
// ---------------------------------------------------------------------------
#ifndef WITH_TICKER
#define WITH_TICKER 0    // ticker feature removed in SelfScreen
#endif
#ifndef WITH_USAGE
#define WITH_USAGE 1
#endif
#ifndef WITH_CLOCK
#define WITH_CLOCK 1     // time + date face
#endif
#ifndef WITH_RADAR
#define WITH_RADAR 0     // radar feature removed in SelfScreen
#endif
#ifndef WITH_WEATHER
#define WITH_WEATHER 1   // current weather + 12h temperature trend (Open-Meteo)
#endif

// ---------------------------------------------------------------------------
// Clock face (MODE_CLOCK) — display options, all configurable in the web UI.
// Colours are stored as a small preset index (see clockColor() in ClockMode.cpp);
// the same order is mirrored in the web UI's colour <select>.
// ---------------------------------------------------------------------------
#define CLK_DATE_DMY     0    // DD.MM.YYYY
#define CLK_DATE_YMD     1    // YYYY-MM-DD
#define CLK_DATE_DM      2    // DD.MM
#define CLK_DATE_OFF     3    // no date line
#define CLK_DATE_DE_LONG 4    // e.g. "5. August 2026" (German long form)
#define CLK_DATE_MAX     CLK_DATE_DE_LONG

#define CLK_COL_WHITE       0
#define CLK_COL_TEAL        1
#define CLK_COL_GREEN       2
#define CLK_COL_YELLOW      3
#define CLK_COL_RED         4
#define CLK_COL_BLUE        5
#define CLK_COL_GRAY        6
#define CLK_COL_SELFBLUE    7    // soft light steel blue (C_SELFBLUE)
#define CLK_COL_SELFBLUE_DK 8    // medium steel blue (C_SELFBLUE_DK)
#define CLK_COL_MAX         CLK_COL_SELFBLUE_DK

// Per-element font sizes. The web UI exposes a slider per element; the value is
// an index into the ordered font tables in features/clock/u8g2_clock_fonts.h
// (small -> large). These counts/defaults are the single source of truth: the
// font header static_asserts against them, Settings clamps against them, and the
// web UI slider max is COUNT-1.
#define CLK_NUM_FONT_COUNT   7    // time: logisoso 16/22/32/42/50/62/78 _tn
#define CLK_PROP_FONT_COUNT  6    // weekday/date: helvB 08/10/12/14/18/24 _tr
#define CLK_NUM_FONT_MAX    (CLK_NUM_FONT_COUNT  - 1)
#define CLK_PROP_FONT_MAX   (CLK_PROP_FONT_COUNT - 1)
#define CLK_AP_FONT_IDX      2    // AM/PM marker font (helvB12, a small prop size)

#define DEFAULT_CLK_24H          true
#define DEFAULT_CLK_SECONDS      false
#define DEFAULT_CLK_WEEKDAY      true
#define DEFAULT_CLK_DATEFMT      CLK_DATE_DE_LONG
#define DEFAULT_CLK_TIMECOLOR    CLK_COL_WHITE
#define DEFAULT_CLK_DATECOLOR    CLK_COL_WHITE
#define DEFAULT_CLK_WEEKDAYCOLOR CLK_COL_WHITE
#define DEFAULT_CLK_LINECOLOR    CLK_COL_SELFBLUE   // visible accent by default
#define DEFAULT_CLK_TIMESIZE     4    // logisoso50_tn  (index 4 = original look)
#define DEFAULT_CLK_WEEKDAYSIZE  4    // helvB18_tr     (index 4 = original look)
#define DEFAULT_CLK_DATESIZE     4    // helvB18_tr     (index 4 = original look)

// ---------------------------------------------------------------------------
// Weather face (MODE_WEATHER) — a fully composable weather screen fed by
// Open-Meteo over plain HTTP (no API key, no TLS). The face is a stack of
// independently toggleable blocks (primary temp+icon, condition, detail line,
// hourly strip, daily strip, temperature trend). Sizes map into the clock's
// font tables; colours reuse the clock's CLK_COL_* presets (same order,
// mirrored in the web UI <select>). Weather icons come from the vendored u8g2
// open_iconic_weather fonts (see features/weather/weather_icons.h).
// ---------------------------------------------------------------------------
#define WX_COL_MAX  CLK_COL_MAX          // colours share the clock preset list

// Open-Meteo forecast endpoint (plain HTTP). The device appends the query with
// latitude/longitude/units at fetch time. One request returns current + hourly
// (12 h) + daily (4 d); an ArduinoJson filter keeps only the rendered fields so
// the parsed doc stays small (~1.5 KB body, chunked -> getString()).
#define WX_API_HOST      "api.open-meteo.com"
#define WX_API_PATH      "/v1/forecast"
#define WX_HOURLY_POINTS 12              // hourly samples kept (trend + hourly strip)
#define WX_DAILY_POINTS  4               // daily samples kept (daily strip)

// Weather-icon glyphs in the open_iconic_weather fonts. The font carries exactly
// SIX glyphs (verified by decoding the bitmaps): a plain cloud, a sun-behind-
// cloud, a crescent moon, a rain cloud, a raindrop, and a full sun. wmoIcon()
// (weather_icons.h) maps a WMO weather_code (+ is_day) onto the best match.
#define WX_ICON_CLOUD    0x40            // overcast / fog
#define WX_ICON_SUNCLOUD 0x41            // partly cloudy
#define WX_ICON_MOON     0x42            // clear at night
#define WX_ICON_RAIN     0x43            // drizzle / rain / showers / thunderstorm
#define WX_ICON_DROP     0x44            // raindrop — snow stand-in (no snowflake glyph)
#define WX_ICON_SUN      0x45            // clear (day)

// Wind-speed unit (Open-Meteo wind_speed_unit).
#define WX_WIND_KMH  0
#define WX_WIND_MPH  1
#define WX_WIND_MS   2
#define WX_WIND_MAX  WX_WIND_MS

// Hourly / daily strip bounds (web UI clamps to these).
#define WX_HOURLY_COUNT_MIN 3
#define WX_HOURLY_COUNT_MAX 6
#define WX_HOURLY_STEP_MIN  1
#define WX_HOURLY_STEP_MAX  3
#define WX_DAILY_COUNT_MIN  2
#define WX_DAILY_COUNT_MAX  4

// Detail-line precipitation display mode.
#define WX_PRECIP_MM    0                // "Regen 0.0 mm" (amount)
#define WX_PRECIP_PCT   1                // "Regen 20%"    (current-hour chance, hours[0].pop)
#define WX_PRECIP_BOTH  2                // "Regen 20% 0.0mm"
#define WX_PRECIP_MAX   WX_PRECIP_BOTH

// Weather-icon colouring.
#define WX_ICONCOL_SEMANTIC 0            // colour by weather (sun=yellow, cloud=grey, rain=blue-grey…)
#define WX_ICONCOL_FIXED    1            // one preset colour for every icon (bigIconColor)
#define WX_ICONCOL_MAX      WX_ICONCOL_FIXED

#define DEFAULT_WX_LAT        53.55f     // Hamburg
#define DEFAULT_WX_LON        9.99f
#define DEFAULT_WX_UNITF      false      // false = Celsius, true = Fahrenheit
#define DEFAULT_WX_WINDUNIT   WX_WIND_KMH
#define DEFAULT_WX_PRECIPMODE WX_PRECIP_PCT   // detail precip: default = % (people want chance)
#define DEFAULT_WX_ICONCOLORMODE WX_ICONCOL_SEMANTIC
#define DEFAULT_WX_REFRESH    600        // seconds between fetches
#define WX_REFRESH_MIN        60
#define WX_REFRESH_MAX        21600      // 6 h
#define WX_RETRY_SEC          30         // faster retry after a failed fetch

// ---- block toggles (defaults = a good-looking "Stunden" preset) ----
#define DEFAULT_WX_SHOWTEMP     true     // A: big current temperature
#define DEFAULT_WX_SHOWBIGICON  true     // A: big weather icon beside the temp
#define DEFAULT_WX_SHOWCOND     true     // B: German condition text
#define DEFAULT_WX_SHOWFEELS    false    // C: "Gefuehlt 20°"
#define DEFAULT_WX_SHOWHUM      false    // C: "Luft 61%"
#define DEFAULT_WX_SHOWWIND     false    // C: "Wind 12 km/h"
#define DEFAULT_WX_SHOWPRECIP   true     // C: "Regen 20%" (compact rain chance, on by default)
#define DEFAULT_WX_SHOWHOURLY   true     // D: hourly strip
#define DEFAULT_WX_SHOWDAILY    false    // E: daily strip
#define DEFAULT_WX_SHOWTREND    false    // F: 12h temperature sparkline

// ---- hourly strip (D) ----
#define DEFAULT_WX_HOURLYCOUNT  4        // columns (WX_HOURLY_COUNT_MIN..MAX)
#define DEFAULT_WX_HOURLYSTEP   1        // hours between columns (1/2/3)
#define DEFAULT_WX_HR_HOUR      true     // sub-field: "15h"
#define DEFAULT_WX_HR_ICON      true     // sub-field: mini icon
#define DEFAULT_WX_HR_TEMP      true     // sub-field: temperature
#define DEFAULT_WX_HR_POP       true     // sub-field: rain probability

// ---- daily strip (E) ----
#define DEFAULT_WX_DAILYCOUNT   3        // columns (WX_DAILY_COUNT_MIN..MAX)
#define DEFAULT_WX_DY_DAY       true     // sub-field: "Fr"
#define DEFAULT_WX_DY_ICON      true     // sub-field: mini icon
#define DEFAULT_WX_DY_TEMPS     true     // sub-field: "24/16"
#define DEFAULT_WX_DY_POP       true     // sub-field: rain %

#define DEFAULT_WX_TRENDLABELS  false    // F: min/max labels on the sparkline

// Sizes are indices into the clock font tables (see u8g2_clock_fonts.h). The web
// UI slider max is the matching CLK_*_FONT_MAX. Temperature uses the big number
// fonts; everything else uses the proportional letter fonts.
#define DEFAULT_WX_TEMPSIZE    5         // CLK_NUM_FONTS  index (logisoso62_tn)
#define DEFAULT_WX_CONDSIZE    3         // CLK_PROP_FONTS index (helvB14_tr)
#define DEFAULT_WX_DETAILSIZE  1         // CLK_PROP_FONTS index (helvB10_tr)
#define DEFAULT_WX_HOURLYSIZE  0         // CLK_PROP_FONTS index (helvB08_tr)
#define DEFAULT_WX_DAILYSIZE   0         // CLK_PROP_FONTS index (helvB08_tr)

#define DEFAULT_WX_TEMPCOLOR    CLK_COL_WHITE
#define DEFAULT_WX_BIGICONCOLOR CLK_COL_YELLOW
#define DEFAULT_WX_CONDCOLOR    CLK_COL_SELFBLUE
#define DEFAULT_WX_DETAILCOLOR  CLK_COL_GRAY
#define DEFAULT_WX_HOURLYCOLOR  CLK_COL_WHITE
#define DEFAULT_WX_HOURLYPOP    CLK_COL_TEAL
#define DEFAULT_WX_DAILYCOLOR   CLK_COL_WHITE
#define DEFAULT_WX_DAILYPOP     CLK_COL_TEAL
#define DEFAULT_WX_TRENDCOLOR   CLK_COL_TEAL

// Claude usage mode: once data stops arriving for this long (PC asleep, daemon
// stopped, network down) the screen switches from the stats to the idle mascot
// animation. Effective timeout also scales with the poll period (see main.cpp).
#define USAGE_STALE_GRACE_MS  20000UL

// ---------------------------------------------------------------------------
// Data source (stock mode)
//   0 = custom webhook (n8n / Node-RED / your own HTTP endpoint)
//   1 = Yahoo Finance, fetched directly by the device (no backend needed)
//   2 = cash.ch, fetched directly by the device (Swiss instruments, incl.
//       off-exchange structured products that Yahoo doesn't carry)
// ---------------------------------------------------------------------------
#define SRC_WEBHOOK  0
#define SRC_YAHOO    1
#define SRC_CASH     2
#define SRC_GHUB     3   // static JSON published to the repo's data branch (see below)
#define DEFAULT_SOURCE  SRC_YAHOO            // works out of the box, no server

// Yahoo Finance public chart endpoint. A browser-like User-Agent is required —
// requests with an empty UA are rejected with HTTP 429. TLS records from Yahoo
// are <=~1.3 KB, so the 4 KB BearSSL receive buffer in StockClient is plenty.
// query1/query2 are interchangeable mirrors; we fall back to the second on a
// transient failure (a single back-to-back HTTPS fetch occasionally drops).
#define YAHOO_CHART_HOST1 "query1.finance.yahoo.com"
#define YAHOO_CHART_HOST2 "query2.finance.yahoo.com"
#define YAHOO_CHART_PATH  "/v8/finance/chart/"
#define YAHOO_USER_AGENT  "Mozilla/5.0 (SmallTV)"

// cash.ch public GraphQL endpoint. The device sends two small hand-written
// GraphQL queries per symbol as plain GETs (?query=...): a ~200 B quote and a
// slim daily-close series for the sparkline. No API key, no cookies, no
// required headers. The symbol is the cash.ch listing key
// `valor-marketId-currencyId` (see the docs for how to find it).
// cash.ch's CDN requires ECDHE. The ESP32 targets (mbedTLS) do this easily. The
// ESP8266 (BearSSL) can too, but the handshake is memory-tight, so the cash
// path is shaped to fit: only cash.ch is offered ECDHE (Yahoo and the GitHub
// source are pinned to the cheap static-RSA suites), the connection uses 512 B
// buffers + TLS session resumption, and StockClient skips a fetch unless a
// large enough contiguous heap block is free. The GitHub source below is a
// zero-crash fallback if a device ever proves too tight for the direct path.

// GitHub source (SRC_GHUB): a scheduled workflow (.github/workflows/quotes.yml)
// fetches cash.ch server-side and publishes one JSON file per listing key to
// the repo's `data` branch. The device reads it from raw.githubusercontent.com,
// which — unlike cash.ch — still accepts the ESP8266's static-RSA handshake
// (the same one GitHub self-update and Yahoo use). The file is the same JSON
// the webhook parser accepts. The symbol is the cash.ch listing key; only keys
// listed in quotes-config.json are published. raw sends a ~4 KB certificate
// record and does not negotiate MFLN, so this path uses a larger TLS buffer.
#define GH_QUOTES_BASE "https://raw.githubusercontent.com/" REPO_OWNER "/" REPO_NAME "/data/quotes/"
#define GH_QUOTES_RXBUF 5120
#define CASH_GQL_HOST   "www.cash.ch"
#define CASH_GQL_PATH   "/_/api/graphql/prod"
#define CASH_USER_AGENT "Mozilla/5.0 (SmallTV)"

// ---------------------------------------------------------------------------
// Plane radar (MODE_RADAR)
//   Data source (radar's own selector, independent of the stock one):
//     0 = adsb.fi opendata, fetched directly by the device over HTTPS (no key)
//     1 = custom webhook (a LAN proxy that pre-filters — robust on the ESP8266)
// ---------------------------------------------------------------------------
#define RADAR_SRC_DIRECT   0
#define RADAR_SRC_WEBHOOK  1
#define DEFAULT_RADAR_SRC  RADAR_SRC_DIRECT

// adsb.fi free open-data endpoint (no API key; public rate limit ~1 req/s).
// Full path: /api/v3/lat/{lat}/lon/{lon}/dist/{nm}
#define ADSB_HOST        "opendata.adsb.fi"
#define ADSB_PATH        "/api/v3/lat/"
#define ADSB_USER_AGENT  "Mozilla/5.0 (SmallTV)"

// Bound RAM: nearest N aircraft kept/drawn, and a few home-area airports.
#define MAX_AIRCRAFT     24
#define MAX_AIRPORTS      6
#define MAX_ICAO_LEN      8      // ICAO ident + NUL (e.g. "LSZH")

// Defaults (lat/lon 0,0 is the "not set yet" sentinel -> shows a prompt).
#define DEFAULT_RADAR_LAT       0.0f
#define DEFAULT_RADAR_LON       0.0f
#define DEFAULT_RADAR_RANGE_KM  20
#define DEFAULT_RADAR_POLL_SEC  10     // >=3 keeps us under the 1 req/s limit

// ---------------------------------------------------------------------------
// Defaults (used on first boot / factory reset)
// ---------------------------------------------------------------------------
#define DEFAULT_AP_SSID      "SmallTV-Setup"
#define DEFAULT_AP_PASS      ""              // empty => open AP
#define DEFAULT_HOSTNAME     "smalltv"
#define DEFAULT_POLL_SEC      120            // how often to refresh data
#define TICKER_RETRY_SEC       12            // fast retry after a failed/skipped fetch
#define TICKER_RETRY_MAX        4            // consecutive fast retries before backing off
#define DEFAULT_ROTATE_SEC    10             // how long each symbol is shown
#define DEFAULT_RANGE        "1d"            // chart timeframe (e.g. 1d/5d/1mo/1y)
#define DEFAULT_POINTS        48             // sparkline points requested
#define DEFAULT_BRIGHTNESS    90             // 0..100 %
#define DEFAULT_HTTP_TIMEOUT  8000           // ms per request

// --- Clock / night mode (device-wide) ---
#define NTP_SERVER1             "pool.ntp.org"
#define NTP_SERVER2             "time.nist.gov"
#define DEFAULT_TZ_NAME         ""        // IANA display name; empty = UTC
#define DEFAULT_TZ_POSIX        "UTC0"    // POSIX TZ rule the device feeds SNTP
#define DEFAULT_NIGHT_ENABLED   false
#define DEFAULT_NIGHT_START_MIN 1320      // 22:00
#define DEFAULT_NIGHT_END_MIN   420       // 07:00
#define DEFAULT_NIGHT_LEVEL     0         // 0..100, 0 = backlight fully off

// Night-mode NTP trust: only ENTER night mode when the clock was confirmed by a
// successful NTP sync within NIGHT_NTP_TRUST_MS (else we assume the clock may be
// wrong and keep the screen on). While inside the window but unconfirmed, re-arm
// SNTP every NIGHT_NTP_RESYNC_MS until a fresh sync lands or the window ends
// (morning). Once night mode has switched on, it stays on until the window ends.
#define NIGHT_NTP_TRUST_MS      300000UL  // 5 min: max age of the sync that unlocks night
#define NIGHT_NTP_RESYNC_MS      30000UL  // re-sync attempt cadence while held off
