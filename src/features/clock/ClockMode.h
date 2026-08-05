// ClockMode.h — time + date face.
//
// A self-contained DisplayMode that renders the SNTP wall clock (from Clock.*)
// as a big HH:MM plus an optional weekday and date line. Every visual option
// (12/24h, seconds, weekday, date format, colours, size) is configurable in the
// web UI and persisted in the clockFace settings slice. The time itself comes
// from the shared Clock helpers — this mode never touches NTP directly.
#pragma once
#include "Mode.h"
#include "config.h"

class ClockMode : public DisplayMode {
 public:
  const char* id() const override { return "clock"; }
  uint8_t     modeConst() const override { return MODE_CLOCK; }

  void begin(const Settings& s) override;
  void service(const Settings& s) override;
  void invalidate(const Settings& s) override;
  void wake(const Settings& s) override { needFull_ = true; }  // repaint only

 private:
  bool needFull_ = true;               // force a full redraw (boot / wake / settings change)
  char lastTime_[16] = {0};            // last drawn time string (dirty-tracking)
  char lastDate_[24] = {0};            // last drawn date string
  char lastWd_[8]    = {0};            // last drawn weekday string
};

extern ClockMode g_clockMode;
