#include "ClockMode.h"
#include <Arduino_GFX_Library.h>
#include <time.h>
#include "Gfx.h"
#include "Clock.h"

ClockMode g_clockMode;

// --- layout (240x240) ------------------------------------------------------
// Three stacked bands, each cleared to black before its text is (re)drawn so a
// per-second update never leaves ghosting and never needs a full-screen wipe.
static const int WD_Y      = 40;                 // weekday line top
static const int TIME_Y    = 96;                 // big time line top
static const int AMPM_Y    = 162;                // small AM/PM (12h only)
static const int DATE_Y    = 194;                // date line top
static const int WD_BAND_Y = 36,  WD_BAND_H = 30;
static const int TM_BAND_Y = 88,  TM_BAND_H = 96;  // covers time + AM/PM
static const int DT_BAND_Y = 186, DT_BAND_H = 40;

// CLK_COL_* preset index -> RGB565 (mirrors the web UI colour <select> order).
static uint16_t clockColor(uint8_t i) {
  switch (i) {
    case CLK_COL_TEAL:   return C_TEAL;
    case CLK_COL_GREEN:  return C_GREEN;
    case CLK_COL_YELLOW: return C_YELLOW;
    case CLK_COL_RED:    return C_RED;
    case CLK_COL_BLUE:   return C_BLUE;
    case CLK_COL_GRAY:   return C_GRAY;
    default:             return C_WHITE;
  }
}

static const char* kWeekday[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Build the time / AM-P / date / weekday strings for the current settings. When
// the clock hasn't synced yet, show a neutral placeholder instead of a wrong time.
static void buildStrings(const ClockFaceSettings& f, char* tm, size_t tmN,
                         char* ap, size_t apN, char* dt, size_t dtN,
                         char* wd, size_t wdN) {
  ap[0] = dt[0] = wd[0] = 0;

  struct tm t;
  if (!clockNow(t)) { strlcpy(tm, "--:--", tmN); return; }

  int hh = t.tm_hour;
  if (!f.hour24) {
    strlcpy(ap, hh < 12 ? "AM" : "PM", apN);
    hh %= 12;
    if (hh == 0) hh = 12;
  }
  if (f.showSeconds) {
    if (f.hour24) snprintf(tm, tmN, "%02d:%02d:%02d", hh, t.tm_min, t.tm_sec);
    else          snprintf(tm, tmN, "%d:%02d:%02d",   hh, t.tm_min, t.tm_sec);
  } else {
    if (f.hour24) snprintf(tm, tmN, "%02d:%02d", hh, t.tm_min);
    else          snprintf(tm, tmN, "%d:%02d",   hh, t.tm_min);
  }

  if (f.showWeekday && t.tm_wday >= 0 && t.tm_wday < 7) strlcpy(wd, kWeekday[t.tm_wday], wdN);

  int d = t.tm_mday, mo = t.tm_mon + 1, y = t.tm_year + 1900;
  switch (f.dateFormat) {
    case CLK_DATE_DMY: snprintf(dt, dtN, "%02d.%02d.%04d", d, mo, y); break;
    case CLK_DATE_YMD: snprintf(dt, dtN, "%04d-%02d-%02d", y, mo, d); break;
    case CLK_DATE_DM:  snprintf(dt, dtN, "%02d.%02d", d, mo);         break;
    default:           dt[0] = 0;                                     break;  // CLK_DATE_OFF
  }
}

// Clear a full-width horizontal band to black.
static void clearBand(Arduino_GFX* gfx, int y, int h) {
  gfx->fillRect(0, y, TFT_WIDTH, h, C_BLACK);
}

void ClockMode::begin(const Settings& s) {
  needFull_ = true;
  lastTime_[0] = lastDate_[0] = lastWd_[0] = 0;
}

void ClockMode::invalidate(const Settings& s) {
  needFull_ = true;                 // colours/format/size may have changed -> full repaint
  lastTime_[0] = lastDate_[0] = lastWd_[0] = 0;
}

void ClockMode::service(const Settings& s) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;

  const ClockFaceSettings& f = s.clockFace;
  char tm[16], ap[4], dt[24], wd[8];
  buildStrings(f, tm, sizeof(tm), ap, sizeof(ap), dt, sizeof(dt), wd, sizeof(wd));

  uint16_t timeCol = clockColor(f.timeColor);
  uint16_t dateCol = clockColor(f.dateColor);

  if (needFull_) gfx->fillScreen(C_BLACK);

  // Weekday band.
  if (needFull_ || strcmp(wd, lastWd_) != 0) {
    clearBand(gfx, WD_BAND_Y, WD_BAND_H);
    if (wd[0]) gfxDrawCentered(wd, WD_Y, 3, dateCol);
    strlcpy(lastWd_, wd, sizeof(lastWd_));
  }

  // Time band (also carries the AM/PM marker so both repaint together).
  if (needFull_ || strcmp(tm, lastTime_) != 0) {
    clearBand(gfx, TM_BAND_Y, TM_BAND_H);
    uint8_t sz = gfxFitSize(tm, 232, f.bigSize ? 8 : 5);
    gfxDrawCentered(tm, TIME_Y, sz, timeCol);
    if (ap[0]) gfxDrawCentered(ap, AMPM_Y, 2, dateCol);
    strlcpy(lastTime_, tm, sizeof(lastTime_));
  }

  // Date band.
  if (needFull_ || strcmp(dt, lastDate_) != 0) {
    clearBand(gfx, DT_BAND_Y, DT_BAND_H);
    if (dt[0]) gfxDrawCentered(dt, DATE_Y, gfxFitSize(dt, 232, 3), dateCol);
    strlcpy(lastDate_, dt, sizeof(lastDate_));
  }

  needFull_ = false;
}
