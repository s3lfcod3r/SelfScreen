#include "ClockMode.h"
#include <Arduino_GFX_Library.h>
#include <time.h>
#include "Gfx.h"
#include "Clock.h"
#include "u8g2_clock_fonts.h"   // vendored, flash-resident U8g2 font tables (see header)

ClockMode g_clockMode;

// --- layout (240x240) ------------------------------------------------------
// The face is a vertically-centred stack, computed dynamically from the chosen
// per-element font sizes so any size combination stays centred and non-
// overlapping:  TIME  (·AM/PM)  ── separator ──  WEEKDAY  DATE.
// Every element gets a full-width horizontal band sized to its own text height.
// Bands never overlap and the separator rule sits in a gap between bands, so a
// per-second time update clears+redraws only the time (and AM/PM) band and never
// disturbs — or flickers — the rows below it. Positions are cached on a full
// repaint and reused for the cheap per-second updates.
static const int LAY_GAP    = 6;    // vertical gap between stacked elements
static const int LAY_AP_GAP = 2;    // tighter gap between the time and its AM/PM marker
static const int LINE_W     = 140;  // separator rule width (px)
static const int LINE_H     = 3;    // separator rule thickness (px)

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

// German day / month names. All ASCII-safe (the fonts are "_tr", ASCII 0x20-0x7E):
// "Maerz" stands in for "März" so every glyph renders cleanly.
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

// Ink height of `s` in the currently-set U8g2 font.
static int fontHeight(Arduino_GFX* gfx, const char* s) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return (int)h;
}

// Draw `s` centred horizontally on screen and vertically within [bandY, bandY+bandH)
// using the currently-set U8g2 font. getTextBounds returns the real ink box (u8g2
// glyphs carry their own bearing/baseline), so proportional text lands dead-centre.
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

void ClockMode::begin(const Settings& s) {
  needFull_ = true;
  lastTime_[0] = lastDate_[0] = lastWd_[0] = 0;
}

void ClockMode::invalidate(const Settings& s) {
  needFull_ = true;                 // colours/format/sizes may have changed -> full repaint + relayout
  lastTime_[0] = lastDate_[0] = lastWd_[0] = 0;
}

void ClockMode::service(const Settings& s) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;

  const ClockFaceSettings& f = s.clockFace;
  char tm[16], ap[4], dt[24], wd[12];
  buildStrings(f, tm, sizeof(tm), ap, sizeof(ap), dt, sizeof(dt), wd, sizeof(wd));

  // Resolve per-element fonts (indices clamped defensively against the tables).
  uint8_t ti = f.timeSize    < CLK_NUM_FONT_COUNT  ? f.timeSize    : DEFAULT_CLK_TIMESIZE;
  uint8_t wi = f.weekdaySize < CLK_PROP_FONT_COUNT ? f.weekdaySize : DEFAULT_CLK_WEEKDAYSIZE;
  uint8_t di = f.dateSize    < CLK_PROP_FONT_COUNT ? f.dateSize    : DEFAULT_CLK_DATESIZE;
  const uint8_t* timeFont = CLK_NUM_FONTS[ti];
  const uint8_t* wdFont   = CLK_PROP_FONTS[wi];
  const uint8_t* dtFont   = CLK_PROP_FONTS[di];
  const uint8_t* apFont   = CLK_PROP_FONTS[CLK_AP_FONT_IDX];

  // Per-element colours.
  uint16_t timeCol = clockColor(f.timeColor);
  uint16_t wdCol   = clockColor(f.weekdayColor);
  uint16_t dtCol   = clockColor(f.dateColor);
  uint16_t lnCol   = clockColor(f.lineColor);

  // U8g2 fonts render at their native pixel size (no scaling) and print UTF-8.
  // All strings are ASCII, so UTF-8 decoding is a no-op, but it keeps the
  // renderer on its u8g2 path.
  gfx->setUTF8Print(true);
  gfx->setTextSize(1);

  if (needFull_) {
    gfx->fillScreen(C_BLACK);

    // ---- compute the centred stack from the chosen font sizes ----
    // Reserve space for every element the *settings* enable (independent of NTP
    // sync), so the first tick after sync fills reserved bands without relayout.
    // Heights use worst-case sample glyphs (full-height digit / ascender+
    // descender letters) so later content never clips its band.
    gfx->setFont(timeFont); int hTime = fontHeight(gfx, "8");
    int hAp = 0; if (!f.hour24) { gfx->setFont(apFont); hAp = fontHeight(gfx, "AM"); }
    int hWd = 0; if (f.showWeekday) { gfx->setFont(wdFont); hWd = fontHeight(gfx, "Mg"); }
    int hDt = 0; if (f.dateFormat != CLK_DATE_OFF) { gfx->setFont(dtFont); hDt = fontHeight(gfx, "0g"); }

    // Try a comfortable gap first; if the stack overflows 240px (extreme size
    // combo, e.g. huge time + seconds + everything on), tighten the gaps, then
    // finally clamp the top to 0 and accept clipping at the very extremes.
    int gap = LAY_GAP, top = 0;
    for (;;) {
      int H = hTime
            + (hAp ? LAY_AP_GAP + hAp : 0)
            + gap + LINE_H
            + (hWd ? gap + hWd : 0)
            + (hDt ? gap + hDt : 0);
      top = (TFT_HEIGHT - H) / 2;
      if (top >= 0 || gap <= 1) break;
      gap -= 2; if (gap < 1) gap = 1;
    }
    if (top < 0) top = 0;

    int y = top;
    yTime_ = y; hTime_ = hTime; y += hTime;
    if (hAp) { yAp_ = y + LAY_AP_GAP; hAp_ = hAp; y += LAY_AP_GAP + hAp; }
    else     { yAp_ = 0; hAp_ = 0; }
    y += gap;
    yLine_ = y; y += LINE_H;
    if (hWd) { y += gap; yWd_ = y; hWd_ = hWd; y += hWd; } else { yWd_ = 0; hWd_ = 0; }
    if (hDt) { y += gap; yDate_ = y; hDate_ = hDt; y += hDt; } else { yDate_ = 0; hDate_ = 0; }

    // Separator rule: drawn once per full repaint and left alone afterwards (it
    // lives in a gap, outside every clear band, so per-second updates never
    // erase it).
    gfx->fillRect((TFT_WIDTH - LINE_W) / 2, yLine_, LINE_W, LINE_H, lnCol);
  }

  // Time band (+ AM/PM). Clear + redraw together on any time change; both sit in
  // their own bands above the separator so the rows below are untouched.
  if (needFull_ || strcmp(tm, lastTime_) != 0) {
    clearBand(gfx, yTime_, hTime_);
    gfx->setFont(timeFont);
    drawFontCentered(gfx, tm, yTime_, hTime_, timeCol);
    if (hAp_) {
      clearBand(gfx, yAp_, hAp_);
      if (ap[0]) { gfx->setFont(apFont); drawFontCentered(gfx, ap, yAp_, hAp_, timeCol); }
    }
    strlcpy(lastTime_, tm, sizeof(lastTime_));
  }

  // Weekday band (proportional font). Reserved only when enabled.
  if (hWd_ && (needFull_ || strcmp(wd, lastWd_) != 0)) {
    clearBand(gfx, yWd_, hWd_);
    if (wd[0]) { gfx->setFont(wdFont); drawFontCentered(gfx, wd, yWd_, hWd_, wdCol); }
    strlcpy(lastWd_, wd, sizeof(lastWd_));
  }

  // Date band (German long form by default; other formats honoured too).
  if (hDate_ && (needFull_ || strcmp(dt, lastDate_) != 0)) {
    clearBand(gfx, yDate_, hDate_);
    if (dt[0]) { gfx->setFont(dtFont); drawFontCentered(gfx, dt, yDate_, hDate_, dtCol); }
    strlcpy(lastDate_, dt, sizeof(lastDate_));
  }

  // Restore the built-in 6x8 font (clears u8g2Font) and plain ASCII printing so
  // shared status / boot screens render normally after the clock has run.
  gfx->setFont((const GFXfont*)nullptr);
  gfx->setUTF8Print(false);
  gfx->setTextSize(1);
  needFull_ = false;
}
