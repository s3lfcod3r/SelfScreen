#include "ClockMode.h"
#include <Arduino_GFX_Library.h>
#include <time.h>
#include "Gfx.h"
#include "Clock.h"
#include "FreeSansBold10pt7b.h"   // vendored Adafruit-GFX bold font (ASCII 0x20-0x7E, ~2.2 KB)
#include "FreeSansBold24pt7b.h"   // large bold font for the big, smooth time

ClockMode g_clockMode;

// --- layout (240x240) ------------------------------------------------------
// Four stacked bands, each cleared to black before its text is (re)drawn so a
// per-second update never leaves ghosting and never needs a full-screen wipe.
// A thin steel-blue accent rule sits in the gap between the time and the date;
// it lives outside every band so no per-second clear ever erases it.
// Time on top (as large as fits the width), then the weekday directly above the
// date, both also maximised to the width. Bands are full-width and non-overlapping
// so a per-second time update never disturbs the weekday/date below.
static const int TM_BAND_Y = 4,   TM_BAND_H = 100;  // big time at top (max width)
static const int AP_BAND_Y = 104, AP_BAND_H = 18;   // AM/PM marker (12h only)
static const int WD_BAND_Y = 124, WD_BAND_H = 52;   // weekday (max width)
static const int DT_BAND_Y = 178, DT_BAND_H = 56;   // date, directly below weekday

// CLK_COL_* preset index -> RGB565 (mirrors the web UI colour <select> order).
static uint16_t clockColor(uint8_t i) {
  switch (i) {
    case CLK_COL_TEAL:        return C_TEAL;
    case CLK_COL_GREEN:       return C_GREEN;
    case CLK_COL_YELLOW:      return C_YELLOW;
    case CLK_COL_RED:         return C_RED;
    case CLK_COL_BLUE:        return C_BLUE;
    case CLK_COL_GRAY:        return C_GRAY;
    case CLK_COL_SELFBLUE:    return C_SELFBLUE;
    case CLK_COL_SELFBLUE_DK: return C_SELFBLUE_DK;
    default:                  return C_WHITE;
  }
}

// German day / month names. All ASCII-safe (the vendored font has no umlauts):
// "Maerz" stands in for "März" so the big font renders every glyph cleanly.
static const char* kWeekdayDE[7] = {
  "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
static const char* kMonthDE[12] = {
  "Januar", "Februar", "Maerz", "April", "Mai", "Juni",
  "Juli", "August", "September", "Oktober", "November", "Dezember"};

// Build the time / AM-PM / date / weekday strings for the current settings. When
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

  if (f.showWeekday && t.tm_wday >= 0 && t.tm_wday < 7) strlcpy(wd, kWeekdayDE[t.tm_wday], wdN);

  int d = t.tm_mday, mo = t.tm_mon + 1, y = t.tm_year + 1900;
  switch (f.dateFormat) {
    case CLK_DATE_DMY:     snprintf(dt, dtN, "%02d.%02d.%04d", d, mo, y); break;
    case CLK_DATE_YMD:     snprintf(dt, dtN, "%04d-%02d-%02d", y, mo, d); break;
    case CLK_DATE_DM:      snprintf(dt, dtN, "%02d.%02d", d, mo);         break;
    case CLK_DATE_DE_LONG:
      if (t.tm_mon >= 0 && t.tm_mon < 12)
        snprintf(dt, dtN, "%d. %s %04d", d, kMonthDE[t.tm_mon], y);      break;
    default:               dt[0] = 0;                                     break;  // CLK_DATE_OFF
  }
}

// Clear a full-width horizontal band to black.
static void clearBand(Arduino_GFX* gfx, int y, int h) {
  gfx->fillRect(0, y, TFT_WIDTH, h, C_BLACK);
}

// Draw `s` centred horizontally on screen and vertically within [bandY, bandY+bandH)
// using the currently-set GFX font and text size. Uses getTextBounds so proportional
// fonts (with per-glyph bearings) land dead-centre — the 6x8 helpers can't do this.
static void drawFontCentered(Arduino_GFX* gfx, const char* s, int bandY, int bandH,
                             uint16_t color) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  int cx = (TFT_WIDTH - (int)w) / 2 - x1;
  int cy = bandY + (bandH - (int)h) / 2 - y1;
  gfx->setTextColor(color);
  gfx->setCursor(cx, cy);
  gfx->print(s);
}

// Largest integer text size (<= maxSize) at which `s` fits within maxW px, for the
// currently-set GFX font. Assumes setFont() has already been called by the caller.
static uint8_t fontFit(Arduino_GFX* gfx, const char* s, int maxW, uint8_t maxSize) {
  for (uint8_t sz = maxSize; sz > 1; --sz) {
    gfx->setTextSize(sz);
    int16_t x1, y1; uint16_t w, h;
    gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
    if ((int)w <= maxW) return sz;
  }
  return 1;
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
  char tm[16], ap[4], dt[24], wd[12];
  buildStrings(f, tm, sizeof(tm), ap, sizeof(ap), dt, sizeof(dt), wd, sizeof(wd));

  uint16_t timeCol = clockColor(f.timeColor);
  uint16_t dateCol = clockColor(f.dateColor);

  if (needFull_) {
    gfx->fillScreen(C_BLACK);
  }

  gfx->setFont(&FreeSansBold10pt7b);   // proportional bold for the whole face

  // Weekday band (light/medium blue, single size).
  if (needFull_ || strcmp(wd, lastWd_) != 0) {
    clearBand(gfx, WD_BAND_Y, WD_BAND_H);
    if (wd[0]) {
      gfx->setTextSize(fontFit(gfx, wd, 234, 4));   // weekday as wide as fits
      drawFontCentered(gfx, wd, WD_BAND_Y, WD_BAND_H, dateCol);
    }
    strlcpy(lastWd_, wd, sizeof(lastWd_));
  }

  // Time band (+ AM/PM band, so both repaint together on any time change). Uses the
  // big 24pt font at size 1 so the digits are smooth, not a small font scaled up
  // into blocky pixels.
  if (needFull_ || strcmp(tm, lastTime_) != 0) {
    clearBand(gfx, TM_BAND_Y, TM_BAND_H);
    clearBand(gfx, AP_BAND_Y, AP_BAND_H);
    // decorative separator line between time and weekday, redrawn with the time so
    // the AP-band clear above never leaves it partly erased.
    gfx->fillRect((TFT_WIDTH - 140) / 2, 114, 140, 3, timeCol);
    gfx->setFont(&FreeSansBold24pt7b);
    gfx->setTextSize(1);
    drawFontCentered(gfx, tm, TM_BAND_Y, TM_BAND_H, timeCol);
    gfx->setFont(&FreeSansBold10pt7b);   // back to the small font for AM/PM + date
    if (ap[0]) {
      gfx->setTextSize(1);
      drawFontCentered(gfx, ap, AP_BAND_Y, AP_BAND_H, dateCol);
    }
    strlcpy(lastTime_, tm, sizeof(lastTime_));
  }

  // Date band (German long form by default; other formats honoured too).
  if (needFull_ || strcmp(dt, lastDate_) != 0) {
    clearBand(gfx, DT_BAND_Y, DT_BAND_H);
    if (dt[0]) {
      gfx->setTextSize(fontFit(gfx, dt, 234, 4));   // date as wide as fits
      drawFontCentered(gfx, dt, DT_BAND_Y, DT_BAND_H, dateCol);
    }
    strlcpy(lastDate_, dt, sizeof(lastDate_));
  }

  // Restore the built-in 6x8 font so shared status/boot screens render normally.
  gfx->setFont(NULL);
  gfx->setTextSize(1);
  needFull_ = false;
}
