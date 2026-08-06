// WeatherMode.h — current weather + 12h temperature-trend face.
//
// A self-contained DisplayMode that renders the Open-Meteo snapshot (from
// WeatherClient) as a big temperature, a German condition string, a precipitation
// line and a small trend sparkline — each element individually toggled, sized and
// coloured from the weather settings slice. Layout is measured-then-centred (the
// same approach as the clock face) so any size combination stays centred and
// non-overlapping, and it reuses the clock's vendored U8g2 font tables.
#pragma once
#include "Mode.h"
#include "config.h"

class WeatherData;   // fwd-decl; only the .cpp pulls in the full definition

class WeatherMode : public DisplayMode {
 public:
  const char* id() const override { return "weather"; }
  uint8_t     modeConst() const override { return MODE_WEATHER; }

  void begin(const Settings& s) override;
  void service(const Settings& s) override;
  void invalidate(const Settings& s) override;
  // Repaint only (carousel switch): reset to the first page, do NOT refetch.
  void wake(const Settings& s) override { needFull_ = true; pageReset_ = true; }

 private:
  // Full-screen page renderers (one is drawn per service tick, see WX_PAGE_*).
  void renderNow(const Settings& s, const WeatherData& d);        // current conditions
  void renderTempTrend(const Settings& s, const WeatherData& d);  // hourly temp chart
  void renderRainTrend(const Settings& s, const WeatherData& d);  // hourly rain bars
  void renderDays7(const Settings& s, const WeatherData& d);      // 7-day forecast
  void renderPage(const Settings& s, const WeatherData& d, uint8_t page);
  // Build the enabled-page list ordered by the per-page order number (ascending,
  // ties by page id). Returns the count; falls back to NOW if none are enabled.
  uint8_t buildPageList(const WeatherSettings& w, uint8_t* out) const;

  bool     needFull_ = true;              // force a repaint (boot / wake / settings change)
  uint32_t lastStamp_ = 0xFFFFFFFF;       // lastOkMs already drawn (dirty-tracking)
  bool     pageReset_ = true;             // jump back to the first page + repaint
  uint8_t  pageSlot_  = 0;                // index into the enabled-page list
  uint8_t  lastPage_  = 0xFF;             // page id already drawn (dirty-tracking)
  uint32_t pageStart_ = 0;                // millis() the current page went up
};

extern WeatherMode g_weatherMode;
