#include "WeatherMode.h"
#include <Arduino_GFX_Library.h>
#include <math.h>
#include "Gfx.h"
#include "WeatherClient.h"
#include "WeatherData.h"
#include "u8g2_clock_fonts.h"   // shared vendored U8g2 font tables (see clock header)

WeatherMode g_weatherMode;

// --- layout (240x240) ------------------------------------------------------
// A vertically-centred stack computed from the chosen per-element font sizes so
// any size combination stays centred and non-overlapping:
//   TEMPERATURE  CONDITION  PRECIPITATION  TREND-SPARKLINE
// Every enabled element gets a full-width band sized to its own height; the
// sparkline gets a fixed-height band. Extreme size combos tighten the gaps.
static const int WX_GAP     = 8;    // vertical gap between stacked elements
static const int WX_TREND_H = 46;   // sparkline band height (px)
static const int WX_TREND_MARGIN = 22;  // left/right inset of the sparkline

// CLK_COL_* preset index -> RGB565 (mirrors the web UI colour <select> order —
// the weather colours reuse the clock preset list).
static uint16_t wxColor(uint8_t i) {
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

// Ink height of `s` in the currently-set U8g2 font.
static int fontHeight(Arduino_GFX* gfx, const char* s) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return (int)h;
}

// Draw `s` centred horizontally and vertically within [bandY, bandY+bandH).
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

// Draw the temperature number (num font — digits + '-') centred, with a hollow
// degree ring drawn at its top-right. The _tn fonts carry no '°'/letters, so the
// ring is drawn as two concentric circles rather than printed as a glyph.
static void drawTempCentered(Arduino_GFX* gfx, const char* num, int bandY, int bandH,
                             uint16_t color) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(num, 0, 0, &x1, &y1, &w, &h);
  int r = (int)h / 10; if (r < 3) r = 3;   // ring radius scales with the font size
  int gap = r;
  int total = (int)w + gap + 2 * r;         // number + gap + ring
  int startx = (TFT_WIDTH - total) / 2 - x1;
  int inkTop = bandY + (bandH - (int)h) / 2;
  int cy = inkTop - y1;
  gfx->setTextColor(color);
  gfx->setCursor(startx, cy);
  gfx->print(num);
  int ringCx = startx + x1 + (int)w + gap + r;
  int ringCy = inkTop + r;
  int th = r / 2; if (th < 1) th = 1;
  gfx->fillCircle(ringCx, ringCy, r, color);
  gfx->fillCircle(ringCx, ringCy, r - th, C_BLACK);
}

// 12h temperature sparkline inside [x,y,w,h], auto-scaled to its own min/max.
static void drawTrend(Arduino_GFX* gfx, const WeatherData& d, int x, int y, int w,
                      int h, uint16_t col) {
  if (d.nPts < 2) return;
  float mn = d.trend[0], mx = d.trend[0];
  for (uint8_t i = 1; i < d.nPts; i++) {
    if (d.trend[i] < mn) mn = d.trend[i];
    if (d.trend[i] > mx) mx = d.trend[i];
  }
  float span = mx - mn;
  if (span < 0.5f) span = 0.5f;             // flat series -> a mid line, no div-by-zero
  int n = d.nPts;
  int prevX = 0, prevY = 0;
  for (int i = 0; i < n; i++) {
    int px = x + (w - 1) * i / (n - 1);
    int py = y + (h - 1) - (int)((d.trend[i] - mn) / span * (h - 1));
    if (i > 0) gfx->drawLine(prevX, prevY, px, py, col);
    gfx->fillCircle(px, py, 1, col);
    prevX = px; prevY = py;
  }
}

void WeatherMode::begin(const Settings& s) {
  weatherInit(s);
  needFull_ = true;
  lastStamp_ = 0xFFFFFFFF;
}

void WeatherMode::invalidate(const Settings& s) {
  weatherInit(s);
  weatherForceRefresh();                    // lat/lon/units may have changed -> refetch
  needFull_ = true;
  lastStamp_ = 0xFFFFFFFF;
}

void WeatherMode::service(const Settings& s) {
  weatherService(s);                        // fetch on the schedule (no-op between polls)
  const WeatherData& d = weatherGet();
  uint32_t stamp = d.valid ? d.lastOkMs : 0;
  if (needFull_ || stamp != lastStamp_) {
    render(s, d);
    lastStamp_ = stamp;
    needFull_ = false;
  }
}

void WeatherMode::render(const Settings& s, const WeatherData& d) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;
  const WeatherSettings& w = s.weather;

  gfx->setUTF8Print(true);
  gfx->setTextSize(1);

  // Resolve per-element fonts (indices clamped defensively against the tables).
  uint8_t tSz = w.tempSize   < CLK_NUM_FONT_COUNT  ? w.tempSize   : DEFAULT_WX_TEMPSIZE;
  uint8_t cSz = w.condSize   < CLK_PROP_FONT_COUNT ? w.condSize   : DEFAULT_WX_CONDSIZE;
  uint8_t pSz = w.precipSize < CLK_PROP_FONT_COUNT ? w.precipSize : DEFAULT_WX_PRECIPSIZE;
  const uint8_t* tempFont = CLK_NUM_FONTS[tSz];
  const uint8_t* condFont = CLK_PROP_FONTS[cSz];
  const uint8_t* precFont = CLK_PROP_FONTS[pSz];

  // Build the display strings (neutral "--" placeholder until the first fetch).
  char tempStr[12];
  if (d.valid) snprintf(tempStr, sizeof(tempStr), "%d", (int)lroundf(d.temp));
  else         strlcpy(tempStr, "--", sizeof(tempStr));

  const char* condStr = d.valid ? weatherCodeDE(d.code) : "";
  char precStr[16];
  if (d.valid) snprintf(precStr, sizeof(precStr), "%.1f mm", d.precip);
  else         precStr[0] = 0;

  // Which elements actually get a band this frame.
  bool hasTemp   = w.showTemp;
  bool hasCond   = w.showCond   && condStr[0];
  bool hasPrecip = w.showPrecip && precStr[0];
  bool hasTrend  = w.showTrend  && d.nPts >= 2;

  // ---- measure element heights (worst-case sample glyphs so nothing clips) ----
  int hTemp = 0, hCond = 0, hPrec = 0, hTrend = hasTrend ? WX_TREND_H : 0;
  if (hasTemp)   { gfx->setFont(tempFont); hTemp = fontHeight(gfx, "-8"); }
  if (hasCond)   { gfx->setFont(condFont); hCond = fontHeight(gfx, "Mg"); }
  if (hasPrecip) { gfx->setFont(precFont); hPrec = fontHeight(gfx, "0g"); }

  // ---- centre the stack; tighten gaps if an extreme size combo overflows ----
  int gap = WX_GAP, top = 0;
  for (;;) {
    int H = (hasTemp   ? hTemp  : 0)
          + (hasCond   ? (hasTemp ? gap : 0) + hCond  : 0)
          + (hasPrecip ? ((hasTemp || hasCond) ? gap : 0) + hPrec : 0)
          + (hasTrend  ? ((hasTemp || hasCond || hasPrecip) ? gap : 0) + hTrend : 0);
    top = (TFT_HEIGHT - H) / 2;
    if (top >= 0 || gap <= 1) break;
    gap -= 2; if (gap < 1) gap = 1;
  }
  if (top < 0) top = 0;

  // ---- draw ----
  gfx->fillScreen(C_BLACK);
  int y = top;
  bool prev = false;
  if (hasTemp) {
    gfx->setFont(tempFont);
    if (d.valid) drawTempCentered(gfx, tempStr, y, hTemp, wxColor(w.tempColor));
    else         drawFontCentered(gfx, tempStr, y, hTemp, wxColor(w.tempColor));
    y += hTemp; prev = true;
  }
  if (hasCond) {
    if (prev) y += gap;
    gfx->setFont(condFont);
    drawFontCentered(gfx, condStr, y, hCond, wxColor(w.condColor));
    y += hCond; prev = true;
  }
  if (hasPrecip) {
    if (prev) y += gap;
    gfx->setFont(precFont);
    drawFontCentered(gfx, precStr, y, hPrec, wxColor(w.precipColor));
    y += hPrec; prev = true;
  }
  if (hasTrend) {
    if (prev) y += gap;
    drawTrend(gfx, d, WX_TREND_MARGIN, y, TFT_WIDTH - 2 * WX_TREND_MARGIN, hTrend,
              wxColor(w.trendColor));
    y += hTrend;
  }

  // If nothing at all rendered (all toggles off before first data), show a hint.
  if (!hasTemp && !hasCond && !hasPrecip && !hasTrend)
    gfxDrawCentered(d.valid ? "no elements" : "--", 116, 2, C_GRAY);

  // Restore the built-in font + plain ASCII printing for shared status screens.
  gfx->setFont((const GFXfont*)nullptr);
  gfx->setUTF8Print(false);
  gfx->setTextSize(1);
}
