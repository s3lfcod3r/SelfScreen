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
                      const char** topStr, const char* iconGlyphs, const char** midStr,
                      const char** botStr, StripRow top, StripRow icon, StripRow mid,
                      StripRow bot) {
  int colW = TFT_WIDTH / cols;
  for (int c = 0; c < cols; c++) {
    int cx = colW * c + colW / 2;
    int y = bandY;
    if (top.on) { gfx->setFont(font); drawCenteredAt(gfx, topStr[c], cx, y, top.h, textCol); y += top.h + WX_ROWGAP; }
    if (icon.on) { drawWeatherIcon(gfx, iconGlyphs[c], cx, y, icon.h, false, textCol); y += icon.h + WX_ROWGAP; }
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

  // Resolve per-block fonts (indices clamped defensively against the tables).
  uint8_t tSz = w.tempSize   < CLK_NUM_FONT_COUNT  ? w.tempSize   : DEFAULT_WX_TEMPSIZE;
  uint8_t cSz = w.condSize   < CLK_PROP_FONT_COUNT ? w.condSize   : DEFAULT_WX_CONDSIZE;
  uint8_t dSz = w.detailSize < CLK_PROP_FONT_COUNT ? w.detailSize : DEFAULT_WX_DETAILSIZE;
  uint8_t hSz = w.hourlySize < CLK_PROP_FONT_COUNT ? w.hourlySize : DEFAULT_WX_HOURLYSIZE;
  uint8_t ySz = w.dailySize  < CLK_PROP_FONT_COUNT ? w.dailySize  : DEFAULT_WX_DAILYSIZE;
  const uint8_t* tempFont   = CLK_NUM_FONTS[tSz];
  const uint8_t* condFont   = CLK_PROP_FONTS[cSz];
  const uint8_t* detailFont = CLK_PROP_FONTS[dSz];
  const uint8_t* hourlyFont = CLK_PROP_FONTS[hSz];
  const uint8_t* dailyFont  = CLK_PROP_FONTS[ySz];

  // ---- build primary / condition / detail strings ----
  char tempStr[12];
  if (d.valid) snprintf(tempStr, sizeof(tempStr), "%d", (int)lroundf(d.temp));
  else         strlcpy(tempStr, "--", sizeof(tempStr));
  const char* condStr = d.valid ? weatherCodeDE(d.code) : "";

  // Detail line: join the enabled items with two spaces. The proportional fonts
  // are ASCII-only (no '°'), so temperatures here are printed as plain numbers.
  char detailStr[64]; detailStr[0] = 0;
  if (d.valid) {
    char part[24];
    #define WX_APPEND(fmt, ...) do { \
      snprintf(part, sizeof(part), fmt, __VA_ARGS__); \
      if (detailStr[0]) strlcat(detailStr, "  ", sizeof(detailStr)); \
      strlcat(detailStr, part, sizeof(detailStr)); \
    } while (0)
    if (w.showFeels) WX_APPEND("Gefuehlt %d", (int)lroundf(d.feelsLike));
    if (w.showHum)   WX_APPEND("Luft %d%%", d.humidity);
    if (w.showWind)  WX_APPEND("Wind %d", (int)lroundf(d.wind));
    if (w.showPrecip) {
      if (w.precipPct) WX_APPEND("Regen %d%%", d.nHours ? d.hours[0].pop : 0);
      else             WX_APPEND("Regen %.1f mm", d.precip);
    }
    #undef WX_APPEND
  }

  // ---- which blocks are present this frame ----
  bool hasPrimary = w.showTemp || w.showBigIcon;
  bool hasCond    = w.showCond && condStr[0];
  bool hasDetail  = detailStr[0];
  bool hasHourly  = w.showHourly && d.nHours > 0;
  bool hasDaily   = w.showDaily  && d.nDays  > 0;
  bool hasTrend   = w.showTrend  && d.nHours >= 2;

  // ---- measure block heights ----
  int hPrimary = 0, hCond = 0, hDetail = 0, hHourly = 0, hDaily = 0;
  int hTrend = hasTrend ? WX_TREND_H : 0;

  if (hasPrimary) {
    int th = 0;
    if (w.showTemp) { gfx->setFont(tempFont); th = fontHeight(gfx, "-8"); }
    int ih = (w.showBigIcon && d.valid) ? WX_ICON_BIG : 0;
    hPrimary = th > ih ? th : ih;
    if (hPrimary == 0) hasPrimary = false;   // temp off + no data yet -> nothing to show
  }
  if (hasCond)   { gfx->setFont(condFont);   hCond   = fontHeight(gfx, "Mg"); }
  if (hasDetail) { gfx->setFont(detailFont); hDetail = fontHeight(gfx, "0g%"); }

  // Hourly / daily strip sub-row heights.
  int hrRow = 0, hrH = 0, hrI = 0, hrT = 0, hrP = 0;
  int hourlyCols = 0;
  if (hasHourly) {
    gfx->setFont(hourlyFont); hrRow = fontHeight(gfx, "88h");
    hrH = w.hrHour ? hrRow : 0;
    hrI = w.hrIcon ? WX_ICON_MINI : 0;
    hrT = w.hrTemp ? hrRow : 0;
    hrP = w.hrPop  ? hrRow : 0;
    int step = w.hourlyStep < WX_HOURLY_STEP_MIN ? WX_HOURLY_STEP_MIN : w.hourlyStep;
    int want = w.hourlyCount < WX_HOURLY_COUNT_MIN ? WX_HOURLY_COUNT_MIN : w.hourlyCount;
    for (int c = 0; c < want && (int)(c * step) < d.nHours; c++) hourlyCols++;
    if (hourlyCols < 1) { hasHourly = false; }
    else {
      int rows = (hrH?1:0) + (hrI?1:0) + (hrT?1:0) + (hrP?1:0);
      hHourly = hrH + hrI + hrT + hrP + (rows > 1 ? (rows - 1) * WX_ROWGAP : 0);
      if (hHourly == 0) hasHourly = false;
    }
  }
  int dyRow = 0, dyH = 0, dyI = 0, dyT = 0, dyP = 0;
  int dailyCols = 0;
  if (hasDaily) {
    gfx->setFont(dailyFont); dyRow = fontHeight(gfx, "88/8");
    dyH = w.dyDay   ? dyRow : 0;
    dyI = w.dyIcon  ? WX_ICON_MINI : 0;
    dyT = w.dyTemps ? dyRow : 0;
    dyP = w.dyPop   ? dyRow : 0;
    int want = w.dailyCount < WX_DAILY_COUNT_MIN ? WX_DAILY_COUNT_MIN : w.dailyCount;
    dailyCols = want < d.nDays ? want : d.nDays;
    if (dailyCols < 1) { hasDaily = false; }
    else {
      int rows = (dyH?1:0) + (dyI?1:0) + (dyT?1:0) + (dyP?1:0);
      hDaily = dyH + dyI + dyT + dyP + (rows > 1 ? (rows - 1) * WX_ROWGAP : 0);
      if (hDaily == 0) hasDaily = false;
    }
  }

  // ---- centre the stack; tighten the gap if an extreme combo overflows ----
  bool present[6] = { hasPrimary, hasCond, hasDetail, hasHourly, hasDaily, hasTrend };
  int  heights[6] = { hPrimary, hCond, hDetail, hHourly, hDaily, hTrend };
  int gap = WX_GAP, top = 0;
  for (;;) {
    int H = 0; bool first = true;
    for (int i = 0; i < 6; i++) if (present[i]) { if (!first) H += gap; H += heights[i]; first = false; }
    top = (TFT_HEIGHT - H) / 2;
    if (top >= 0 || gap <= 1) break;
    gap -= 1;
  }
  if (top < 0) top = 0;

  // ---- draw ----
  gfx->fillScreen(C_BLACK);
  int y = top;
  bool prev = false;

  if (hasPrimary) {
    char glyph = wmoIcon(d.code, d.isDay);
    if (d.valid && w.showTemp) {
      drawPrimary(gfx, tempStr, w.showBigIcon, glyph, y, hPrimary,
                  tempFont, wxColor(w.tempColor), wxColor(w.bigIconColor));
    } else if (d.valid && w.showBigIcon) {
      drawWeatherIcon(gfx, glyph, TFT_WIDTH / 2, y, hPrimary, true, wxColor(w.bigIconColor));
    } else if (w.showTemp) {
      gfx->setFont(tempFont);
      drawFontCentered(gfx, tempStr, y, hPrimary, wxColor(w.tempColor));
    }
    y += hPrimary; prev = true;
  }
  if (hasCond) {
    if (prev) y += gap;
    gfx->setFont(condFont);
    drawFontCentered(gfx, condStr, y, hCond, wxColor(w.condColor));
    y += hCond; prev = true;
  }
  if (hasDetail) {
    if (prev) y += gap;
    gfx->setFont(detailFont);
    drawFontCentered(gfx, detailStr, y, hDetail, wxColor(w.detailColor));
    y += hDetail; prev = true;
  }
  if (hasHourly) {
    if (prev) y += gap;
    int step = w.hourlyStep < WX_HOURLY_STEP_MIN ? WX_HOURLY_STEP_MIN : w.hourlyStep;
    const char* topS[WX_HOURLY_COUNT_MAX];
    const char* midS[WX_HOURLY_COUNT_MAX];
    const char* botS[WX_HOURLY_COUNT_MAX];
    char iconG[WX_HOURLY_COUNT_MAX];
    static char buf[WX_HOURLY_COUNT_MAX][3][8];   // hour / temp / pop text
    for (int c = 0; c < hourlyCols; c++) {
      const WxHour& hh = d.hours[c * step];
      snprintf(buf[c][0], 8, "%dh", hh.hour);
      snprintf(buf[c][1], 8, "%d", (int)lroundf(hh.temp));
      snprintf(buf[c][2], 8, "%d%%", hh.pop);
      topS[c] = buf[c][0]; midS[c] = buf[c][1]; botS[c] = buf[c][2];
      iconG[c] = wmoIcon(hh.code, true);
    }
    drawStrip(gfx, hourlyCols, y, hourlyFont, wxColor(w.hourlyColor), wxColor(w.hourlyPop),
              topS, iconG, midS, botS,
              { (bool)w.hrHour, hrH }, { (bool)w.hrIcon, hrI },
              { (bool)w.hrTemp, hrT }, { (bool)w.hrPop, hrP });
    y += hHourly; prev = true;
  }
  if (hasDaily) {
    if (prev) y += gap;
    int want = w.dailyCount < WX_DAILY_COUNT_MIN ? WX_DAILY_COUNT_MIN : w.dailyCount;
    (void)want;
    const char* topS[WX_DAILY_COUNT_MAX];
    const char* midS[WX_DAILY_COUNT_MAX];
    const char* botS[WX_DAILY_COUNT_MAX];
    char iconG[WX_DAILY_COUNT_MAX];
    static char dbuf[WX_DAILY_COUNT_MAX][3][10];  // day / temps / pop text
    for (int c = 0; c < dailyCols; c++) {
      const WxDay& dd = d.days[c];
      strlcpy(dbuf[c][0], kWeekdayShortDE[dd.wday < 7 ? dd.wday : 0], 10);
      snprintf(dbuf[c][1], 10, "%d/%d", (int)lroundf(dd.tmax), (int)lroundf(dd.tmin));
      snprintf(dbuf[c][2], 10, "%d%%", dd.popMax);
      topS[c] = dbuf[c][0]; midS[c] = dbuf[c][1]; botS[c] = dbuf[c][2];
      iconG[c] = wmoIcon(dd.code, true);
    }
    drawStrip(gfx, dailyCols, y, dailyFont, wxColor(w.dailyColor), wxColor(w.dailyPop),
              topS, iconG, midS, botS,
              { (bool)w.dyDay, dyH }, { (bool)w.dyIcon, dyI },
              { (bool)w.dyTemps, dyT }, { (bool)w.dyPop, dyP });
    y += hDaily; prev = true;
  }
  if (hasTrend) {
    if (prev) y += gap;
    drawTrend(gfx, d, WX_TREND_MARGIN, y, TFT_WIDTH - 2 * WX_TREND_MARGIN, hTrend,
              wxColor(w.trendColor), w.trendLabels, CLK_PROP_FONTS[0]);
    y += hTrend;
  }

  // Nothing enabled (or no data yet with everything off) -> a neutral hint.
  if (!hasPrimary && !hasCond && !hasDetail && !hasHourly && !hasDaily && !hasTrend)
    gfxDrawCentered(d.valid ? "no blocks" : "--", 116, 2, C_GRAY);

  // Restore the built-in font + plain ASCII printing for shared status screens.
  gfx->setFont((const GFXfont*)nullptr);
  gfx->setUTF8Print(false);
  gfx->setTextSize(1);
}
