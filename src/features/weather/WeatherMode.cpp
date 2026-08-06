#include "WeatherMode.h"
#include <Arduino_GFX_Library.h>
#include <math.h>
#include "Gfx.h"
#include "WeatherClient.h"
#include "WeatherData.h"
#include "weather_icons.h"      // vendored open_iconic_weather fonts + wmoIcon()
#include "u8g2_clock_fonts.h"   // shared vendored U8g2 text fonts (see clock header)

WeatherMode g_weatherMode;

// --- layout (240x240) ------------------------------------------------------
// The weather face is a vertically-centred STACK of independently toggleable
// blocks, laid out top->bottom and auto-fitted to 240x240 (the same measure-then-
// place idea as the clock face):
//   A Now-primary (big temp + optional big icon)   B Condition text
//   C Detail line (feels / humidity / wind / rain)  D Hourly strip (N columns)
//   E Daily strip (N columns)                       F Temperature trend
// Every enabled block gets a full-width band sized to its own content; the gap
// between blocks tightens if an extreme combination overflows the screen.
static const int WX_GAP        = 6;    // vertical gap between stacked blocks
static const int WX_ROWGAP     = 1;    // gap between sub-rows inside a strip column
static const int WX_ICON_BIG   = 32;   // 4x icon box (px)
static const int WX_ICON_MINI  = 16;   // 2x icon box (px)
static const int WX_ICON_GAP   = 8;    // gap between the big icon and the temperature
static const int WX_TREND_H    = 40;   // sparkline band height (px)
static const int WX_TREND_MARGIN = 20; // left/right inset of the sparkline
static const int WX_DETAIL_MARGIN  = 6; // side inset used by the detail-line wrap
static const int WX_DETAIL_LINEGAP = 2; // gap between the two detail lines

// German short weekday names, index 0=Sun .. 6=Sat (ASCII, matches the fonts).
static const char* kWeekdayShortDE[7] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa" };

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

// Semantic weather-icon colour: pick the tint from the WMO weather_code (+ is_day)
// so each single-colour open_iconic glyph still reads correctly — sun→yellow,
// moon/clear-night & snow→white, cloud/fog→grey, rain/drizzle/thunder→a muted
// blue-grey (C_SELFBLUE_DK). Used for both the big icon and the strip mini icons.
static uint16_t wxIconColor(int code, bool isDay) {
  if (code < 0) return C_GRAY;
  switch (code) {
    case 0: case 1: case 2:                 // clear / mainly / partly cloudy
      return isDay ? C_YELLOW : C_WHITE;
    case 3: case 45: case 48:               // overcast / fog
      return C_GRAY;
    case 71: case 73: case 75: case 77:     // snow
    case 85: case 86:
      return C_WHITE;
    case 51: case 53: case 55: case 56: case 57:   // drizzle
    case 61: case 63: case 65: case 66: case 67:   // rain
    case 80: case 81: case 82:                     // showers
    case 95: case 96: case 99:                     // thunderstorm
      return C_SELFBLUE_DK;
    default:
      return C_GRAY;
  }
}

// Resolve the icon colour for one weather code under the current icon-colour mode.
static uint16_t wxResolveIconColor(const WeatherSettings& w, int code, bool isDay) {
  return w.iconColorMode == WX_ICONCOL_SEMANTIC ? wxIconColor(code, isDay)
                                                : wxColor(w.bigIconColor);
}

// Ink height of `s` in the currently-set U8g2 font.
static int fontHeight(Arduino_GFX* gfx, const char* s) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return (int)h;
}

// Ink width of `s` in the currently-set U8g2 font.
static int fontWidth(Arduino_GFX* gfx, const char* s) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return (int)w;
}

// Draw `s` centred horizontally on `cx` and vertically within [bandY, bandY+bandH)
// using the currently-set U8g2 font.
static void drawCenteredAt(Arduino_GFX* gfx, const char* s, int cx, int bandY,
                           int bandH, uint16_t color) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  int px = cx - (int)w / 2 - x1;
  int py = bandY + (bandH - (int)h) / 2 - y1;
  gfx->setTextColor(color);
  gfx->setCursor(px, py);
  gfx->print(s);
}

// Draw `s` centred on the whole screen width, within [bandY, bandY+bandH).
static void drawFontCentered(Arduino_GFX* gfx, const char* s, int bandY, int bandH,
                             uint16_t color) {
  drawCenteredAt(gfx, s, TFT_WIDTH / 2, bandY, bandH, color);
}

// Draw `s` left-aligned at `x`, vertically centred within [bandY, bandY+bandH).
static void drawLeftAt(Arduino_GFX* gfx, const char* s, int x, int bandY,
                       int bandH, uint16_t color) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  int py = bandY + (bandH - (int)h) / 2 - y1;
  gfx->setTextColor(color);
  gfx->setCursor(x - x1, py);
  gfx->print(s);
}

// Draw `s` right-aligned so its ink ends at `xRight`, vertically centred.
static void drawRightAt(Arduino_GFX* gfx, const char* s, int xRight, int bandY,
                        int bandH, uint16_t color) {
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  int py = bandY + (bandH - (int)h) / 2 - y1;
  gfx->setTextColor(color);
  gfx->setCursor(xRight - (int)w - x1, py);
  gfx->print(s);
}

// Largest CLK_PROP_FONTS index <= maxIdx whose "Mg" ink height fits `availH`.
// Used by the full-screen pages to grow the font as the row/list count shrinks.
static uint8_t wxFitPropFont(Arduino_GFX* gfx, int availH, uint8_t maxIdx) {
  if (maxIdx > CLK_PROP_FONT_MAX) maxIdx = CLK_PROP_FONT_MAX;
  for (int i = maxIdx; i > 0; i--) {
    gfx->setFont(CLK_PROP_FONTS[i]);
    if (fontHeight(gfx, "Mg") <= availH) return (uint8_t)i;
  }
  return 0;
}

// ---- weather icon (declared in weather_icons.h) ---------------------------
// Sets the icon font, prints the glyph centred on (cx) within [topY,topY+boxH),
// and leaves the icon font selected (callers always setFont before text again).
void drawWeatherIcon(Arduino_GFX* gfx, char glyph, int cx, int topY, int boxH,
                     bool big, uint16_t color) {
  gfx->setFont(big ? u8g2_font_open_iconic_weather_4x_t
                   : u8g2_font_open_iconic_weather_2x_t);
  char s[2] = { glyph, 0 };
  drawCenteredAt(gfx, s, cx, topY, boxH, color);
}

// ---- C: pack the detail tokens into up to two centred lines ---------------
// Greedily fills line 1, then line 2, so each line's ink width fits `maxW` in
// the given proportional font. Returns the line count (0/1/2); any tokens that
// still don't fit are appended to line 2 (the fit loop then shrinks the font).
static int wxWrapDetail(Arduino_GFX* gfx, const uint8_t* font,
                        const char tok[][24], int nTok, int maxW,
                        char* l1, size_t l1n, char* l2, size_t l2n) {
  l1[0] = 0; l2[0] = 0;
  if (nTok <= 0) return 0;
  gfx->setFont(font);
  int line = 0;
  for (int i = 0; i < nTok; i++) {
    char* cur = line == 0 ? l1 : l2;
    size_t curn = line == 0 ? l1n : l2n;
    if (!cur[0]) { strlcpy(cur, tok[i], curn); continue; }
    char cand[80];
    snprintf(cand, sizeof(cand), "%s  %s", cur, tok[i]);
    if (fontWidth(gfx, cand) <= maxW) {
      strlcpy(cur, cand, curn);
    } else if (line == 0) {
      line = 1; strlcpy(l2, tok[i], l2n);
    } else {
      char c2[80]; snprintf(c2, sizeof(c2), "%s  %s", l2, tok[i]); strlcpy(l2, c2, l2n);
    }
  }
  return l2[0] ? 2 : 1;
}

// ---- A: now-primary (big temp + optional big icon) ------------------------
// The temperature keeps its hollow degree ring (the number fonts carry no '°');
// the icon sits to its left. The whole group is centred as a unit.
static void drawPrimary(Arduino_GFX* gfx, const char* num, bool showIcon, char glyph,
                        int bandY, int bandH, const uint8_t* tempFont,
                        uint16_t tempCol, uint16_t iconCol) {
  gfx->setFont(tempFont);
  int16_t x1, y1; uint16_t w, h;
  gfx->getTextBounds(num, 0, 0, &x1, &y1, &w, &h);
  int r   = (int)h / 10; if (r < 3) r = 3;   // ring radius scales with the font
  int gap = r;
  int tempTotal = (int)w + gap + 2 * r;      // number + gap + ring

  int iconW = 0;
  if (showIcon) {
    char gs[2] = { glyph, 0 };
    gfx->setFont(u8g2_font_open_iconic_weather_4x_t);
    iconW = fontWidth(gfx, gs) + WX_ICON_GAP;
  }
  int total  = iconW + tempTotal;
  int startX = (TFT_WIDTH - total) / 2;

  if (showIcon) {
    drawWeatherIcon(gfx, glyph, startX + (iconW - WX_ICON_GAP) / 2, bandY, bandH, true, iconCol);
  }

  // Temperature number, vertically centred in the band.
  gfx->setFont(tempFont);
  int inkTop  = bandY + (bandH - (int)h) / 2;
  int numX    = startX + iconW - x1;
  gfx->setTextColor(tempCol);
  gfx->setCursor(numX, inkTop - y1);
  gfx->print(num);

  // Hollow degree ring at the number's top-right.
  int ringCx = startX + iconW + x1 + (int)w + gap + r;
  int ringCy = inkTop + r;
  int th = r / 2; if (th < 1) th = 1;
  gfx->fillCircle(ringCx, ringCy, r, tempCol);
  gfx->fillCircle(ringCx, ringCy, r - th, C_BLACK);
}

// ---- D/E: an evenly-spaced column strip -----------------------------------
// Draws `cols` columns across the full width. Each column stacks up to four
// sub-rows (top->bottom): a top label, a mini icon, a middle text and a bottom
// text — any of which may be disabled (row heights come from the caller).
struct StripRow { bool on; int h; };

static void drawStrip(Arduino_GFX* gfx, int cols, int bandY,
                      const uint8_t* font, uint16_t textCol, uint16_t popCol,
                      // per-column content callbacks are inlined by the caller
                      const char** topStr, const char* iconGlyphs, const uint16_t* iconCols,
                      const char** midStr,
                      const char** botStr, StripRow top, StripRow icon, StripRow mid,
                      StripRow bot) {
  int colW = TFT_WIDTH / cols;
  for (int c = 0; c < cols; c++) {
    int cx = colW * c + colW / 2;
    int y = bandY;
    if (top.on) { gfx->setFont(font); drawCenteredAt(gfx, topStr[c], cx, y, top.h, textCol); y += top.h + WX_ROWGAP; }
    if (icon.on) { drawWeatherIcon(gfx, iconGlyphs[c], cx, y, icon.h, false, iconCols[c]); y += icon.h + WX_ROWGAP; }
    if (mid.on) { gfx->setFont(font); drawCenteredAt(gfx, midStr[c], cx, y, mid.h, textCol); y += mid.h + WX_ROWGAP; }
    if (bot.on) { gfx->setFont(font); drawCenteredAt(gfx, botStr[c], cx, y, bot.h, popCol); }
  }
}

// ---- F: 12h temperature sparkline -----------------------------------------
static void drawTrend(Arduino_GFX* gfx, const WeatherData& d, int x, int y, int w,
                      int h, uint16_t col, bool labels, const uint8_t* labelFont) {
  if (d.nHours < 2) return;
  float mn = d.hours[0].temp, mx = d.hours[0].temp;
  for (uint8_t i = 1; i < d.nHours; i++) {
    if (d.hours[i].temp < mn) mn = d.hours[i].temp;
    if (d.hours[i].temp > mx) mx = d.hours[i].temp;
  }
  float span = mx - mn;
  if (span < 0.5f) span = 0.5f;             // flat series -> a mid line, no div-by-zero
  int n = d.nHours;
  int prevX = 0, prevY = 0;
  for (int i = 0; i < n; i++) {
    int px = x + (w - 1) * i / (n - 1);
    int py = y + (h - 1) - (int)((d.hours[i].temp - mn) / span * (h - 1));
    if (i > 0) gfx->drawLine(prevX, prevY, px, py, col);
    gfx->fillCircle(px, py, 1, col);
    prevX = px; prevY = py;
  }
  if (labels) {
    gfx->setFont(labelFont);
    char s[8];
    snprintf(s, sizeof(s), "%d", (int)lroundf(mx));
    gfx->setTextColor(col); gfx->setCursor(2, y + fontHeight(gfx, s)); gfx->print(s);
    snprintf(s, sizeof(s), "%d", (int)lroundf(mn));
    gfx->setCursor(2, y + h); gfx->print(s);
  }
}

void WeatherMode::begin(const Settings& s) {
  weatherInit(s);
  needFull_ = true;
  pageReset_ = true;
  lastStamp_ = 0xFFFFFFFF;
  lastPage_ = 0xFF;
}

void WeatherMode::invalidate(const Settings& s) {
  weatherInit(s);
  weatherForceRefresh();                    // lat/lon/units may have changed -> refetch
  needFull_ = true;
  pageReset_ = true;                         // page set/order/dwell may have changed
  lastStamp_ = 0xFFFFFFFF;
  lastPage_ = 0xFF;
}

// Ordered list of enabled pages (ascending order number, ties by page id). Packs
// (order<<4 | id) into one comparable byte and insertion-sorts the small set.
uint8_t WeatherMode::buildPageList(const WeatherSettings& w, uint8_t* out) const {
  const bool    en[WX_PAGE_COUNT] = { w.pageNow, w.pageTemp, w.pageRain, w.pageDays };
  const uint8_t ord[WX_PAGE_COUNT] = { w.orderNow, w.orderTemp, w.orderRain, w.orderDays };
  uint8_t key[WX_PAGE_COUNT]; uint8_t n = 0;
  for (uint8_t id = 0; id < WX_PAGE_COUNT; id++) {
    if (!en[id]) continue;
    uint8_t k = (uint8_t)((ord[id] << 4) | id);
    uint8_t j = n;
    while (j > 0 && key[j - 1] > k) { key[j] = key[j - 1]; out[j] = out[j - 1]; j--; }
    key[j] = k; out[j] = id; n++;
  }
  if (n == 0) { out[0] = WX_PAGE_NOW; n = 1; }   // nothing enabled -> show NOW
  return n;
}

void WeatherMode::service(const Settings& s) {
  weatherService(s);                        // fetch on the schedule (no-op between polls)
  const WeatherData& d = weatherGet();
  const WeatherSettings& w = s.weather;

  // ---- pick the page to show (internal carousel) ----
  uint8_t pages[WX_PAGE_COUNT];
  uint8_t nPages = buildPageList(w, pages);
  bool     cycle  = w.cyclePages && nPages > 1;
  uint32_t dwellMs = (uint32_t)constrain((int)w.pageDwellSec, WX_PAGEDWELL_MIN, WX_PAGEDWELL_MAX) * 1000UL;

  if (pageReset_) {                          // boot / wake / settings change
    pageSlot_ = 0;
    pageStart_ = millis();
    pageReset_ = false;
    needFull_ = true;
  } else if (cycle && (millis() - pageStart_) >= dwellMs) {
    pageSlot_ = (uint8_t)((pageSlot_ + 1) % nPages);
    pageStart_ = millis();
    needFull_ = true;
  }
  if (pageSlot_ >= nPages) { pageSlot_ = 0; needFull_ = true; }   // list shrank
  uint8_t page = pages[pageSlot_];

  uint32_t stamp = d.valid ? d.lastOkMs : 0;
  if (needFull_ || stamp != lastStamp_ || page != lastPage_) {
    renderPage(s, d, page);
    lastStamp_ = stamp;
    lastPage_ = page;
    needFull_ = false;
  }
}

void WeatherMode::renderPage(const Settings& s, const WeatherData& d, uint8_t page) {
  switch (page) {
    case WX_PAGE_TEMP: renderTempTrend(s, d); break;
    case WX_PAGE_RAIN: renderRainTrend(s, d); break;
    case WX_PAGE_DAYS: renderDays7(s, d);     break;
    default:           renderNow(s, d);       break;
  }
}

void WeatherMode::renderNow(const Settings& s, const WeatherData& d) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;
  const WeatherSettings& w = s.weather;

  gfx->setUTF8Print(true);
  gfx->setTextSize(1);

  // Resolve per-block fonts (indices clamped defensively against the tables).
  uint8_t tSz = w.tempSize   < CLK_NUM_FONT_COUNT  ? w.tempSize   : DEFAULT_WX_TEMPSIZE;
  uint8_t cSz = w.condSize   < CLK_PROP_FONT_COUNT ? w.condSize   : DEFAULT_WX_CONDSIZE;
  uint8_t dSz = w.detailSize < CLK_PROP_FONT_COUNT ? w.detailSize : DEFAULT_WX_DETAILSIZE;
  uint8_t hSz = w.hourlySize < CLK_PROP_FONT_COUNT ? w.hourlySize : DEFAULT_WX_HOURLYSIZE;
  uint8_t ySz = w.dailySize  < CLK_PROP_FONT_COUNT ? w.dailySize  : DEFAULT_WX_DAILYSIZE;

  // ---- build primary / condition / detail strings ----
  char tempStr[12];
  if (d.valid) snprintf(tempStr, sizeof(tempStr), "%d", (int)lroundf(d.temp));
  else         strlcpy(tempStr, "--", sizeof(tempStr));
  const char* condStr = d.valid ? weatherCodeDE(d.code) : "";

  // Detail values as short independent tokens; they are packed into <=2 centred
  // lines below (see wxWrapDetail) so an all-on detail block never runs off the
  // 240 px width. The proportional fonts are ASCII-only (no '°') -> plain numbers.
  char detailTok[4][24]; int nTok = 0;
  if (d.valid) {
    if (w.showFeels) snprintf(detailTok[nTok++], 24, "Gefuehlt %d", (int)lroundf(d.feelsLike));
    if (w.showHum)   snprintf(detailTok[nTok++], 24, "Luft %d%%", d.humidity);
    if (w.showWind)  snprintf(detailTok[nTok++], 24, "Wind %d", (int)lroundf(d.wind));
    if (w.showPrecip) {
      int pop = d.nHours ? d.hours[0].pop : 0;
      if      (w.precipMode == WX_PRECIP_PCT)  snprintf(detailTok[nTok++], 24, "Regen %d%%", pop);
      else if (w.precipMode == WX_PRECIP_BOTH) snprintf(detailTok[nTok++], 24, "Regen %d%% %.1fmm", pop, d.precip);
      else                                     snprintf(detailTok[nTok++], 24, "Regen %.1f mm", d.precip);
    }
  }

  // ---- fixed (font-size-independent) facts about which blocks are present ----
  const int detailMax = TFT_WIDTH - 2 * WX_DETAIL_MARGIN;
  bool showTempP  = w.showTemp;
  bool showIconP  = w.showBigIcon && d.valid;
  bool hasPrimary = (w.showTemp || w.showBigIcon);
  bool hasCond    = w.showCond && condStr[0];
  bool hasHourly  = w.showHourly && d.nHours > 0;
  bool hasDaily   = w.showDaily  && d.nDays  > 0;
  bool hasTrend   = w.showTrend  && d.nHours >= 2;

  int hourlyStep = w.hourlyStep < WX_HOURLY_STEP_MIN ? WX_HOURLY_STEP_MIN : w.hourlyStep;
  int hourlyCols = 0;
  if (hasHourly) {
    int want = w.hourlyCount < WX_HOURLY_COUNT_MIN ? WX_HOURLY_COUNT_MIN : w.hourlyCount;
    for (int c = 0; c < want && (int)(c * hourlyStep) < d.nHours; c++) hourlyCols++;
    if (hourlyCols < 1) hasHourly = false;
  }
  int dailyCols = 0;
  if (hasDaily) {
    int want = w.dailyCount < WX_DAILY_COUNT_MIN ? WX_DAILY_COUNT_MIN : w.dailyCount;
    dailyCols = want < d.nDays ? want : d.nDays;
    if (dailyCols < 1) hasDaily = false;
  }

  // ---- measure everything for a candidate set of size indices ----
  // Called repeatedly by the fit loop: shrinking the size indices re-measures
  // until the stack fits 240 px, so nothing is ever clipped (see below).
  bool present[6];
  int  heights[6];
  int  hrH = 0, hrI = 0, hrT = 0, hrP = 0;
  int  dyH = 0, dyI = 0, dyT = 0, dyP = 0;
  char dl1[64], dl2[64]; int detailLines = 0;

  auto measureAll = [&](uint8_t tSz, uint8_t cSz, uint8_t dSz, uint8_t hSz, uint8_t ySz) {
    for (int i = 0; i < 6; i++) { present[i] = false; heights[i] = 0; }
    // A primary
    if (hasPrimary) {
      int th = 0;
      if (showTempP) { gfx->setFont(CLK_NUM_FONTS[tSz]); th = fontHeight(gfx, "-8"); }
      int ih = showIconP ? WX_ICON_BIG : 0;
      int hP = th > ih ? th : ih;
      if (hP > 0) { present[0] = true; heights[0] = hP; }
    }
    // B condition
    if (hasCond) { gfx->setFont(CLK_PROP_FONTS[cSz]); present[1] = true; heights[1] = fontHeight(gfx, "Mg"); }
    // C detail (packed into <=2 lines that each fit the width in this font)
    detailLines = wxWrapDetail(gfx, CLK_PROP_FONTS[dSz], detailTok, nTok, detailMax,
                               dl1, sizeof(dl1), dl2, sizeof(dl2));
    if (detailLines > 0) {
      gfx->setFont(CLK_PROP_FONTS[dSz]);
      int lineH = fontHeight(gfx, "0g%");
      present[2] = true;
      heights[2] = detailLines * lineH + (detailLines - 1) * WX_DETAIL_LINEGAP;
    }
    // D hourly
    if (hasHourly) {
      gfx->setFont(CLK_PROP_FONTS[hSz]); int hrRow = fontHeight(gfx, "88h");
      hrH = w.hrHour ? hrRow : 0; hrI = w.hrIcon ? WX_ICON_MINI : 0;
      hrT = w.hrTemp ? hrRow : 0; hrP = w.hrPop  ? hrRow : 0;
      int rows = (hrH?1:0) + (hrI?1:0) + (hrT?1:0) + (hrP?1:0);
      int hh = hrH + hrI + hrT + hrP + (rows > 1 ? (rows - 1) * WX_ROWGAP : 0);
      if (hh > 0) { present[3] = true; heights[3] = hh; }
    }
    // E daily
    if (hasDaily) {
      gfx->setFont(CLK_PROP_FONTS[ySz]); int dyRow = fontHeight(gfx, "88/8");
      dyH = w.dyDay ? dyRow : 0; dyI = w.dyIcon ? WX_ICON_MINI : 0;
      dyT = w.dyTemps ? dyRow : 0; dyP = w.dyPop ? dyRow : 0;
      int rows = (dyH?1:0) + (dyI?1:0) + (dyT?1:0) + (dyP?1:0);
      int hh = dyH + dyI + dyT + dyP + (rows > 1 ? (rows - 1) * WX_ROWGAP : 0);
      if (hh > 0) { present[4] = true; heights[4] = hh; }
    }
    // F trend
    if (hasTrend) { present[5] = true; heights[5] = WX_TREND_H; }
  };

  auto stackH = [&](int gap) {
    int H = 0; bool first = true;
    for (int i = 0; i < 6; i++) if (present[i]) { if (!first) H += gap; H += heights[i]; first = false; }
    return H;
  };

  // Working size indices (start from the settings, clamped).
  uint8_t sz[5] = { tSz, cSz, dSz, hSz, ySz };
  measureAll(sz[0], sz[1], sz[2], sz[3], sz[4]);

  // ---- fit to 240: shrink block font sizes (tallest blocks first) ----
  // Instead of tightening gaps and clipping the lowest block, step the size
  // indices down until the measured stack fits, so content stays on-screen and
  // legible. Blocks 0..4 carry a font size (5=trend is fixed-height).
  for (int guard = 0; guard < 64; guard++) {
    if (stackH(WX_GAP) <= TFT_HEIGHT) break;
    int bestBlk = -1, bestH = -1;
    for (int b = 0; b < 5; b++)
      if (present[b] && sz[b] > 0 && heights[b] > bestH) { bestH = heights[b]; bestBlk = b; }
    if (bestBlk < 0) break;                 // nothing left to shrink -> gap-tighten below
    sz[bestBlk]--;
    measureAll(sz[0], sz[1], sz[2], sz[3], sz[4]);
  }

  // Last-resort gap tighten (only if an extreme combo still overflows at minimum
  // sizes) so we never centre off the top edge.
  int gap = WX_GAP, top;
  for (;;) {
    top = (TFT_HEIGHT - stackH(gap)) / 2;
    if (top >= 0 || gap <= 1) break;
    gap -= 1;
  }
  if (top < 0) top = 0;

  const uint8_t* tempFontF   = CLK_NUM_FONTS[sz[0]];
  const uint8_t* condFontF   = CLK_PROP_FONTS[sz[1]];
  const uint8_t* detailFontF = CLK_PROP_FONTS[sz[2]];
  const uint8_t* hourlyFontF = CLK_PROP_FONTS[sz[3]];
  const uint8_t* dailyFontF  = CLK_PROP_FONTS[sz[4]];

  // ---- draw ----
  gfx->fillScreen(C_BLACK);
  int y = top;
  bool prev = false;

  if (present[0]) {
    char glyph = wmoIcon(d.code, d.isDay);
    uint16_t iconCol = wxResolveIconColor(w, d.code, d.isDay);
    if (d.valid && w.showTemp) {
      drawPrimary(gfx, tempStr, w.showBigIcon, glyph, y, heights[0],
                  tempFontF, wxColor(w.tempColor), iconCol);
    } else if (d.valid && w.showBigIcon) {
      drawWeatherIcon(gfx, glyph, TFT_WIDTH / 2, y, heights[0], true, iconCol);
    } else if (w.showTemp) {
      gfx->setFont(tempFontF);
      drawFontCentered(gfx, tempStr, y, heights[0], wxColor(w.tempColor));
    }
    y += heights[0]; prev = true;
  }
  if (present[1]) {
    if (prev) y += gap;
    gfx->setFont(condFontF);
    drawFontCentered(gfx, condStr, y, heights[1], wxColor(w.condColor));
    y += heights[1]; prev = true;
  }
  if (present[2]) {
    if (prev) y += gap;
    gfx->setFont(detailFontF);
    int lineH = fontHeight(gfx, "0g%");
    int yy = y;
    drawFontCentered(gfx, dl1, yy, lineH, wxColor(w.detailColor));
    if (detailLines > 1) { yy += lineH + WX_DETAIL_LINEGAP; drawFontCentered(gfx, dl2, yy, lineH, wxColor(w.detailColor)); }
    y += heights[2]; prev = true;
  }
  if (present[3]) {
    if (prev) y += gap;
    const char* topS[WX_HOURLY_COUNT_MAX];
    const char* midS[WX_HOURLY_COUNT_MAX];
    const char* botS[WX_HOURLY_COUNT_MAX];
    char iconG[WX_HOURLY_COUNT_MAX];
    uint16_t iconC[WX_HOURLY_COUNT_MAX];
    static char buf[WX_HOURLY_COUNT_MAX][3][8];   // hour / temp / pop text
    for (int c = 0; c < hourlyCols; c++) {
      const WxHour& hh = d.hours[c * hourlyStep];
      snprintf(buf[c][0], 8, "%dh", hh.hour);
      snprintf(buf[c][1], 8, "%d", (int)lroundf(hh.temp));
      snprintf(buf[c][2], 8, "%d%%", hh.pop);
      topS[c] = buf[c][0]; midS[c] = buf[c][1]; botS[c] = buf[c][2];
      iconG[c] = wmoIcon(hh.code, true);
      iconC[c] = wxResolveIconColor(w, hh.code, true);
    }
    drawStrip(gfx, hourlyCols, y, hourlyFontF, wxColor(w.hourlyColor), wxColor(w.hourlyPop),
              topS, iconG, iconC, midS, botS,
              { (bool)w.hrHour, hrH }, { (bool)w.hrIcon, hrI },
              { (bool)w.hrTemp, hrT }, { (bool)w.hrPop, hrP });
    y += heights[3]; prev = true;
  }
  if (present[4]) {
    const char* topS[WX_DAILY_COUNT_MAX];
    const char* midS[WX_DAILY_COUNT_MAX];
    const char* botS[WX_DAILY_COUNT_MAX];
    char iconG[WX_DAILY_COUNT_MAX];
    uint16_t iconC[WX_DAILY_COUNT_MAX];
    static char dbuf[WX_DAILY_COUNT_MAX][3][10];  // day / temps / pop text
    if (prev) y += gap;
    for (int c = 0; c < dailyCols; c++) {
      const WxDay& dd = d.days[c];
      strlcpy(dbuf[c][0], kWeekdayShortDE[dd.wday < 7 ? dd.wday : 0], 10);
      snprintf(dbuf[c][1], 10, "%d/%d", (int)lroundf(dd.tmax), (int)lroundf(dd.tmin));
      snprintf(dbuf[c][2], 10, "%d%%", dd.popMax);
      topS[c] = dbuf[c][0]; midS[c] = dbuf[c][1]; botS[c] = dbuf[c][2];
      iconG[c] = wmoIcon(dd.code, true);
      iconC[c] = wxResolveIconColor(w, dd.code, true);
    }
    drawStrip(gfx, dailyCols, y, dailyFontF, wxColor(w.dailyColor), wxColor(w.dailyPop),
              topS, iconG, iconC, midS, botS,
              { (bool)w.dyDay, dyH }, { (bool)w.dyIcon, dyI },
              { (bool)w.dyTemps, dyT }, { (bool)w.dyPop, dyP });
    y += heights[4]; prev = true;
  }
  if (present[5]) {
    if (prev) y += gap;
    drawTrend(gfx, d, WX_TREND_MARGIN, y, TFT_WIDTH - 2 * WX_TREND_MARGIN, WX_TREND_H,
              wxColor(w.trendColor), w.trendLabels, CLK_PROP_FONTS[0]);
    y += WX_TREND_H;
  }

  // Nothing enabled (or no data yet with everything off) -> a neutral hint.
  if (!present[0] && !present[1] && !present[2] && !present[3] && !present[4] && !present[5])
    gfxDrawCentered(d.valid ? "no blocks" : "--", 116, 2, C_GRAY);

  // Restore the built-in font + plain ASCII printing for shared status screens.
  gfx->setFont((const GFXfont*)nullptr);
  gfx->setUTF8Print(false);
  gfx->setTextSize(1);
}

// ===========================================================================
// Full-screen pages (TEMP / RAIN / DAYS) — each uses the whole 240x240 with a
// title band up top, comfortable margins, and NUMBER labels so the values read
// even across the room. All share the clock font tables + weather colours.
// ===========================================================================
static const int WX_PG_TITLE_Y = 6;      // title band top inset
static const int WX_PG_MARGIN_B = 26;    // bottom band reserved for hour labels

// Draw a centred page title in helvB12; return the y just below the title band.
static int drawPageTitle(Arduino_GFX* gfx, const char* title, uint16_t col) {
  gfx->setFont(CLK_PROP_FONTS[2]);       // helvB12
  int h = fontHeight(gfx, "Mg");
  drawFontCentered(gfx, title, WX_PG_TITLE_Y, h, col);
  return WX_PG_TITLE_Y + h;
}

// Shared page prologue/epilogue keep the UTF8/size flags consistent.
static void wxPageBegin(Arduino_GFX* gfx) {
  gfx->setUTF8Print(true);
  gfx->setTextSize(1);
  gfx->fillScreen(C_BLACK);
}
static void wxPageEnd(Arduino_GFX* gfx) {
  gfx->setFont((const GFXfont*)nullptr);
  gfx->setUTF8Print(false);
  gfx->setTextSize(1);
}

// ---- TEMP_TREND: full-screen hourly temperature line chart ------------------
void WeatherMode::renderTempTrend(const Settings& s, const WeatherData& d) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;
  const WeatherSettings& w = s.weather;
  wxPageBegin(gfx);

  uint16_t curveCol = wxColor(w.trendColor);
  uint16_t valCol   = wxColor(w.tempColor);
  int titleBot = drawPageTitle(gfx, "Temperatur", wxColor(w.condColor));

  if (!d.valid || d.nHours < 2) {
    gfx->setFont(CLK_PROP_FONTS[3]);
    drawFontCentered(gfx, d.valid ? "--" : "...", 108, 24, C_GRAY);
    wxPageEnd(gfx);
    return;
  }

  int n = d.nHours;
  float mn = d.hours[0].temp, mx = d.hours[0].temp;
  for (int i = 1; i < n; i++) {
    if (d.hours[i].temp < mn) mn = d.hours[i].temp;
    if (d.hours[i].temp > mx) mx = d.hours[i].temp;
  }
  float span = mx - mn; if (span < 0.5f) span = 0.5f;

  const int X0 = 30, X1 = TFT_WIDTH - 12;
  const int Y0 = titleBot + 24;              // room above for the max label
  const int Y1 = TFT_HEIGHT - WX_PG_MARGIN_B;
  const int PH = Y1 - Y0;

  // y-axis min/max value labels (left inset)
  char sb[8];
  gfx->setFont(CLK_PROP_FONTS[0]);           // helvB08
  snprintf(sb, sizeof(sb), "%d", (int)lroundf(mx));
  gfx->setTextColor(valCol); gfx->setCursor(2, Y0 + fontHeight(gfx, sb)); gfx->print(sb);
  snprintf(sb, sizeof(sb), "%d", (int)lroundf(mn));
  gfx->setCursor(2, Y1); gfx->print(sb);

  gfx->drawLine(X0, Y1, X1, Y1, C_GRAY);     // baseline

  // curve (drawn 2px thick for legibility)
  int prevX = 0, prevY = 0;
  for (int i = 0; i < n; i++) {
    int px = X0 + (X1 - X0) * i / (n - 1);
    int py = Y1 - (int)((d.hours[i].temp - mn) / span * PH);
    if (i > 0) { gfx->drawLine(prevX, prevY, px, py, curveCol);
                 gfx->drawLine(prevX, prevY + 1, px, py + 1, curveCol); }
    gfx->fillCircle(px, py, 2, curveCol);
    prevX = px; prevY = py;
  }

  // NUMBER labels at first / mid / last points
  gfx->setFont(CLK_PROP_FONTS[1]);           // helvB10
  int fh = fontHeight(gfx, "8");
  int marks[3] = { 0, n / 2, n - 1 };
  for (int m = 0; m < 3; m++) {
    int i = marks[m];
    int px = X0 + (X1 - X0) * i / (n - 1);
    int py = Y1 - (int)((d.hours[i].temp - mn) / span * PH);
    snprintf(sb, sizeof(sb), "%d", (int)lroundf(d.hours[i].temp));
    int tw = fontWidth(gfx, sb);
    int tx = px - tw / 2; if (tx < 0) tx = 0; if (tx + tw > TFT_WIDTH) tx = TFT_WIDTH - tw;
    int ty = py - 4;                          // baseline above the point
    if (ty - fh < Y0) ty = py + fh + 4;       // flip below if it would clip the top
    gfx->setTextColor(valCol); gfx->setCursor(tx, ty); gfx->print(sb);
  }

  // x-axis hour labels (first / mid / last)
  gfx->setFont(CLK_PROP_FONTS[0]);
  for (int m = 0; m < 3; m++) {
    int i = marks[m];
    int px = X0 + (X1 - X0) * i / (n - 1);
    snprintf(sb, sizeof(sb), "%dh", d.hours[i].hour);
    drawCenteredAt(gfx, sb, px, Y1 + 4, 16, C_GRAY);
  }

  wxPageEnd(gfx);
}

// ---- RAIN_TREND: full-screen hourly precipitation probability --------------
// Two readable styles: wide labelled bars (fewer than the old 12 so each % is
// big), or a two-column list ("15 Uhr   40%") in a large auto-fitted font.
void WeatherMode::renderRainTrend(const Settings& s, const WeatherData& d) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;
  const WeatherSettings& w = s.weather;
  wxPageBegin(gfx);

  uint16_t barCol  = wxColor(w.hourlyPop);
  uint16_t textCol = wxColor(w.hourlyColor);
  int titleBot = drawPageTitle(gfx, "Regen %", wxColor(w.condColor));

  if (!d.valid || d.nHours < 1) {
    gfx->setFont(CLK_PROP_FONTS[3]);
    drawFontCentered(gfx, d.valid ? "--" : "...", 108, 24, C_GRAY);
    wxPageEnd(gfx);
    return;
  }

  // How many hours to show (clamped to the buffer we actually hold).
  int want = constrain((int)w.rainHours, WX_RAIN_HOURS_MIN, WX_RAIN_HOURS_MAX);
  int n = want < d.nHours ? want : d.nHours;
  uint8_t lblMax = w.rainLabelSize < CLK_PROP_FONT_COUNT ? w.rainLabelSize : DEFAULT_WX_RAINLABELSIZE;

  char sb[12];

  if (w.rainStyle == WX_RAIN_LIST) {
    // ---- List style: big "HH Uhr    NN%" rows, auto-sized to fill the height.
    int top    = titleBot + 4;
    int availH = TFT_HEIGHT - top - 4;
    int rowH   = availH / n;
    uint8_t fi = wxFitPropFont(gfx, rowH - 3, lblMax);
    const uint8_t* font = CLK_PROP_FONTS[fi];
    const int LX = 22;                  // hour left inset
    const int RX = TFT_WIDTH - 22;      // % right inset
    for (int i = 0; i < n; i++) {
      int ry = top + rowH * i;
      gfx->setFont(font);
      snprintf(sb, sizeof(sb), "%d Uhr", d.hours[i].hour);
      drawLeftAt(gfx, sb, LX, ry, rowH, textCol);
      snprintf(sb, sizeof(sb), "%d%%", d.hours[i].pop);
      drawRightAt(gfx, sb, RX, ry, rowH, barCol);
    }
    wxPageEnd(gfx);
    return;
  }

  // ---- Bars style: fewer, wider bars so the % labels use a big font. --------
  const int X0 = 16, X1 = TFT_WIDTH - 8;
  const int Y0 = titleBot + 22;              // room above the tallest bar for its %
  const int Y1 = TFT_HEIGHT - WX_PG_MARGIN_B;
  const int PH = Y1 - Y0;
  int slot = (X1 - X0) / n;
  int barW = slot * 3 / 4; if (barW < 3) barW = 3;

  gfx->drawLine(X0, Y1, X1, Y1, C_GRAY);     // baseline (0%)

  const uint8_t* lblFont = CLK_PROP_FONTS[lblMax];
  gfx->setFont(lblFont);
  int lblH = fontHeight(gfx, "88");
  // Only thin the labels to every-other bar when a two-digit % is wider than the
  // slot (i.e. the 12-bar case at a big font); otherwise label every bar.
  int stride = (fontWidth(gfx, "88") > slot - 2) ? 2 : 1;

  for (int i = 0; i < n; i++) {
    int cx  = X0 + slot * i + slot / 2;
    int pop = d.hours[i].pop;
    int bh  = PH * pop / 100;
    int by  = Y1 - bh;
    gfx->fillRect(cx - barW / 2, by, barW, bh, barCol);
    bool label = (stride == 1) || (i % 2 == 0);
    if (label && pop > 0) {
      gfx->setFont(lblFont);
      snprintf(sb, sizeof(sb), "%d", pop);
      drawCenteredAt(gfx, sb, cx, by - lblH - 3, lblH, barCol);
    }
    if (label) {
      gfx->setFont(CLK_PROP_FONTS[0]);
      snprintf(sb, sizeof(sb), "%d", d.hours[i].hour);
      drawCenteredAt(gfx, sb, cx, Y1 + 4, 16, C_GRAY);
    }
  }

  wxPageEnd(gfx);
}

// ---- DAYS7: 7-day forecast, one row per day --------------------------------
void WeatherMode::renderDays7(const Settings& s, const WeatherData& d) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;
  const WeatherSettings& w = s.weather;
  wxPageBegin(gfx);

  uint16_t dayCol = wxColor(w.dailyColor);
  uint16_t popCol = wxColor(w.dailyPop);
  int titleBot = drawPageTitle(gfx, "7 Tage", wxColor(w.condColor));

  if (!d.valid || d.nDays < 1) {
    gfx->setFont(CLK_PROP_FONTS[3]);
    drawFontCentered(gfx, d.valid ? "--" : "...", 108, 24, C_GRAY);
    wxPageEnd(gfx);
    return;
  }

  // How many days to show (fewer rows -> bigger font).
  int want = constrain((int)w.daysCount, WX_DAYS_COUNT_MIN, WX_DAYS_COUNT_MAX);
  int rows = want < d.nDays ? want : d.nDays;
  if (rows > WX_DAILY_POINTS) rows = WX_DAILY_POINTS;

  int top  = titleBot + 4;
  int rowH = (TFT_HEIGHT - top) / rows;

  // Big rows earn a big (4x) icon; tight rows keep the 2x mini icon.
  bool bigIcon = rowH >= 42;
  int  iconW   = bigIcon ? 30 : WX_ICON_MINI;   // ~glyph width of the 4x/2x icon

  // Auto-fit the row font: start at the preferred size and shrink until every
  // row fits BOTH the row height and the 240 px width (widest weekday/temp/pop).
  uint8_t fiMax = w.daysRowSize < CLK_PROP_FONT_COUNT ? w.daysRowSize : DEFAULT_WX_DAYSROWSIZE;
  const int COLGAP = 8;
  char sb[12];
  int maxDayW = 0, maxTempW = 0, maxPopW = 0;
  uint8_t fi = fiMax;
  for (;; fi--) {
    gfx->setFont(CLK_PROP_FONTS[fi]);
    int fh = fontHeight(gfx, "Mg");
    maxDayW = maxTempW = maxPopW = 0;
    for (int r = 0; r < rows; r++) {
      const WxDay& dd = d.days[r];
      int dw = fontWidth(gfx, kWeekdayShortDE[dd.wday < 7 ? dd.wday : 0]);
      if (dw > maxDayW) maxDayW = dw;
      snprintf(sb, sizeof(sb), "%d/%d", (int)lroundf(dd.tmax), (int)lroundf(dd.tmin));
      int tw = fontWidth(gfx, sb); if (tw > maxTempW) maxTempW = tw;
      snprintf(sb, sizeof(sb), "%d%%", dd.popMax);
      int pw = fontWidth(gfx, sb); if (pw > maxPopW) maxPopW = pw;
    }
    int totalW = maxDayW + COLGAP + iconW + COLGAP + maxTempW + COLGAP + maxPopW;
    bool fits = (fh <= rowH - 2) && (totalW <= TFT_WIDTH - 12);
    if (fits || fi == 0) break;
  }
  const uint8_t* rowFont = CLK_PROP_FONTS[fi];

  // Evenly distribute the four columns across the width using the fitted widths:
  // weekday hard-left, rain% hard-right, icon after the weekday, temps centred in
  // the remaining gap — so the layout scales with the chosen font.
  int leftX  = 6;
  int rightX = TFT_WIDTH - 6;
  int dayCx  = leftX + maxDayW / 2;
  int popCx  = rightX - maxPopW / 2;
  int iconCx = dayCx + maxDayW / 2 + COLGAP + iconW / 2;
  int tempCx = ((iconCx + iconW / 2) + (popCx - maxPopW / 2)) / 2;

  for (int r = 0; r < rows; r++) {
    const WxDay& dd = d.days[r];
    int ry = top + rowH * r;

    gfx->setFont(rowFont);
    drawCenteredAt(gfx, kWeekdayShortDE[dd.wday < 7 ? dd.wday : 0], dayCx, ry, rowH, dayCol);

    uint16_t iconCol = wxResolveIconColor(w, dd.code, true);
    drawWeatherIcon(gfx, wmoIcon(dd.code, true), iconCx, ry, rowH, bigIcon, iconCol);

    gfx->setFont(rowFont);
    snprintf(sb, sizeof(sb), "%d/%d", (int)lroundf(dd.tmax), (int)lroundf(dd.tmin));
    drawCenteredAt(gfx, sb, tempCx, ry, rowH, dayCol);

    snprintf(sb, sizeof(sb), "%d%%", dd.popMax);
    drawCenteredAt(gfx, sb, popCx, ry, rowH, popCol);
  }

  wxPageEnd(gfx);
}
