"""Host-side preview renderer for the SelfScreen weather face (240x240).

Reproduces WeatherMode.cpp's measure-then-stack-centred block layout so the
on-device fit can be judged visually without flashing. It is NOT a pixel-exact
u8g2 rasteriser: text is drawn with a bold TrueType face sized so each element's
ink height matches the u8g2 font height parsed from the vendored headers, and
the geometry (block heights, gaps, stacking, primary composition) is computed
with the SAME formulas as the firmware. Weather icons are drawn as simple
coloured symbols at ~16/32 px.

Two layout algorithms are modelled:
  algo="before" -> gap-tighten-then-clip (current firmware), single-line detail,
                   fixed one-colour icons, precip in mm.
  algo="after"  -> fit-by-shrinking-font-sizes, 2-line wrapped compact detail,
                   semantic icon colours, precip as % (default).
"""
import os
from PIL import Image, ImageDraw, ImageFont
from u8g2fonts import load_all, NUM_FONT_NAMES, PROP_FONT_NAMES

W = H = 240
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
os.makedirs(OUT, exist_ok=True)

# ---- layout constants (mirror WeatherMode.cpp) ----
WX_GAP = 6
WX_ROWGAP = 1
WX_ICON_BIG = 32
WX_ICON_MINI = 16
WX_ICON_GAP = 8
WX_TREND_H = 40
WX_TREND_MARGIN = 20
BIG_ICON_GLYPH_W = 30   # approx fontWidth() of the 4x icon glyph
DETAIL_MARGIN = 6       # side inset used by the 2-line wrap
DETAIL_LINEGAP = 2

FONTS = load_all()
def num_ascent(i):  return FONTS[NUM_FONT_NAMES[i]]["ascent"]
def num_desc(i):    return FONTS[NUM_FONT_NAMES[i]]["descent"]
def prop_ascent(i): return FONTS[PROP_FONT_NAMES[i]]["ascent"]
def prop_desc(i):   return FONTS[PROP_FONT_NAMES[i]]["descent"]

# ink height of a measured string, matching gfx->getTextBounds()
def num_ink(i):         return num_ascent(i)                       # "-8" (no descender)
def prop_ink_desc(i):   return prop_ascent(i) - prop_desc(i)       # "Mg" / "0g%"
def prop_ink_cap(i):    return prop_ascent(i)                      # "88h" / "88/8"

# ---- colour palette (Gfx.h RGB565 -> RGB888) ----
def c565(v):
    r = (v >> 11) & 0x1F; g = (v >> 5) & 0x3F; b = v & 0x1F
    return (round(r * 255 / 31), round(g * 255 / 63), round(b * 255 / 31))
C_WHITE=c565(0xFFFF); C_GREEN=c565(0x07E0); C_RED=c565(0xF800); C_GRAY=c565(0x8410)
C_YELLOW=c565(0xFFE0); C_BLUE=c565(0x041F); C_TEAL=c565(0x2EB7)
C_SELFBLUE=c565(0x6DBF); C_SELFBLUE_DK=c565(0x4C9C); C_BLACK=(0,0,0)
C_GRAYBLUE = C_SELFBLUE_DK  # semantic rain colour
PRESET = [C_WHITE,C_TEAL,C_GREEN,C_YELLOW,C_RED,C_BLUE,C_GRAY,C_SELFBLUE,C_SELFBLUE_DK]
def wxColor(i): return PRESET[i] if 0 <= i < len(PRESET) else C_WHITE

# ---- TrueType face sized to a target u8g2 cap height ----
_TTF = r"C:\Windows\Fonts\arialbd.ttf"
_font_cache = {}
def ttf_for_cap(cap_px):
    cap_px = max(6, int(round(cap_px)))
    if cap_px in _font_cache: return _font_cache[cap_px]
    # binary search Arial-Bold size so the ink height of "8" ~= cap_px
    lo, hi = 6, 200
    best = ImageFont.truetype(_TTF, cap_px)
    for _ in range(20):
        mid = (lo + hi) // 2
        f = ImageFont.truetype(_TTF, mid)
        b = f.getbbox("8")
        ink = b[3] - b[1]
        if ink < cap_px: lo = mid + 1
        else: hi = mid - 1
        best = f
        if abs(ink - cap_px) <= 0: break
    _font_cache[cap_px] = best
    return best

def text_size(draw, s, font):
    b = draw.textbbox((0, 0), s, font=font)
    return b[2]-b[0], b[3]-b[1]

def draw_centered_at(draw, s, cx, bandY, bandH, font, color):
    b = draw.textbbox((0, 0), s, font=font)
    w = b[2]-b[0]; h = b[3]-b[1]
    px = cx - w/2 - b[0]
    py = bandY + (bandH - h)/2 - b[1]
    draw.text((px, py), s, font=font, fill=color)

def draw_centered(draw, s, bandY, bandH, font, color):
    draw_centered_at(draw, s, W/2, bandY, bandH, font, color)

# ---- weather icons drawn as simple coloured symbols ----
WX_ICON_CLOUD=0x40; WX_ICON_SUNCLOUD=0x41; WX_ICON_MOON=0x42
WX_ICON_RAIN=0x43; WX_ICON_DROP=0x44; WX_ICON_SUN=0x45

def wmo_icon(code, isDay):
    if code < 0: return WX_ICON_CLOUD
    if code in (0,1): return WX_ICON_SUN if isDay else WX_ICON_MOON
    if code == 2:     return WX_ICON_SUNCLOUD if isDay else WX_ICON_MOON
    if code in (3,45,48): return WX_ICON_CLOUD
    if code in (51,53,55,56,57,61,63,65,66,67,80,81,82): return WX_ICON_RAIN
    if code in (71,73,75,77,85,86): return WX_ICON_DROP
    if code in (95,96,99): return WX_ICON_RAIN
    return WX_ICON_CLOUD

def icon_semantic_color(code, isDay):
    if code < 0: return C_GRAY
    if code in (0,1,2): return C_YELLOW if isDay else C_WHITE
    if code in (3,45,48): return C_GRAY
    if code in (71,73,75,77,85,86): return C_WHITE
    if code in (51,53,55,56,57,61,63,65,66,67,80,81,82,95,96,99): return C_GRAYBLUE
    return C_GRAY

def draw_icon(draw, glyph, cx, topY, boxH, color):
    s = min(boxH, 40)
    cy = topY + boxH/2
    r = s*0.30
    def blob(bx, by, br, col):
        draw.ellipse([bx-br, by-br*0.7, bx+br, by+br*0.7], fill=col)
        draw.ellipse([bx-br*0.5, by-br*1.1, bx+br*0.4, by+br*0.3], fill=col)
        draw.ellipse([bx-br*1.0, by-br*0.4, bx-br*0.2, by+br*0.4], fill=col)
        draw.ellipse([bx+br*0.2, by-br*0.5, bx+br*1.0, by+br*0.4], fill=col)
    if glyph == WX_ICON_SUN:
        for k in range(8):
            import math
            a = k*math.pi/4
            draw.line([cx+math.cos(a)*r*1.1, cy+math.sin(a)*r*1.1,
                       cx+math.cos(a)*r*1.7, cy+math.sin(a)*r*1.7], fill=color, width=max(1,int(s*0.06)))
        draw.ellipse([cx-r, cy-r, cx+r, cy+r], fill=color)
    elif glyph == WX_ICON_MOON:
        col = color
        draw.ellipse([cx-r, cy-r, cx+r, cy+r], fill=col)
        draw.ellipse([cx-r*0.2, cy-r*1.1, cx+r*1.4, cy+r*0.9], fill=C_BLACK)
    elif glyph == WX_ICON_SUNCLOUD:
        draw.ellipse([cx-r*1.4, cy-r*1.4, cx-r*0.2, cy-r*0.2], fill=C_YELLOW)  # sun peek
        blob(cx+r*0.2, cy+r*0.2, r*1.1, color)
    elif glyph == WX_ICON_CLOUD:
        blob(cx, cy, r*1.2, color)
    elif glyph == WX_ICON_RAIN:
        blob(cx, cy-r*0.3, r*1.1, color)
        for dx in (-r*0.6, 0, r*0.6):
            draw.line([cx+dx, cy+r*0.7, cx+dx-r*0.2, cy+r*1.3], fill=C_BLUE, width=max(1,int(s*0.06)))
    elif glyph == WX_ICON_DROP:
        draw.polygon([cx, cy-r, cx-r*0.8, cy+r*0.6, cx+r*0.8, cy+r*0.6], fill=color)
        draw.ellipse([cx-r*0.8, cy-r*0.1, cx+r*0.8, cy+r], fill=color)
    else:
        blob(cx, cy, r*1.1, color)

# ---------------------------------------------------------------------------
# Sample weather data
# ---------------------------------------------------------------------------
SAMPLE = {
    "valid": True, "isDay": True,
    "temp": 22.0, "feelsLike": 20.0, "precip": 0.0, "wind": 12.0,
    "humidity": 61, "code": 3,
    "hours": [  # (hour, temp, pop, code)
        (12,22,10,3),(13,23,10,2),(14,24,20,2),(15,24,30,3),(16,23,40,61),(17,22,40,61),
        (18,21,30,3),(19,20,20,3),(20,20,10,2),(21,20,10,0),(22,21,0,0),(23,20,0,0)],
    "days": [  # (wday, tmax, tmin, popMax, code) — 7 days (WX_DAILY_POINTS)
        (3,24,16,40,3),(4,25,15,20,2),(5,21,14,60,61),(6,23,15,10,0),
        (0,22,14,30,3),(1,26,17,10,45),(2,24,16,50,80)],
}
WD_DE = ["So","Mo","Di","Mi","Do","Fr","Sa"]
COND_DE = {0:"Klar",1:"Leicht bewoelkt",2:"Leicht bewoelkt",3:"Bewoelkt",45:"Nebel",48:"Nebel",
           61:"Regen",63:"Regen",80:"Schauer",95:"Gewitter"}

# ---------------------------------------------------------------------------
# Default weather settings (mirror config.h DEFAULT_WX_* AFTER the fix)
# ---------------------------------------------------------------------------
def base_settings():
    return dict(
        unitF=False, windUnit=0, precipMode=1, iconColorMode=0, refreshSec=600,
        showTemp=True, showBigIcon=True, showCond=True,
        showFeels=False, showHum=False, showWind=False, showPrecip=True,
        showHourly=True, showDaily=False, showTrend=False,
        hourlyCount=4, hourlyStep=1, hrHour=True, hrIcon=True, hrTemp=True, hrPop=True,
        dailyCount=3, dyDay=True, dyIcon=True, dyTemps=True, dyPop=True, trendLabels=False,
        tempSize=5, condSize=3, detailSize=1, hourlySize=0, dailySize=0,
        tempColor=0, bigIconColor=3, condColor=7, detailColor=6,
        hourlyColor=0, hourlyPop=1, dailyColor=0, dailyPop=1, trendColor=1,
    )

# ---- detail tokens ----
def detail_tokens(w, d):
    toks = []
    if w["showFeels"]: toks.append("Gefuehlt %d" % round(d["feelsLike"]))
    if w["showHum"]:   toks.append("Luft %d%%" % d["humidity"])
    if w["showWind"]:  toks.append("Wind %d" % round(d["wind"]))
    if w["showPrecip"]:
        pop = d["hours"][0][2] if d["hours"] else 0
        m = w["precipMode"]
        if m == 1:   toks.append("Regen %d%%" % pop)
        elif m == 2: toks.append("Regen %d%% %.1fmm" % (pop, d["precip"]))
        else:        toks.append("Regen %.1f mm" % d["precip"])
    return toks

def detail_single(w, d):  # "before": one joined line (may overflow)
    return "  ".join(detail_tokens(w, d))

def detail_wrapped(draw, w, d, dSz):  # "after": greedy 2-line wrap
    toks = detail_tokens(w, d)
    if not toks: return []
    font = ttf_for_cap(prop_ascent(dSz))
    usable = W - 2*DETAIL_MARGIN
    lines = []
    cur = ""
    for tk in toks:
        cand = tk if not cur else cur + "  " + tk
        wpx, _ = text_size(draw, cand, font)
        if wpx <= usable or not cur:
            cur = cand
        else:
            lines.append(cur); cur = tk
            if len(lines) == 1:  # only allow 2 lines; rest gets appended below
                pass
    if cur: lines.append(cur)
    if len(lines) > 2:  # collapse overflow into 2 lines
        lines = [lines[0], "  ".join(lines[1:])]
    return lines

# ---------------------------------------------------------------------------
# Measure block heights for a given set of size indices
# ---------------------------------------------------------------------------
def measure(draw, w, d, sizes, detail_lines):
    tSz,cSz,dSz,hSz,ySz = sizes
    condStr = COND_DE.get(d["code"], "") if d["valid"] else ""
    present = [False]*6
    heights = [0]*6

    # A primary
    hasPrimary = w["showTemp"] or w["showBigIcon"]
    if hasPrimary:
        th = num_ink(tSz) if w["showTemp"] else 0
        ih = WX_ICON_BIG if (w["showBigIcon"] and d["valid"]) else 0
        hP = max(th, ih)
        if hP == 0: hasPrimary = False
        else: present[0] = True; heights[0] = hP
    # B condition
    if w["showCond"] and condStr:
        present[1] = True; heights[1] = prop_ink_desc(cSz)
    # C detail
    if detail_lines:
        n = len(detail_lines)
        lineH = prop_ink_desc(dSz)
        present[2] = True; heights[2] = n*lineH + (n-1)*DETAIL_LINEGAP
    # D hourly
    hourly_cols = 0
    hr = (0,0,0,0)
    if w["showHourly"] and d["hours"]:
        hrRow = prop_ink_cap(hSz)
        hrH = hrRow if w["hrHour"] else 0
        hrI = WX_ICON_MINI if w["hrIcon"] else 0
        hrT = hrRow if w["hrTemp"] else 0
        hrP = hrRow if w["hrPop"] else 0
        hr = (hrH,hrI,hrT,hrP)
        step = max(1, w["hourlyStep"]); want = max(3, w["hourlyCount"])
        c = 0
        while c < want and c*step < len(d["hours"]): hourly_cols += 1; c += 1
        rows = sum(1 for x in hr if x)
        hh = sum(hr) + (rows-1)*WX_ROWGAP if rows else 0
        if hourly_cols >= 1 and hh > 0:
            present[3] = True; heights[3] = hh
    # E daily
    daily_cols = 0
    dy = (0,0,0,0)
    if w["showDaily"] and d["days"]:
        dyRow = prop_ink_cap(ySz)
        dyH = dyRow if w["dyDay"] else 0
        dyI = WX_ICON_MINI if w["dyIcon"] else 0
        dyT = dyRow if w["dyTemps"] else 0
        dyP = dyRow if w["dyPop"] else 0
        dy = (dyH,dyI,dyT,dyP)
        want = max(2, w["dailyCount"]); daily_cols = min(want, len(d["days"]))
        rows = sum(1 for x in dy if x)
        hh = sum(dy) + (rows-1)*WX_ROWGAP if rows else 0
        if daily_cols >= 1 and hh > 0:
            present[4] = True; heights[4] = hh
    # F trend
    if w["showTrend"] and len(d["hours"]) >= 2:
        present[5] = True; heights[5] = WX_TREND_H
    return present, heights, condStr, hourly_cols, hr, daily_cols, dy

def stack_height(present, heights, gap):
    Htot = 0; first = True
    for i in range(6):
        if present[i]:
            if not first: Htot += gap
            Htot += heights[i]; first = False
    return Htot

# ---------------------------------------------------------------------------
# Full render
# ---------------------------------------------------------------------------
def render(w, d, algo):
    img = Image.new("RGB", (W, H), C_BLACK)
    draw = ImageDraw.Draw(img)

    # detail lines depend on algo
    if algo == "before":
        s = detail_single(w, d)
        detail_lines = [s] if s else []
    else:
        detail_lines = detail_wrapped(draw, w, d, w["detailSize"])

    sizes = [w["tempSize"], w["condSize"], w["detailSize"], w["hourlySize"], w["dailySize"]]
    shrunk = False

    if algo == "after":
        # fit-by-shrinking: step size indices down (largest blocks first) until it fits
        for _ in range(60):
            present, heights, *_ = measure(draw, w, d, sizes, detail_lines)
            if stack_height(present, heights, WX_GAP) <= H: break
            # candidate blocks that can still shrink: 0=temp(num),1=cond,2=detail,3=hourly,4=daily
            cand = []
            block_idx_map = {0:0,1:1,2:2,3:3,4:4}  # sizes index == present index here
            for bi in (0,1,2,3,4):
                if present[bi] and sizes[bi] > 0:
                    cand.append((heights[bi], bi))
            if not cand:
                break
            cand.sort(reverse=True)  # tallest block first
            bi = cand[0][1]
            sizes[bi] -= 1; shrunk = True
            if bi == 2:  # detail font changed -> re-wrap
                detail_lines = detail_wrapped(draw, w, d, sizes[2])

    present, heights, condStr, hcols, hr, dcols, dy = measure(draw, w, d, sizes, detail_lines)

    gap = WX_GAP
    while True:
        Htot = stack_height(present, heights, gap)
        top = (H - Htot) // 2
        if top >= 0 or gap <= 1: break
        gap -= 1
    if top < 0: top = 0

    tSz,cSz,dSz,hSz,ySz = sizes
    y = top; prev = False

    # A primary
    if present[0]:
        glyph = wmo_icon(d["code"], d["isDay"])
        hP = heights[0]
        if w["iconColorMode"] == 0:
            icol = icon_semantic_color(d["code"], d["isDay"])
        else:
            icol = wxColor(w["bigIconColor"])
        if algo == "before":
            icol = wxColor(w["bigIconColor"])  # old: fixed colour only
        if d["valid"] and w["showTemp"]:
            tempStr = "%d" % round(d["temp"])
            font = ttf_for_cap(num_ascent(tSz))
            nb = draw.textbbox((0,0), tempStr, font=font)
            numW = nb[2]-nb[0]
            r = max(3, num_ink(tSz)//10); ggap = r
            tempTotal = numW + ggap + 2*r
            iconW = (BIG_ICON_GLYPH_W + WX_ICON_GAP) if w["showBigIcon"] else 0
            total = iconW + tempTotal
            startX = (W - total)//2
            if w["showBigIcon"]:
                draw_icon(draw, glyph, startX + (iconW-WX_ICON_GAP)/2, y, hP, icol)
            inkTop = y + (hP - (nb[3]-nb[1]))/2
            numX = startX + iconW - nb[0]
            draw.text((numX, inkTop - nb[1]), tempStr, font=font, fill=wxColor(w["tempColor"]))
            ringCx = startX + iconW + nb[0] + numW + ggap + r
            ringCy = inkTop + r
            draw.ellipse([ringCx-r, ringCy-r, ringCx+r, ringCy+r], outline=wxColor(w["tempColor"]), width=max(1,r//2))
        elif d["valid"] and w["showBigIcon"]:
            draw_icon(draw, glyph, W/2, y, hP, icol)
        elif w["showTemp"]:
            tempStr = "%d" % round(d["temp"]) if d["valid"] else "--"
            draw_centered(draw, tempStr, y, hP, ttf_for_cap(num_ascent(tSz)), wxColor(w["tempColor"]))
        y += hP; prev = True
    # B condition
    if present[1]:
        if prev: y += gap
        draw_centered(draw, condStr, y, heights[1], ttf_for_cap(prop_ascent(cSz)), wxColor(w["condColor"]))
        y += heights[1]; prev = True
    # C detail
    if present[2]:
        if prev: y += gap
        font = ttf_for_cap(prop_ascent(dSz))
        lineH = prop_ink_desc(dSz)
        yy = y
        for ln in detail_lines:
            draw_centered(draw, ln, yy, lineH, font, wxColor(w["detailColor"]))
            yy += lineH + DETAIL_LINEGAP
        y += heights[2]; prev = True
    # D hourly
    if present[3]:
        if prev: y += gap
        draw_strip(draw, w, d, hcols, y, hSz, hr, "hourly", algo)
        y += heights[3]; prev = True
    # E daily
    if present[4]:
        if prev: y += gap
        draw_strip(draw, w, d, dcols, y, ySz, dy, "daily", algo)
        y += heights[4]; prev = True
    # F trend
    if present[5]:
        if prev: y += gap
        draw_trend(draw, d, WX_TREND_MARGIN, y, W-2*WX_TREND_MARGIN, WX_TREND_H, wxColor(w["trendColor"]), w["trendLabels"])
        y += WX_TREND_H

    if not any(present):
        draw_centered(draw, "no blocks" if d["valid"] else "--", 100, 32, ttf_for_cap(12), C_GRAY)

    # overflow markers (debug): red hairline where content ran off top/bottom
    return img, shrunk, sizes

def draw_strip(draw, w, d, cols, bandY, sz, rows, kind, algo):
    hrH,hrI,hrT,hrP = rows
    colW = W // cols
    font = ttf_for_cap(prop_ascent(sz))
    textCol = wxColor(w["hourlyColor"] if kind=="hourly" else w["dailyColor"])
    popCol  = wxColor(w["hourlyPop"] if kind=="hourly" else w["dailyPop"])
    step = max(1, w["hourlyStep"])
    for c in range(cols):
        cx = colW*c + colW//2
        yy = bandY
        if kind == "hourly":
            hr = d["hours"][c*step]; hour,temp,pop,code = hr
            top_s = "%dh" % hour; mid_s = "%d" % round(temp); bot_s = "%d%%" % pop
            on = (w["hrHour"], w["hrIcon"], w["hrTemp"], w["hrPop"])
        else:
            dd = d["days"][c]; wday,tmax,tmin,popMax,code = dd
            top_s = WD_DE[wday]; mid_s = "%d/%d" % (round(tmax),round(tmin)); bot_s = "%d%%" % popMax
            on = (w["dyDay"], w["dyIcon"], w["dyTemps"], w["dyPop"])
        if on[0]: draw_centered_at(draw, top_s, cx, yy, hrH, font, textCol); yy += hrH + WX_ROWGAP
        if on[1]:
            if w["iconColorMode"] == 0 and algo == "after":
                icol = icon_semantic_color(code, True)
            else:
                icol = textCol
            draw_icon(draw, wmo_icon(code, True), cx, yy, hrI, icol); yy += hrI + WX_ROWGAP
        if on[2]: draw_centered_at(draw, mid_s, cx, yy, hrT, font, textCol); yy += hrT + WX_ROWGAP
        if on[3]: draw_centered_at(draw, bot_s, cx, yy, hrP, font, popCol)

def draw_trend(draw, d, x, y, w, h, col, labels):
    hs = d["hours"]
    if len(hs) < 2: return
    temps = [p[1] for p in hs]
    mn, mx = min(temps), max(temps); span = max(0.5, mx-mn)
    n = len(hs); prev = None
    for i in range(n):
        px = x + (w-1)*i//(n-1)
        py = y + (h-1) - int((temps[i]-mn)/span*(h-1))
        if prev: draw.line([prev[0],prev[1],px,py], fill=col, width=1)
        draw.ellipse([px-1,py-1,px+1,py+1], fill=col)
        prev = (px,py)
    if labels:
        f = ttf_for_cap(prop_ascent(0))
        draw.text((2,y), "%d"%round(mx), font=f, fill=col)
        draw.text((2,y+h-10), "%d"%round(mn), font=f, fill=col)

# ---------------------------------------------------------------------------
# Full-screen pages (mirror WeatherMode.cpp renderTempTrend/RainTrend/Days7)
# ---------------------------------------------------------------------------
WX_PG_TITLE_Y = 6
WX_PG_MARGIN_B = 26

def _th(draw, s, font):
    b = draw.textbbox((0, 0), s, font=font); return b[3]-b[1]

def draw_page_title(draw, title, col):
    f = ttf_for_cap(prop_ascent(2))          # helvB12
    h = prop_ink_desc(2)
    draw_centered(draw, title, WX_PG_TITLE_Y, h, f, col)
    return WX_PG_TITLE_Y + h

def render_page_now(w, d):
    img, _shrunk, _sizes = render(w, d, "after")
    return img

def render_page_temp(w, d):
    img = Image.new("RGB", (W, H), C_BLACK); draw = ImageDraw.Draw(img)
    curveCol = wxColor(w["trendColor"]); valCol = wxColor(w["tempColor"])
    titleBot = draw_page_title(draw, "Temperatur", wxColor(w["condColor"]))
    hs = d["hours"]; n = len(hs); temps = [p[1] for p in hs]
    mn = min(temps); mx = max(temps); span = max(0.5, mx-mn)
    X0 = 30; X1 = W-12; Y0 = titleBot+24; Y1 = H-WX_PG_MARGIN_B; PH = Y1-Y0
    f8 = ttf_for_cap(prop_ascent(0))
    draw.text((2, Y0), "%d" % round(mx), font=f8, fill=valCol)
    draw.text((2, Y1-_th(draw, "8", f8)), "%d" % round(mn), font=f8, fill=valCol)
    draw.line([X0, Y1, X1, Y1], fill=C_GRAY, width=1)
    pts = []
    for i in range(n):
        px = X0 + (X1-X0)*i//(n-1); py = Y1 - int((temps[i]-mn)/span*PH); pts.append((px, py))
    for i in range(1, n):
        draw.line([pts[i-1][0], pts[i-1][1], pts[i][0], pts[i][1]], fill=curveCol, width=2)
    for (px, py) in pts:
        draw.ellipse([px-2, py-2, px+2, py+2], fill=curveCol)
    f10 = ttf_for_cap(prop_ascent(1)); fh = _th(draw, "8", f10)
    marks = [0, n//2, n-1]
    for i in marks:
        px, py = pts[i]; s = "%d" % round(temps[i])
        ty = py-4-fh
        if ty < Y0: ty = py+4
        draw_centered_at(draw, s, px, ty, fh, f10, valCol)
    for i in marks:
        px, _py = pts[i]; s = "%dh" % hs[i][0]
        draw_centered_at(draw, s, px, Y1+4, 16, f8, C_GRAY)
    return img

def render_page_rain(w, d):
    img = Image.new("RGB", (W, H), C_BLACK); draw = ImageDraw.Draw(img)
    barCol = wxColor(w["hourlyPop"])
    titleBot = draw_page_title(draw, "Regen %", wxColor(w["condColor"]))
    hs = d["hours"]; n = len(hs)
    X0 = 16; X1 = W-8; Y0 = titleBot+20; Y1 = H-WX_PG_MARGIN_B; PH = Y1-Y0
    slot = (X1-X0)//n; barW = max(3, slot*3//4)
    draw.line([X0, Y1, X1, Y1], fill=C_GRAY, width=1)
    f8 = ttf_for_cap(prop_ascent(0))
    for i in range(n):
        cx = X0 + slot*i + slot//2; pop = hs[i][2]; bh = PH*pop//100; by = Y1-bh
        draw.rectangle([cx-barW//2, by, cx-barW//2+barW, Y1], fill=barCol)
        if pop > 0:
            draw_centered_at(draw, "%d" % pop, cx, by-13, 12, f8, barCol)
        draw_centered_at(draw, "%d" % hs[i][0], cx, Y1+4, 16, f8, C_GRAY)
    return img

def render_page_days(w, d):
    img = Image.new("RGB", (W, H), C_BLACK); draw = ImageDraw.Draw(img)
    dayCol = wxColor(w["dailyColor"]); popCol = wxColor(w["dailyPop"])
    titleBot = draw_page_title(draw, "7 Tage", wxColor(w["condColor"]))
    days = d["days"]; rows = min(len(days), 7)
    top = titleBot+4; rowH = (H-top)//rows
    fR = ttf_for_cap(prop_ascent(2))         # helvB12
    CX_DAY = 24; CX_ICON = 72; CX_TEMP = 148; CX_POP = 212
    for r in range(rows):
        wday, tmax, tmin, popMax, code = days[r]; ry = top + rowH*r
        draw_centered_at(draw, WD_DE[wday], CX_DAY, ry, rowH, fR, dayCol)
        icol = icon_semantic_color(code, True) if w["iconColorMode"] == 0 else dayCol
        draw_icon(draw, wmo_icon(code, True), CX_ICON, ry+(rowH-16)//2, 16, icol)
        draw_centered_at(draw, "%d/%d" % (round(tmax), round(tmin)), CX_TEMP, ry, rowH, fR, dayCol)
        draw_centered_at(draw, "%d%%" % popMax, CX_POP, ry, rowH, fR, popCol)
    return img

# ---------------------------------------------------------------------------
# Scenarios
# ---------------------------------------------------------------------------
def scenarios():
    S = {}
    # (i) current/new default
    S["01_default"] = base_settings()
    # (ii) minimal
    m = base_settings(); m.update(showPrecip=False, showHourly=False); S["02_minimal"] = m
    # (iii) the overflow case the user hit
    o = base_settings(); o.update(showFeels=True, showHum=True, showWind=True, showPrecip=True,
                                  showHourly=True); S["03_overflow_details"] = o
    # (iv) all six blocks on
    a = base_settings(); a.update(showFeels=True, showHum=True, showWind=True, showPrecip=True,
                                  showHourly=True, showDaily=True, showTrend=True,
                                  hourlyCount=4, dailyCount=3, trendLabels=True); S["04_all_blocks"] = a
    # (v) large combo 1: 78px temp + icon + cond + hourly
    l1 = base_settings(); l1.update(tempSize=6, condSize=4, showPrecip=True, showHourly=True); S["05_large_temp78"] = l1
    # (vi) large combo 2: 78px temp + cond + all details + hourly + daily
    l2 = base_settings(); l2.update(tempSize=6, condSize=4, detailSize=2,
                                    showFeels=True, showHum=True, showWind=True, showPrecip=True,
                                    showHourly=True, showDaily=True); S["06_large_everything"] = l2
    return S

def main():
    d = SAMPLE
    S = scenarios()
    paths = []
    for algo in ("before", "after"):
        for name, w in S.items():
            img, shrunk, sizes = render(w, d, algo)
            tag = f"{name}_{algo}"
            if shrunk: tag += "_shrunk"
            p = os.path.join(OUT, tag + ".png")
            img.save(p)
            paths.append(p)
            print(f"{tag:40s} sizes={sizes}")
    print("\nWrote", len(paths), "PNGs to", OUT)

    # ---- deliverable: the four full-screen pages (default settings) ----
    w = base_settings()
    page_render = {
        "page_now":  render_page_now,
        "page_temp": render_page_temp,
        "page_rain": render_page_rain,
        "page_days": render_page_days,
    }
    print("\nPages:")
    for name, fn in page_render.items():
        p = os.path.join(OUT, name + ".png")
        fn(w, d).save(p)
        print(" ", os.path.abspath(p))

if __name__ == "__main__":
    main()
