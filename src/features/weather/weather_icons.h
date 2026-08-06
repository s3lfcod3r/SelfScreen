// weather_icons.h — vendored U8g2 weather-icon fonts for the SelfScreen weather face.
//
// Extracted verbatim from the U8g2 project's csrc/u8g2_fonts.c
// (github.com/olikraus/u8g2, BSD-2-Clause), same approach as the clock's
// u8g2_clock_fonts.h. Each array keeps its U8G2_FONT_SECTION(name) attribute so
// it lands in FLASH (irom on ESP8266, drom on ESP32), read via pgm_read_byte by
// Arduino_GFX's U8g2 font renderer — never in RAM.
//
// Two sizes of open_iconic_weather:
//   u8g2_font_open_iconic_weather_4x_t  ~32 px  — the big current-conditions icon
//   u8g2_font_open_iconic_weather_2x_t  ~16 px  — mini icons in the hourly/daily strips
//
// The font carries exactly SIX glyphs (verified by decoding the bitmaps), at
// encodings 0x40..0x45:
//   0x40 cloud (overcast)   0x41 sun-behind-cloud (partly cloudy)
//   0x42 crescent moon      0x43 rain cloud
//   0x44 raindrop           0x45 sun (clear)
// wmoIcon() maps a WMO weather_code (+ is_day) onto the closest available glyph.
#pragma once
#include <Arduino.h>       // pgm_read_byte / PROGMEM
#include "config.h"

class Arduino_GFX;         // fwd-decl; the .cpp pulls in the full renderer

// See u8g2_clock_fonts.h for why this macro must map to PROGMEM on the ESP8266.
#ifndef U8G2_FONT_SECTION
  #if defined(ESP8266)
    #define U8G2_FONT_SECTION(name) PROGMEM
  #else
    #define U8G2_FONT_SECTION(name)
  #endif
#endif

// ==== Vendored open_iconic_weather icon fonts ====
const uint8_t u8g2_font_open_iconic_weather_2x_t[189] U8G2_FONT_SECTION("u8g2_font_open_iconic_weather_2x_t") =
  "\6\0\4\4\5\5\2\3\6\20\20\0\0\20\0\20\0\0\24\0\0\0\240@\24\220i\370\210\42D\247"
  "\210M\223\27\17>\30\361\244\5\0A\34\20Jx\210\42,Z\226\14\251!h\304\240S\304\246\311\213"
  "\7\37\214x\322\2\0B\30\17J\230B\13\36K\251I\223\7\321\245[\22\302\11#eH\205\1C"
  "!\20J\370\210\42D\247\210M\223\27\17\14-\63&j\230\260\60B$\23\42\231\20\311\204H\4\0"
  "D\26\256]\330\4KK\312\5\33U\350\14\232\33\62L\220\260h\0E\37\20J\370\204\213\24(H"
  "\240@\262&\21\11AB\4\211 \224f\11J$P\244pq\0\0\0\0\4\377\377\0";

const uint8_t u8g2_font_open_iconic_weather_4x_t[373] U8G2_FONT_SECTION("u8g2_font_open_iconic_weather_4x_t") =
  "\6\0\5\5\6\6\2\4\7  \0\0 \0 \0\0'\0\0\1X@' &\203\237\201\25\352"
  "D\350EOjS\243\322u\62\243\225\255p\303\23_\370\201\37\30\302\27\274\321\221\355\134\4\0A:"
  " (\202\217\201\25\352D\10\62s\251\211M.r\214u\30%\31\305\71E\201LA\242\23\34\251M"
  "\215J\327\311\214V\266\302\15O|\341\7~`\10_\360FG\266s\21\0B\62\337g\202U\300\205"
  "M\350\42\33\370t\21\233X\245*t\241\314df#\235\370@\350EOj\25\12\202'\270\301\21\315"
  "`\310R\24\223\236C\221\5\0CH (\202\237\201\25\352D\350EOjS\243\322u\62\243\225\255"
  "p\303\23_\370@C\64t\251& \252\11\202\22\322\321\10U\64b\15\216\250B!\212\320\10Bb"
  "\4!\61\202\220\30AH\214 $F\20R\23\214(\1D\65^w\202\235\300E\233\320\244l`\323"
  "E\226\27\270\241\25\14QKj\320s\42#\231\350@\7\32\302x\6\61\134a\12G \242\15l`"
  "\202\23X\0\3\3\0EF (\202\237\320\11Nt\201\12RH\4$\20\1\211$HA\32X\251"
  "\314t\244\23\241$\24\250\10\201 \20A\10D\210 \24\250\10\11\212\216t&S\25lHQ\42 "
  "\201\10H$A\12T\350\4'\272\360\0\0\0\0\4\377\377\0";

// WMO weather_code (+ is_day) -> the closest open_iconic_weather glyph char.
// With only six glyphs some categories share an icon (documented above): snow has
// no snowflake glyph so it borrows the raindrop; thunderstorm uses the rain cloud.
static inline char wmoIcon(int code, bool isDay) {
  if (code < 0) return WX_ICON_CLOUD;
  switch (code) {
    case 0:
    case 1:  return isDay ? WX_ICON_SUN : WX_ICON_MOON;   // clear / mainly clear
    case 2:  return isDay ? WX_ICON_SUNCLOUD : WX_ICON_MOON; // partly cloudy
    case 3:  return WX_ICON_CLOUD;                         // overcast
    case 45:
    case 48: return WX_ICON_CLOUD;                         // fog
    case 51: case 53: case 55:                             // drizzle
    case 56: case 57:                                      // freezing drizzle
    case 61: case 63: case 65:                             // rain
    case 66: case 67:                                      // freezing rain
    case 80: case 81: case 82: return WX_ICON_RAIN;        // rain showers
    case 71: case 73: case 75: case 77:                    // snow
    case 85: case 86: return WX_ICON_DROP;                 // snow showers (no flake glyph)
    case 95: case 96: case 99: return WX_ICON_RAIN;        // thunderstorm
    default: return WX_ICON_CLOUD;
  }
}

// Draw a weather icon centred horizontally on `cx` and vertically inside
// [topY, topY+boxH), using the 4x (big) or 2x (mini) icon font. Sets the icon
// font, prints the glyph, then restores the caller's font/flags. Implemented in
// WeatherMode.cpp (needs the full Arduino_GFX definition).
void drawWeatherIcon(Arduino_GFX* gfx, char glyph, int cx, int topY, int boxH,
                     bool big, uint16_t color);
