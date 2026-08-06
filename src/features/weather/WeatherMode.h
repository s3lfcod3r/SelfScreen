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
  void wake(const Settings& s) override { needFull_ = true; }   // repaint only, no refetch

 private:
  void render(const Settings& s, const WeatherData& d);

  bool     needFull_ = true;              // force a repaint (boot / wake / settings change)
  uint32_t lastStamp_ = 0xFFFFFFFF;       // lastOkMs already drawn (dirty-tracking)
};

extern WeatherMode g_weatherMode;
