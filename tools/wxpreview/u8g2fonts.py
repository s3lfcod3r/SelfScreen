"""Parse vendored U8g2 BDF-compiled font headers to recover per-font metrics.

We only need two header fields to reproduce the firmware's measure-then-stack
layout faithfully:
  offset 13 = ascent_A   (cap height; for the logisoso _tn number fonts this is
                          the digit height)
  offset 14 = descent_g  (signed; how far 'g' etc. drop below the baseline)

WeatherMode.cpp measures block heights with gfx->getTextBounds(str), whose ink
height equals ascent_A for a string with no descender (digits, "88h", "88/8",
"-8") and ascent_A - descent_g for a string that has one ("Mg", "0g%").
"""
import re, os

def _decode_c_string(chunks):
    # chunks: list of the raw contents between double quotes (C string literal
    # pieces, implicitly concatenated). Decode C octal / simple escapes to bytes.
    s = "".join(chunks)
    out = bytearray()
    i = 0
    while i < len(s):
        c = s[i]
        if c == "\\":
            i += 1
            n = s[i]
            if n in "01234567":
                oct_digits = n
                i += 1
                # up to 3 octal digits total
                while i < len(s) and s[i] in "01234567" and len(oct_digits) < 3:
                    oct_digits += s[i]; i += 1
                out.append(int(oct_digits, 8) & 0xFF); continue
            simple = {"n":10,"t":9,"r":13,'"':34,"\\":92,"'":39,"a":7,"b":8,"f":12,"v":11,"0":0}
            out.append(simple.get(n, ord(n))); i += 1; continue
        else:
            out.append(ord(c)); i += 1
    return bytes(out)

def parse_font_headers(path):
    """Return {font_name: {'ascent':int,'descent':int,'maxh':int}} from a .h file."""
    txt = open(path, "r", encoding="utf-8", errors="replace").read()
    fonts = {}
    # Match: const uint8_t NAME[...] U8G2_FONT_SECTION("...") = "..." "..." ... ;
    pat = re.compile(
        r'const\s+uint8_t\s+(\w+)\s*\[[^\]]*\]\s*U8G2_FONT_SECTION\([^)]*\)\s*=\s*(.*?);',
        re.DOTALL)
    for m in pat.finditer(txt):
        name = m.group(1)
        body = m.group(2)
        chunks = re.findall(r'"((?:[^"\\]|\\.)*)"', body)
        data = _decode_c_string(chunks)
        if len(data) < 17:
            continue
        def sb(x):  # signed byte
            return x - 256 if x >= 128 else x
        fonts[name] = {
            "ascent": sb(data[13]),
            "descent": sb(data[14]),
            "maxh": data[10],
        }
    return fonts

_HERE = os.path.dirname(os.path.abspath(__file__))

def _find_repo():
    # Prefer the repo this script lives in (tools/wxpreview/ -> repo root two up),
    # else fall back to the known dev checkout path.
    cand = os.path.abspath(os.path.join(_HERE, "..", ".."))
    if os.path.exists(os.path.join(cand, "src", "features", "clock", "u8g2_clock_fonts.h")):
        return cand
    return r"F:\09_Cloude\Github SelfCoder\selfscreen"

_REPO = _find_repo()

def load_all():
    f = {}
    f.update(parse_font_headers(os.path.join(_REPO, "src", "features", "clock", "u8g2_clock_fonts.h")))
    return f

# Ordered tables mirroring config.h CLK_NUM_FONTS / CLK_PROP_FONTS.
NUM_FONT_NAMES = [
    "u8g2_font_logisoso16_tn", "u8g2_font_logisoso22_tn", "u8g2_font_logisoso32_tn",
    "u8g2_font_logisoso42_tn", "u8g2_font_logisoso50_tn", "u8g2_font_logisoso62_tn",
    "u8g2_font_logisoso78_tn",
]
PROP_FONT_NAMES = [
    "u8g2_font_helvB08_tr", "u8g2_font_helvB10_tr", "u8g2_font_helvB12_tr",
    "u8g2_font_helvB14_tr", "u8g2_font_helvB18_tr", "u8g2_font_helvB24_tr",
]

if __name__ == "__main__":
    fonts = load_all()
    print("Parsed", len(fonts), "fonts")
    for n in NUM_FONT_NAMES + PROP_FONT_NAMES:
        if n in fonts:
            m = fonts[n]
            print(f"{n:32s} ascent={m['ascent']:3d} descent={m['descent']:3d} maxh={m['maxh']:3d}")
        else:
            print(f"{n:32s} MISSING")
