// webui.h — single-page config UI served from PROGMEM
//
// Tabs: shared Status/WiFi/Display/Update plus the Clock and Usage feature tabs.
// The config JSON mirrors the nested Settings layout:
// { ..shared.., clockFace:{...}, usage:{...} }.
//
// i18n: the whole UI switches between English and German purely client-side.
// Static text carries data-i18n / data-i18n-html / data-i18n-ph attributes that
// applyLang() fills from the I18N dictionary; JS-generated strings (colour
// options, status rows, toasts, update/scan messages) go through t(). The choice
// lives in localStorage ('ss_lang'); default follows navigator.language. Nothing
// here changes the firmware, settings, or any value=""/enum sent to the device.
#pragma once
#include <Arduino.h>

static const char WEBUI_HTML[] PROGMEM = R"HTMLPAGE(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>SelfScreen</title>
<link rel="icon" type="image/png" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAHGElEQVR42tWXe2yVdxnHP7/f733PpT1dr6cUaNeytvSyrQ3UbREdk8GEwQi4BJTIqsY5F5hRExJdXBRdsjgVTVzA7GbmVg1zkAW2sWXRIIMKrFA2Si8jLZSetqe3c+jl9Fzf9/35xxmNCgU2nYvPn+/ted7v83yf3/crPL5czacYkk85jI/3mkBI8S9XtNag9SeIgBAIKZGGwvC4sHFwJCAFGo1ym0hDIaQEIf67CAgpEUoilEKjiUQmKamowk4miTpJDNPFxIUAZoYXKQTastG2g3ac/xABIRCGQrlMlNuNjUa5Xdy3+UF2PLeb4tIb8RfPZ82T21mwfiXKZWIlUwjTRLoMhKGuiYZxteTSUCjTJJFKkkxGKS1fyNZHn+SmqlrmeCUacFwm3owMKjespfTuO+ne8zrDJ99HOw7SZaJTFo5lzzofynB5t1/xhsvEQRONRqiqX0xpZQ3DQ/1cuHCBSHiEhsWL2b9vD2Z1GYY/j9Zdv0fl51C04k58pfOJDY2SCIURUiGkmLUdV2yBNAxStoUvJ48Ht/2Ynz7VxAMPb6OgaC4dp5vBgMmpCMHwMIWV5ahML1Y8TuezTXT/cS8qN4fyrV+n5P7VKF8GWut0O66nBUJJUlaKkpvKeWzHM/jy/Lyx90X27n6OL21oxDEUtbc20NV1hqQpyZ43j5Rj4SuZhxONEz7TycQH3eTfvoj8Oxrw19dy9ukmIgNDCCkvQ+IyBKRU2NomK99PXU01TTuf4IWfP8q9azew+LbPMzU9RemChbx3+iRZxXMxPR6SsSjju3GxyKxfwuYcaMTwegm8fYvjAX5lfXYk3NwftOAglr9ECIUAKpFIkbYuL0zFGhvqpX7GaR773E44eO8S8kjI83gw6zrZTWFPF9NgY4z293HDjfC6OjZFRU0nRwnKEx4WVTDIaHEJrB5RCCHEZK/6tAGYeMgzFRCxOWWUtW77/OEeOHeH1Ay+zqGEJo6FRBkJD+CsqCPcFmBgI4i30o2yboeAQriI/OJpIYIBTv/wdEwNDGB43WqRzzDoDMxUKgVKKWDzG0rVfZWR8nJ2/eQwEVFXV0XOui5RLkukvoO94C8loBOn1YDmaxGCQ4mVLkBkeRo+fQkwnmIgnsJMfrnCR3pyz7wEBQgqsVAqtBfGURdPTT5CTnUtWbg5z5hTzbsshMgsLkUoRDYdIRGN4DINblt/FmQNvM3mhn7p197J6/Tr6L4Z442e/ItHTh5QCfc1FpNOrNxaN4GjBn579BXMK5lBWdTPtHa14vZl093RQUL2A6FgY7Tj4qyuwLYvJ8XGk240rv4BoNMZfXnmVYCBAcnwSqRTYzrVpqLVGCkkiHmf387/mxMEDfO07P6K7p5PqylsIj4cYHh2kfuVdjPf3Y2T5yKmupGPvfhKRGHNXrSAzO4vOl/cx2PIeLiTS5UofWra+Bgt0ugDDMAmNDtN+8ijfeuSHHD/6N/rOd1NX10BPdzsWmhv8fkIfnEWYbvrfaUFKg+JVK9CxOOGjLQy1trH84W+wqHEDViqJQIBO55i1gEtnukAQj0W5Y+lKlq3ZRGhkkGQsSlVVHd1dbWQW5DNtW2ifj0j3OSLnzuMr9JPtz2Pknb8Tm5omr6iQnPpawhOT6KR1KUE6x+wIpAvQto1pmJxuPcbFhEVlbQOlN5YzMjLIqZNHCHZ00f7qfrKqKym6526UL5PJ3j5cXjcYioKyUmo2byAwPMRYaxtSKbRtz3z/qptQOw6ObeMy3fR2tXH2dAtL77mfseFBvrv1K2A7bFzfiKt/jMCe/SRGQxQt/wJZNVWk4kmyigoJtr7PRMpmqifAdGAQYahZ9cHlQ+hohOOAbSOQvPbSToQyCAZ6Wb+xkU2NW5hKJAn099J78E28WVnk11TgyctmbDiEp3g+wcPHib/05/Sfa42wHXActHP5EIorqWIh5YwQiacS5PoL+cHjv6W+ro7X9r/CH55/CtPl5r7N32a4PIvhM+0EDzaj/AXk37YIn9ekc9eLREfDSGWgU6m0SroeBC61ARvsFLgMN7GpCO+2HMfKLKC5+Qg33/oZNn5zG5F4hMNNOxg6dgIcjdk3SPx8gJzqclLRGFJKtJVC2/asekBczRcIpZCGwgEsO8XqB7awZM2X8XlMek8f45kd25kOXcRXVMiCdavIKCumc9cLRAJBDK8nDbvlpAdwlphVEc2wAo0UAqUMOk42MxkeIb+kAuXNwI5OYZQVUbhqGUmXycBbB4kFgkjDADudWNtXF6biupyREMgPVXEsGmFeWTlf3PQQn125mn3nDtP25ltMNJ8gPjyKNE201unE1+ETxEeyZkKgDINkMo5tWVTfvoSJRITRtnakMmbo9lEMivg43lAICQIS0WmUUOmzfqZl/wNrprUDGtwZmYC+Ir8/YW/4T3T9f3fH/wDGji8VfMwnvAAAAABJRU5ErkJggg==">
<style>
:root{--bg:#0e1116;--card:#171c24;--mut:#8b96a5;--fg:#e6edf3;--acc:#3fb950;--acc2:#2f81f7;--red:#f85149;--bd:#262d38}
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif;background:var(--bg);color:var(--fg);font-size:15px}
header{padding:14px 16px;border-bottom:1px solid var(--bd);display:flex;align-items:center;gap:10px}
header h1{font-size:17px;margin:0;font-weight:600}
header .dot{width:9px;height:9px;border-radius:50%;background:var(--mut)}
header .dot.ok{background:var(--acc)}
#langSel{margin-left:auto;width:auto;padding:5px 8px;font-size:13px;background:#0b0e13;border:1px solid var(--bd);border-radius:8px;color:var(--fg);cursor:pointer}
nav{display:flex;gap:4px;padding:8px;overflow-x:auto;border-bottom:1px solid var(--bd);position:sticky;top:0;background:var(--bg);z-index:5}
nav button{background:none;border:0;color:var(--mut);padding:8px 12px;border-radius:8px;font-size:14px;cursor:pointer;white-space:nowrap}
nav button.active{background:var(--card);color:var(--fg)}
main{padding:16px;max-width:680px;margin:0 auto}
.tab{display:none}.tab.active{display:block}
.card{background:var(--card);border:1px solid var(--bd);border-radius:12px;padding:16px;margin-bottom:14px}
h2{font-size:14px;text-transform:uppercase;letter-spacing:.04em;color:var(--mut);margin:0 0 12px}
label{display:block;margin:10px 0 4px;font-size:13px;color:var(--mut)}
input[type=text],input[type=password],input[type=number],input[type=url],select{
 width:100%;padding:9px 10px;background:#0b0e13;border:1px solid var(--bd);border-radius:8px;color:var(--fg);font-size:15px}
input[type=range]{width:100%}
.row{display:flex;gap:10px}.row>*{flex:1}
.chk{display:flex;align-items:center;gap:8px;margin:8px 0}
.chk input{width:18px;height:18px}
.chk label{margin:0;color:var(--fg);font-size:14px}
.carrow{display:flex;align-items:center;gap:10px;margin:8px 0}
.carrow>input[type=checkbox]{width:18px;height:18px;flex:none}
.carrow>span.nm{flex:1;color:var(--fg);font-size:14px}
.carrow>input[type=number]{width:64px;flex:none;padding:6px 8px}
button.btn{background:var(--acc);color:#04130a;border:0;padding:10px 16px;border-radius:9px;font-size:15px;font-weight:600;cursor:pointer}
button.btn.sec{background:#222b36;color:var(--fg)}
button.btn.danger{background:var(--red);color:#1a0606}
button.btn:disabled{opacity:.5}
.muted{color:var(--mut);font-size:13px}
table{width:100%;border-collapse:collapse}
td{padding:6px 4px}
.symrow input{margin:0}
.kv{display:flex;justify-content:space-between;padding:5px 0;border-bottom:1px solid var(--bd)}
.kv:last-child{border:0}.kv b{font-weight:600}
.toast{position:fixed;bottom:16px;left:50%;transform:translateX(-50%);background:#0b0e13;border:1px solid var(--bd);padding:10px 16px;border-radius:10px;opacity:0;transition:.3s;pointer-events:none}
.toast.show{opacity:1}
.net{padding:8px;border:1px solid var(--bd);border-radius:8px;margin:4px 0;cursor:pointer;display:flex;justify-content:space-between}
.net:hover{border-color:var(--acc2)}
.bar{height:8px;background:#0b0e13;border-radius:6px;overflow:hidden;margin-top:8px}
.bar>div{height:100%;width:0;background:var(--acc2);transition:.2s}
small.hint{display:block;color:var(--mut);margin-top:4px;font-size:12px}
.chip{display:inline-block;margin-left:8px;padding:2px 8px;border-radius:10px;font-size:11px;font-weight:600;letter-spacing:.03em;background:var(--acc2);color:#fff;vertical-align:middle}
</style></head>
<body>
<header><span id="dot" class="dot"></span><h1>SelfScreen</h1><span id="chip" class="chip" style="display:none"></span><span id="hi" class="muted"></span>
 <select id="langSel" aria-label="Language"><option value="de">DE</option><option value="en">EN</option></select></header>
<nav>
 <button data-t="status" class="active" data-i18n="nav_status">Status</button>
 <button data-t="wifi" data-i18n="nav_wifi">WiFi</button>
 <button data-t="display" data-i18n="nav_display">Display</button>
 <button data-t="clock" data-i18n="nav_clock">Clock</button>
 <button data-t="weather" data-i18n="nav_weather">Weather</button>
 <button data-t="usage" data-i18n="nav_usage">Usage</button>
 <button data-t="update" data-i18n="nav_update">Update</button>
</nav>
<main>
 <!-- STATUS -->
 <section id="status" class="tab active">
  <div class="card"><h2 data-i18n="h_device">Device</h2><div id="statusBox" class="muted" data-i18n="loading">Loading...</div></div>
 </section>

 <!-- WIFI -->
 <section id="wifi" class="tab">
  <div class="card"><h2 data-i18n="h_saved_networks">Saved networks</h2>
   <button class="btn sec" onclick="scan()" data-i18n="btn_scan">Scan networks</button>
   <div id="scanList"></div>
   <table id="wifiTable"></table>
   <button class="btn sec" style="margin-top:10px" onclick="addWifi()" data-i18n="btn_add_network">+ Add network</button>
   <div style="margin-top:14px"><button class="btn" onclick="saveWifi()" data-i18n="btn_save_connect">Save &amp; connect (reboots)</button></div>
   <small class="hint" data-i18n-html="hint_wifi">2.4&nbsp;GHz only. Up to 4 networks; at boot the device joins the strongest one it can see. Tap a scan result to fill a row. Leave a password blank to keep the stored one.</small>
  </div>
  <div class="card"><h2 data-i18n="h_device_name">Device name</h2>
   <label data-i18n="l_hostname">Hostname</label><input id="hostname" type="text" placeholder="smalltv">
   <small class="hint" data-i18n-html="hint_hostname">Reachable as <code>http://&lt;hostname&gt;.local</code> via mDNS. Running several SelfScreens? Give each its own name (<code>selfscreen-desk</code>, <code>selfscreen-shelf</code>) so browsers and the clawdmeter daemon <code>--push-to</code> reach the right device. Saving a new name reboots the device.</small>
  </div>
  <div class="card"><h2 data-i18n="h_ap">Setup hotspot (AP)</h2>
   <label data-i18n="l_ap_name">AP name</label><input id="apSsid" type="text">
   <label data-i18n-html="l_ap_pass">AP password <span class="muted">(blank = open, else min 8 chars)</span></label>
   <input id="apPass" type="text" placeholder="(unchanged)">
   <small class="hint" data-i18n="hint_ap">The AP appears when no WiFi is configured or the connection fails.</small>
  </div>
 </section>

 <!-- DISPLAY (shared) -->
 <section id="display" class="tab">
  <div class="card"><h2 data-i18n="h_mode">Mode</h2>
   <label data-i18n="l_what_shows">What this device shows</label>
   <select id="mode" onchange="modeChanged()">
    <option value="clock" data-i18n="opt_mode_clock">Clock</option>
    <option value="weather" data-i18n="opt_mode_weather">Weather</option>
    <option value="usage" data-i18n="opt_mode_usage">Claude usage</option>
    <option value="carousel" data-i18n="opt_mode_carousel">Carousel (rotate modes)</option>
   </select>
   <div id="carouselRow" class="card" style="margin:12px 0 0;background:#0b0e13">
    <h2 style="margin-top:0" data-i18n="h_carousel">Carousel</h2>
    <small class="hint" style="margin-top:0" data-i18n-html="hint_carousel">Only used when Mode is set to <b>Carousel</b> (above). The device rotates through the ticked screens, waiting the dwell time on each, in the order you choose (1 = shown first).</small>
    <label data-i18n="l_dwell">Dwell &mdash; switch every (s)</label><input id="carouselSec" type="number" min="5" max="3600">
    <label data-i18n-html="l_screens_rotation">Screens in rotation <span class="muted">(tick to include &middot; number = position)</span></label>
    <div class="carrow"><input id="carouselClock" type="checkbox"><span class="nm" data-i18n="opt_mode_clock">Clock</span><input id="carouselOrderClock" type="number" min="1" max="3"></div>
    <div class="carrow"><input id="carouselWeather" type="checkbox"><span class="nm" data-i18n="opt_mode_weather">Weather</span><input id="carouselOrderWeather" type="number" min="1" max="3"></div>
    <div class="carrow"><input id="carouselUsage" type="checkbox"><span class="nm" data-i18n="opt_mode_usage">Claude usage</span><input id="carouselOrderUsage" type="number" min="1" max="3"></div>
   </div>
   <small class="hint" data-i18n="hint_pick_feature">Pick the active feature, then configure it in its own tab.</small>
  </div>
  <div class="card"><h2 data-i18n="h_screen">Screen</h2>
   <label><span data-i18n="l_brightness">Brightness:</span> <span id="brVal"></span>%</label>
   <input id="brightness" type="range" min="0" max="100" oninput="brVal.textContent=this.value">
   <div class="chk"><input id="autoBrightness" type="checkbox"><label data-i18n="chk_autobright">Auto-brightness (light sensor on A0)</label></div>
   <label data-i18n="l_orientation">Orientation</label>
   <select id="rotation"><option value="0">0&deg;</option><option value="1">90&deg;</option>
    <option value="2">180&deg;</option><option value="3">270&deg;</option></select>
   <div class="chk"><input id="backlightInverted" type="checkbox"><label data-i18n="chk_backlight">Backlight is active-low (try if screen stays dark)</label></div>
  </div>
  <div class="card"><h2 data-i18n="h_clock_night">Clock &amp; night mode</h2>
   <label data-i18n="l_timezone">Timezone</label>
   <select id="tz"></select>
   <div class="muted" id="clockNow" style="margin:8px 0" data-i18n="clock_dash">Clock: -</div>
   <div class="chk"><input id="nightEnabled" type="checkbox"><label data-i18n="chk_night">Dim or blank the screen on a nightly schedule</label></div>
   <div class="row">
    <div><label data-i18n="l_from">From</label><input id="nightStart" type="time"></div>
    <div><label data-i18n="l_to">To</label><input id="nightEnd" type="time"></div>
   </div>
   <label><span data-i18n="l_nightbr">Night brightness:</span> <span id="nlVal"></span>% <span class="muted" data-i18n="nightbr_note">(0 = screen off)</span></label>
   <input id="nightLevel" type="range" min="0" max="100" oninput="nlVal.textContent=this.value">
   <small class="hint" data-i18n="hint_night">Needs internet once to set the clock over NTP (no on-screen clock, this just drives the schedule). While the window is active it overrides the brightness and auto-brightness above. Times are local to the selected timezone; DST is handled automatically. After a reboot the schedule resumes once the clock re-syncs, so the screen may show normal brightness for a few seconds.</small>
  </div>
 </section>

 <!-- CLOCK (feature) -->
 <section id="clock" class="tab">
  <div class="card"><h2 data-i18n="h_clock_face">Clock face</h2>
   <div class="chk"><input id="clk24" type="checkbox"><label data-i18n="chk_24h">24-hour time (off = 12-hour with AM/PM)</label></div>
   <div class="chk"><input id="clkSeconds" type="checkbox"><label data-i18n="chk_seconds">Show seconds</label></div>
   <div class="chk"><input id="clkWeekday" type="checkbox"><label data-i18n="chk_weekday">Show weekday (Montag, Dienstag, ...)</label></div>
   <label data-i18n="l_date_format">Date format</label>
   <select id="clkDateFmt">
    <option value="4" data-i18n="opt_datefmt_de">5. August 2026 (Deutsch)</option>
    <option value="0">DD.MM.YYYY</option>
    <option value="1">YYYY-MM-DD</option>
    <option value="2">DD.MM</option>
    <option value="3" data-i18n="opt_datefmt_off">Off (no date)</option>
   </select>
   <label data-i18n="l_element_sizes">Element sizes</label>
   <div class="row">
    <div><label class="muted"><span data-i18n="sz_time">Time</span> <span id="clkTimeSizeV"></span></label>
     <input id="clkTimeSize" type="range" min="0" max="6" oninput="clkTimeSizeV.textContent=(+this.value+1)+'/7'"></div>
    <div><label class="muted"><span data-i18n="sz_weekday">Weekday</span> <span id="clkWeekdaySizeV"></span></label>
     <input id="clkWeekdaySize" type="range" min="0" max="5" oninput="clkWeekdaySizeV.textContent=(+this.value+1)+'/6'"></div>
    <div><label class="muted"><span data-i18n="sz_date">Date</span> <span id="clkDateSizeV"></span></label>
     <input id="clkDateSize" type="range" min="0" max="5" oninput="clkDateSizeV.textContent=(+this.value+1)+'/6'"></div>
   </div>
   <label data-i18n="l_element_colours">Element colours</label>
   <div class="row">
    <div><label class="muted" data-i18n="sz_time">Time</label>
     <select id="clkTimeColor" class="wxc"></select></div>
    <div><label class="muted" data-i18n="sz_weekday">Weekday</label>
     <select id="clkWeekdayColor" class="wxc"></select></div>
   </div>
   <div class="row">
    <div><label class="muted" data-i18n="sz_date">Date</label>
     <select id="clkDateColor" class="wxc"></select></div>
    <div><label class="muted" data-i18n="l_separator">Separator line</label>
     <select id="clkLineColor" class="wxc"></select></div>
   </div>
   <small class="hint" data-i18n-html="hint_clockface">Every element is independent: drag a slider for its size and pick its colour. The clock is driven by NTP, which the device syncs automatically whenever the clock face is shown (or night mode is on). Set your timezone in the <b>Display</b> tab. Until the first sync lands the screen shows <code>--:--</code>.</small>
  </div>
 </section>

 <!-- WEATHER (feature) -->
 <section id="weather" class="tab">
  <div class="card"><h2 data-i18n="h_location_data">Location &amp; data</h2>
   <div class="row">
    <div><label data-i18n="l_latitude">Latitude</label><input id="wxLat" type="number" step="0.01" min="-90" max="90"></div>
    <div><label data-i18n="l_longitude">Longitude</label><input id="wxLon" type="number" step="0.01" min="-180" max="180"></div>
   </div>
   <div class="row">
    <div><label data-i18n="l_temp_unit">Temperature unit</label>
     <select id="wxUnit"><option value="0">&deg;C (Celsius)</option><option value="1">&deg;F (Fahrenheit)</option></select></div>
    <div><label data-i18n="l_wind_unit">Wind unit</label>
     <select id="wxWindUnit"><option value="0">km/h</option><option value="1">mph</option><option value="2">m/s</option></select></div>
   </div>
   <div class="row">
    <div><label data-i18n="l_rain_detail">Rain in detail line</label>
     <select id="wxPrecipPct"><option value="0" data-i18n="opt_rain_mm">mm (amount)</option><option value="1" data-i18n="opt_rain_pct">% (chance)</option></select></div>
    <div><label data-i18n="l_wx_refresh">Refresh weather (s)</label><input id="wxRefresh" type="number" min="60" max="21600"></div>
   </div>
   <label data-i18n="l_layout_preset">Layout preset</label>
   <select id="wxPreset" onchange="wxApplyPreset(this.value)">
    <option value="" data-i18n="opt_preset_pick">— pick a starting point —</option>
    <option value="minimal" data-i18n="opt_preset_minimal">Minimal (icon + temp + condition)</option>
    <option value="stunden" data-i18n="opt_preset_stunden">Stunden (primary + hourly strip)</option>
    <option value="tage" data-i18n="opt_preset_tage">Tage (primary + daily strip)</option>
    <option value="detail" data-i18n="opt_preset_detail">Detail (primary + details + hourly)</option>
    <option value="alles" data-i18n="opt_preset_alles">Alles (everything on)</option>
   </select>
   <small class="hint" data-i18n-html="hint_weather">Data from <a href="https://open-meteo.com" target="_blank">Open-Meteo</a> over plain HTTP (no API key). Default location is Hamburg (53.55, 9.99). Condition text is German. A preset just prefills the toggles below — fine-tune anything, then <b>Save settings</b>. Until the first fetch lands the screen shows <code>--</code>.</small>
  </div>

  <div class="card"><h2 data-i18n="h_wx_a">A &middot; Now (temperature + icon)</h2>
   <div class="chk"><input id="wxShowTemp" type="checkbox"><label data-i18n="chk_bigtemp">Big temperature (with drawn &deg; ring)</label></div>
   <div class="chk"><input id="wxShowBigIcon" type="checkbox"><label data-i18n="chk_bigicon">Big weather icon beside the temperature</label></div>
   <div class="row">
    <div><label class="muted"><span data-i18n="l_temp_size">Temperature size</span> <span id="wxTempSizeV"></span></label>
     <input id="wxTempSize" type="range" min="0" max="6" oninput="wxTempSizeV.textContent=(+this.value+1)+'/7'"></div>
   </div>
   <div class="row">
    <div><label class="muted" data-i18n="l_temp_colour">Temperature colour</label><select id="wxTempColor" class="wxc"></select></div>
    <div><label class="muted" data-i18n="l_icon_colour">Icon colour</label><select id="wxBigIconColor" class="wxc"></select></div>
   </div>
  </div>

  <div class="card"><h2 data-i18n="h_wx_b">B &middot; Condition text</h2>
   <div class="chk"><input id="wxShowCond" type="checkbox"><label data-i18n="chk_cond">Condition text (Klar, Bewölkt, Regen, ...)</label></div>
   <div class="row">
    <div><label class="muted"><span data-i18n="l_size">Size</span> <span id="wxCondSizeV"></span></label>
     <input id="wxCondSize" type="range" min="0" max="5" oninput="wxCondSizeV.textContent=(+this.value+1)+'/6'"></div>
    <div><label class="muted" data-i18n="l_colour">Colour</label><select id="wxCondColor" class="wxc"></select></div>
   </div>
  </div>

  <div class="card"><h2 data-i18n="h_wx_c">C &middot; Detail line</h2>
   <div class="chk"><input id="wxShowFeels" type="checkbox"><label data-i18n="chk_feels">Feels-like (Gefühlt 20)</label></div>
   <div class="chk"><input id="wxShowHum" type="checkbox"><label data-i18n="chk_hum">Humidity (Luft 61%)</label></div>
   <div class="chk"><input id="wxShowWind" type="checkbox"><label data-i18n="chk_wind">Wind (Wind 12)</label></div>
   <div class="chk"><input id="wxShowPrecip" type="checkbox"><label data-i18n="chk_precip">Precipitation (Regen 0.0 mm / %)</label></div>
   <div class="row">
    <div><label class="muted"><span data-i18n="l_size">Size</span> <span id="wxDetailSizeV"></span></label>
     <input id="wxDetailSize" type="range" min="0" max="5" oninput="wxDetailSizeV.textContent=(+this.value+1)+'/6'"></div>
    <div><label class="muted" data-i18n="l_colour">Colour</label><select id="wxDetailColor" class="wxc"></select></div>
   </div>
   <small class="hint" data-i18n-html="hint_detail">Enabled items join into one centred line. The proportional font is ASCII-only, so temperatures here print without the &deg; symbol.</small>
  </div>

  <div class="card"><h2 data-i18n="h_wx_d">D &middot; Hourly strip</h2>
   <div class="chk"><input id="wxShowHourly" type="checkbox"><label data-i18n="chk_hourly">Show hourly strip</label></div>
   <div class="row">
    <div><label class="muted"><span data-i18n="l_columns">Columns</span> <span id="wxHourlyCountV"></span></label>
     <input id="wxHourlyCount" type="range" min="3" max="6" oninput="wxHourlyCountV.textContent=this.value"></div>
    <div><label class="muted"><span data-i18n="l_every">Every</span> <span id="wxHourlyStepV"></span> <span data-i18n="l_hours_h">h</span></label>
     <input id="wxHourlyStep" type="range" min="1" max="3" oninput="wxHourlyStepV.textContent=this.value"></div>
   </div>
   <label class="muted" data-i18n="l_per_column">Show per column</label>
   <div class="chk"><input id="wxHrHour" type="checkbox"><label data-i18n="chk_hr_hour">Hour (15h)</label></div>
   <div class="chk"><input id="wxHrIcon" type="checkbox"><label data-i18n="chk_mini_icon">Mini weather icon</label></div>
   <div class="chk"><input id="wxHrTemp" type="checkbox"><label data-i18n="chk_hr_temp">Temperature (24)</label></div>
   <div class="chk"><input id="wxHrPop" type="checkbox"><label data-i18n="chk_hr_pop">Rain probability (20%)</label></div>
   <div class="row">
    <div><label class="muted"><span data-i18n="l_size">Size</span> <span id="wxHourlySizeV"></span></label>
     <input id="wxHourlySize" type="range" min="0" max="5" oninput="wxHourlySizeV.textContent=(+this.value+1)+'/6'"></div>
   </div>
   <div class="row">
    <div><label class="muted" data-i18n="l_text_colour">Text colour</label><select id="wxHourlyColor" class="wxc"></select></div>
    <div><label class="muted" data-i18n="l_rain_colour">Rain% colour</label><select id="wxHourlyPop" class="wxc"></select></div>
   </div>
  </div>

  <div class="card"><h2 data-i18n="h_wx_e">E &middot; Daily strip</h2>
   <div class="chk"><input id="wxShowDaily" type="checkbox"><label data-i18n="chk_daily">Show daily strip</label></div>
   <div class="row">
    <div><label class="muted"><span data-i18n="l_columns">Columns</span> <span id="wxDailyCountV"></span></label>
     <input id="wxDailyCount" type="range" min="2" max="4" oninput="wxDailyCountV.textContent=this.value"></div>
   </div>
   <label class="muted" data-i18n="l_per_column">Show per column</label>
   <div class="chk"><input id="wxDyDay" type="checkbox"><label data-i18n="chk_dy_day">Weekday (Fr)</label></div>
   <div class="chk"><input id="wxDyIcon" type="checkbox"><label data-i18n="chk_mini_icon">Mini weather icon</label></div>
   <div class="chk"><input id="wxDyTemps" type="checkbox"><label data-i18n="chk_dy_temps">Max/min (24/16)</label></div>
   <div class="chk"><input id="wxDyPop" type="checkbox"><label data-i18n="chk_dy_pop">Rain probability (30%)</label></div>
   <div class="row">
    <div><label class="muted"><span data-i18n="l_size">Size</span> <span id="wxDailySizeV"></span></label>
     <input id="wxDailySize" type="range" min="0" max="5" oninput="wxDailySizeV.textContent=(+this.value+1)+'/6'"></div>
   </div>
   <div class="row">
    <div><label class="muted" data-i18n="l_text_colour">Text colour</label><select id="wxDailyColor" class="wxc"></select></div>
    <div><label class="muted" data-i18n="l_rain_colour">Rain% colour</label><select id="wxDailyPop" class="wxc"></select></div>
   </div>
  </div>

  <div class="card"><h2 data-i18n="h_wx_f">F &middot; Temperature trend</h2>
   <div class="chk"><input id="wxShowTrend" type="checkbox"><label data-i18n="chk_trend">12h temperature sparkline</label></div>
   <div class="chk"><input id="wxTrendLabels" type="checkbox"><label data-i18n="chk_trend_labels">Min/max labels</label></div>
   <div class="row">
    <div><label class="muted" data-i18n="l_colour">Colour</label><select id="wxTrendColor" class="wxc"></select></div>
   </div>
   <small class="hint" data-i18n-html="hint_trend">Blocks stack top&rarr;bottom (A&rarr;F) and auto-fit 240&times;240. Turning many blocks on at big sizes can overflow; the gaps tighten and the lowest block may clip.</small>
  </div>
 </section>

 <!-- USAGE (feature) -->
 <section id="usage" class="tab">
  <div class="card"><h2 data-i18n="h_usage">Claude usage</h2>
   <label data-i18n="l_usage_url">Usage daemon URL</label>
   <input id="usageUrl" type="url" placeholder="http://192.168.1.10:8787/">
   <label data-i18n="l_usage_refresh">Refresh data (s)</label><input id="usagePollSec" type="number" min="10" max="3600">
   <small class="hint" data-i18n-html="hint_usage">Runs on the PC-side <a href="https://github.com/giovi321/clawdmeter-daemon" target="_blank">clawdmeter-daemon</a>, which reads your Claude usage and sends it here. <b>Pull:</b> set the Usage URL to the daemon. <b>Push:</b> leave it blank and run the daemon with <code>--push-to &lt;hostname&gt;.local</code> (for networks where the device cannot reach the PC). Running several SelfScreens? Give each a unique hostname in the WiFi tab so every PC pushes to its own device. Idle animation plays until data arrives.</small>
  </div>
 </section>

 <!-- UPDATE -->
 <section id="update" class="tab">
  <div class="card"><h2 data-i18n="h_update_gh">Update from GitHub</h2>
   <div class="muted"><span data-i18n="l_installed">Installed:</span> <b id="fwVer">-</b></div>
   <div style="margin-top:10px">
    <button class="btn sec" onclick="checkUpdate()" id="chkBtn" data-i18n="btn_check_latest">Check for latest</button>
    <button class="btn" style="margin-left:8px" onclick="selfUpdate()" id="ghUpBtn" disabled data-i18n="btn_update_now">Update now</button>
   </div>
   <div id="ghMsg" class="muted" style="margin-top:8px"></div>
   <small class="hint" data-i18n-html="hint_update_gh">Pulls the newest release straight from <a id="repoLink" href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">the GitHub repo</a>. HTTPS OTA is tight on the ESP8266; if it fails, use the manual upload below.</small>
  </div>
  <div class="card"><h2 data-i18n="h_manual_ota">Manual update (OTA)</h2>
   <input id="fw" type="file" accept=".bin">
   <div style="margin-top:12px"><button class="btn" onclick="upload()" id="upBtn" data-i18n="btn_upload_flash">Upload &amp; flash</button></div>
   <div class="bar"><div id="upBar"></div></div>
   <div id="upMsg" class="muted" style="margin-top:8px"></div>
   <small class="hint" data-i18n-html="hint_manual_ota">Upload a firmware.bin from the <a href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">releases page</a> or a local build. The device reboots when done.</small>
  </div>
  <div class="card"><h2 data-i18n="h_backup">Settings backup</h2>
   <button class="btn sec" onclick="location.href='/api/export'" data-i18n="btn_export">Export settings</button>
   <input id="cfgFile" type="file" accept=".json,application/json" style="margin-top:10px">
   <div style="margin-top:10px"><button class="btn" onclick="importCfg()" data-i18n="btn_import">Import &amp; reboot</button></div>
   <small class="hint" data-i18n-html="hint_backup">The export is the device <code>config.json</code>, including WiFi passwords in clear text; treat the file accordingly. Import applies everything and reboots.</small>
  </div>
  <div class="card"><h2 data-i18n="h_maintenance">Maintenance</h2>
   <button class="btn sec" onclick="reboot()" data-i18n="btn_reboot">Reboot</button>
   <button class="btn danger" style="margin-left:8px" onclick="factory()" data-i18n="btn_factory">Factory reset</button>
  </div>
 </section>
</main>

<div style="text-align:center;padding:0 0 16px"><button class="btn" onclick="saveAll()" data-i18n="btn_save">Save settings</button></div>
<div style="text-align:center;padding:0 0 24px;font-size:12px">
 <a id="footRepo" href="https://github.com/s3lfcod3r/SelfScreen" target="_blank" style="color:var(--acc2);text-decoration:none">GitHub: s3lfcod3r/SelfScreen</a>
 <span class="muted" data-i18n="foot_based"> · based on smalltv-mod</span>
 <span id="footVer" class="muted"></span>
</div>
<div id="toast" class="toast"></div>

<script>
var C={};
function $(id){return document.getElementById(id)}
// null-safe field helpers: a lean build removes some feature tabs entirely
function sv(id,v){var e=$(id);if(e)e.value=(v!=null?v:'')}
function sc(id,v){var e=$(id);if(e)e.checked=!!v}
function gv(id){var e=$(id);return e?e.value:''}
function gc(id){var e=$(id);return e?e.checked:false}
function toast(m){var t=$('toast');t.textContent=m;t.classList.add('show');setTimeout(function(){t.classList.remove('show')},2200)}
function j(url,opt){return fetch(url,opt).then(function(r){return r.json()})}

// ===== i18n =====
// Every user-visible string lives here as {en,de}. Static markup carries
// data-i18n (textContent), data-i18n-html (innerHTML, for hints with tags) or
// data-i18n-ph (placeholder); JS-generated text goes through t(). Brand tokens
// (SelfScreen, smalltv), URLs, format tokens and value="" enums stay as-is.
var LANG='en';
var I18N={
 nav_status:{en:'Status',de:'Status'},
 nav_wifi:{en:'WiFi',de:'WLAN'},
 nav_display:{en:'Display',de:'Anzeige'},
 nav_clock:{en:'Clock',de:'Uhr'},
 nav_weather:{en:'Weather',de:'Wetter'},
 nav_usage:{en:'Usage',de:'Auslastung'},
 nav_update:{en:'Update',de:'Update'},
 h_device:{en:'Device',de:'Gerät'},
 loading:{en:'Loading...',de:'Lädt...'},
 h_saved_networks:{en:'Saved networks',de:'Gespeicherte Netzwerke'},
 btn_scan:{en:'Scan networks',de:'Netzwerke suchen'},
 btn_add_network:{en:'+ Add network',de:'+ Netzwerk hinzufügen'},
 btn_save_connect:{en:'Save & connect (reboots)',de:'Speichern & verbinden (Neustart)'},
 hint_wifi:{en:'2.4&nbsp;GHz only. Up to 4 networks; at boot the device joins the strongest one it can see. Tap a scan result to fill a row. Leave a password blank to keep the stored one.',de:'Nur 2,4&nbsp;GHz. Bis zu 4 Netzwerke; beim Start verbindet sich das Gerät mit dem stärksten erreichbaren. Tippe ein Suchergebnis an, um eine Zeile zu füllen. Lasse das Passwort leer, um das gespeicherte zu behalten.'},
 h_device_name:{en:'Device name',de:'Gerätename'},
 l_hostname:{en:'Hostname',de:'Hostname'},
 hint_hostname:{en:'Reachable as <code>http://&lt;hostname&gt;.local</code> via mDNS. Running several SelfScreens? Give each its own name (<code>selfscreen-desk</code>, <code>selfscreen-shelf</code>) so browsers and the clawdmeter daemon <code>--push-to</code> reach the right device. Saving a new name reboots the device.',de:'Erreichbar als <code>http://&lt;hostname&gt;.local</code> über mDNS. Mehrere SelfScreens im Einsatz? Gib jedem einen eigenen Namen (<code>selfscreen-desk</code>, <code>selfscreen-shelf</code>), damit Browser und der <code>--push-to</code>-Parameter des clawdmeter-Daemons das richtige Gerät erreichen. Das Speichern eines neuen Namens startet das Gerät neu.'},
 h_ap:{en:'Setup hotspot (AP)',de:'Einrichtungs-Hotspot (AP)'},
 l_ap_name:{en:'AP name',de:'AP-Name'},
 l_ap_pass:{en:'AP password <span class="muted">(blank = open, else min 8 chars)</span>',de:'AP-Passwort <span class="muted">(leer = offen, sonst min. 8 Zeichen)</span>'},
 hint_ap:{en:'The AP appears when no WiFi is configured or the connection fails.',de:'Der AP erscheint, wenn kein WLAN konfiguriert ist oder die Verbindung fehlschlägt.'},
 h_mode:{en:'Mode',de:'Modus'},
 l_what_shows:{en:'What this device shows',de:'Was dieses Gerät anzeigt'},
 opt_mode_clock:{en:'Clock',de:'Uhr'},
 opt_mode_weather:{en:'Weather',de:'Wetter'},
 opt_mode_usage:{en:'Claude usage',de:'Claude-Auslastung'},
 opt_mode_carousel:{en:'Carousel (rotate modes)',de:'Karussell (Modi wechseln)'},
 h_carousel:{en:'Carousel',de:'Karussell'},
 hint_carousel:{en:'Only used when Mode is set to <b>Carousel</b> (above). The device rotates through the ticked screens, waiting the dwell time on each, in the order you choose (1 = shown first).',de:'Nur aktiv, wenn der Modus oben auf <b>Karussell</b> steht. Das Gerät wechselt durch die angehakten Screens, verweilt je die Wechselzeit und in der von dir gewählten Reihenfolge (1 = zuerst gezeigt).'},
 l_dwell:{en:'Dwell &mdash; switch every (s)',de:'Verweildauer &mdash; wechseln alle (s)'},
 l_screens_rotation:{en:'Screens in rotation <span class="muted">(tick to include &middot; number = position)</span>',de:'Screens im Wechsel <span class="muted">(anhaken zum Einbeziehen &middot; Zahl = Position)</span>'},
 hint_pick_feature:{en:'Pick the active feature, then configure it in its own tab.',de:'Wähle die aktive Funktion und konfiguriere sie im eigenen Tab.'},
 h_screen:{en:'Screen',de:'Bildschirm'},
 l_brightness:{en:'Brightness:',de:'Helligkeit:'},
 chk_autobright:{en:'Auto-brightness (light sensor on A0)',de:'Auto-Helligkeit (Lichtsensor an A0)'},
 l_orientation:{en:'Orientation',de:'Ausrichtung'},
 chk_backlight:{en:'Backlight is active-low (try if screen stays dark)',de:'Hintergrundbeleuchtung ist active-low (probieren, falls der Screen dunkel bleibt)'},
 h_clock_night:{en:'Clock & night mode',de:'Uhr & Nachtmodus'},
 l_timezone:{en:'Timezone',de:'Zeitzone'},
 clock_dash:{en:'Clock: -',de:'Uhr: -'},
 chk_night:{en:'Dim or blank the screen on a nightly schedule',de:'Screen nachts nach Zeitplan dimmen oder abschalten'},
 l_from:{en:'From',de:'Von'},
 l_to:{en:'To',de:'Bis'},
 l_nightbr:{en:'Night brightness:',de:'Nacht-Helligkeit:'},
 nightbr_note:{en:'(0 = screen off)',de:'(0 = Screen aus)'},
 hint_night:{en:'Needs internet once to set the clock over NTP (no on-screen clock, this just drives the schedule). While the window is active it overrides the brightness and auto-brightness above. Times are local to the selected timezone; DST is handled automatically. After a reboot the schedule resumes once the clock re-syncs, so the screen may show normal brightness for a few seconds.',de:'Braucht einmal Internet, um die Uhr per NTP zu stellen (keine Uhr auf dem Screen, steuert nur den Zeitplan). Während das Zeitfenster aktiv ist, überschreibt es Helligkeit und Auto-Helligkeit oben. Zeiten gelten in der gewählten Zeitzone; die Sommerzeit wird automatisch berücksichtigt. Nach einem Neustart läuft der Zeitplan weiter, sobald die Uhr neu synchronisiert ist — der Screen kann daher ein paar Sekunden normal hell sein.'},
 h_clock_face:{en:'Clock face',de:'Uhr-Anzeige'},
 chk_24h:{en:'24-hour time (off = 12-hour with AM/PM)',de:'24-Stunden-Zeit (aus = 12-Stunden mit AM/PM)'},
 chk_seconds:{en:'Show seconds',de:'Sekunden anzeigen'},
 chk_weekday:{en:'Show weekday (Montag, Dienstag, ...)',de:'Wochentag anzeigen (Montag, Dienstag, ...)'},
 l_date_format:{en:'Date format',de:'Datumsformat'},
 opt_datefmt_de:{en:'5. August 2026 (German)',de:'5. August 2026 (Deutsch)'},
 opt_datefmt_off:{en:'Off (no date)',de:'Aus (kein Datum)'},
 l_element_sizes:{en:'Element sizes',de:'Elementgrößen'},
 sz_time:{en:'Time',de:'Zeit'},
 sz_weekday:{en:'Weekday',de:'Wochentag'},
 sz_date:{en:'Date',de:'Datum'},
 l_element_colours:{en:'Element colours',de:'Elementfarben'},
 l_separator:{en:'Separator line',de:'Trennlinie'},
 hint_clockface:{en:'Every element is independent: drag a slider for its size and pick its colour. The clock is driven by NTP, which the device syncs automatically whenever the clock face is shown (or night mode is on). Set your timezone in the <b>Display</b> tab. Until the first sync lands the screen shows <code>--:--</code>.',de:'Jedes Element ist unabhängig: Größe per Schieberegler, Farbe per Auswahl. Die Uhr läuft über NTP, das das Gerät automatisch synchronisiert, sobald die Uhr angezeigt wird (oder der Nachtmodus aktiv ist). Stelle deine Zeitzone im Tab <b>Anzeige</b> ein. Bis zur ersten Synchronisierung zeigt der Screen <code>--:--</code>.'},
 h_location_data:{en:'Location & data',de:'Standort & Daten'},
 l_latitude:{en:'Latitude',de:'Breitengrad'},
 l_longitude:{en:'Longitude',de:'Längengrad'},
 l_temp_unit:{en:'Temperature unit',de:'Temperatureinheit'},
 l_wind_unit:{en:'Wind unit',de:'Windeinheit'},
 l_rain_detail:{en:'Rain in detail line',de:'Regen in Detailzeile'},
 opt_rain_mm:{en:'mm (amount)',de:'mm (Menge)'},
 opt_rain_pct:{en:'% (chance)',de:'% (Wahrscheinlichkeit)'},
 l_wx_refresh:{en:'Refresh weather (s)',de:'Wetter aktualisieren (s)'},
 l_layout_preset:{en:'Layout preset',de:'Layout-Vorlage'},
 opt_preset_pick:{en:'— pick a starting point —',de:'— Startpunkt wählen —'},
 opt_preset_minimal:{en:'Minimal (icon + temp + condition)',de:'Minimal (Symbol + Temp + Wetterlage)'},
 opt_preset_stunden:{en:'Hourly (primary + hourly strip)',de:'Stunden (Haupt + Stundenleiste)'},
 opt_preset_tage:{en:'Daily (primary + daily strip)',de:'Tage (Haupt + Tagesleiste)'},
 opt_preset_detail:{en:'Detail (primary + details + hourly)',de:'Detail (Haupt + Details + Stunden)'},
 opt_preset_alles:{en:'All (everything on)',de:'Alles (alles an)'},
 hint_weather:{en:'Data from <a href="https://open-meteo.com" target="_blank">Open-Meteo</a> over plain HTTP (no API key). Default location is Hamburg (53.55, 9.99). Condition text is German. A preset just prefills the toggles below — fine-tune anything, then <b>Save settings</b>. Until the first fetch lands the screen shows <code>--</code>.',de:'Daten von <a href="https://open-meteo.com" target="_blank">Open-Meteo</a> über einfaches HTTP (kein API-Schlüssel). Standardort ist Hamburg (53,55, 9,99). Der Wetter-Text ist deutsch. Eine Vorlage füllt nur die Schalter unten vor — passe alles an und dann <b>Einstellungen speichern</b>. Bis zum ersten Abruf zeigt der Screen <code>--</code>.'},
 h_wx_a:{en:'A &middot; Now (temperature + icon)',de:'A &middot; Jetzt (Temperatur + Symbol)'},
 chk_bigtemp:{en:'Big temperature (with drawn &deg; ring)',de:'Große Temperatur (mit gezeichnetem &deg;-Ring)'},
 chk_bigicon:{en:'Big weather icon beside the temperature',de:'Großes Wettersymbol neben der Temperatur'},
 l_temp_size:{en:'Temperature size',de:'Temperaturgröße'},
 l_temp_colour:{en:'Temperature colour',de:'Temperaturfarbe'},
 l_icon_colour:{en:'Icon colour',de:'Symbolfarbe'},
 h_wx_b:{en:'B &middot; Condition text',de:'B &middot; Wetter-Text'},
 chk_cond:{en:'Condition text (Klar, Bewölkt, Regen, ...)',de:'Wetter-Text (Klar, Bewölkt, Regen, ...)'},
 l_size:{en:'Size',de:'Größe'},
 l_colour:{en:'Colour',de:'Farbe'},
 h_wx_c:{en:'C &middot; Detail line',de:'C &middot; Detailzeile'},
 chk_feels:{en:'Feels-like (Gefühlt 20)',de:'Gefühlte Temperatur (Gefühlt 20)'},
 chk_hum:{en:'Humidity (Luft 61%)',de:'Luftfeuchte (Luft 61%)'},
 chk_wind:{en:'Wind (Wind 12)',de:'Wind (Wind 12)'},
 chk_precip:{en:'Precipitation (Regen 0.0 mm / %)',de:'Niederschlag (Regen 0.0 mm / %)'},
 hint_detail:{en:'Enabled items join into one centred line. The proportional font is ASCII-only, so temperatures here print without the &deg; symbol.',de:'Aktivierte Angaben bilden eine zentrierte Zeile. Die proportionale Schrift kann nur ASCII, daher werden Temperaturen hier ohne &deg;-Zeichen dargestellt.'},
 h_wx_d:{en:'D &middot; Hourly strip',de:'D &middot; Stundenleiste'},
 chk_hourly:{en:'Show hourly strip',de:'Stundenleiste anzeigen'},
 l_columns:{en:'Columns',de:'Spalten'},
 l_every:{en:'Every',de:'Alle'},
 l_hours_h:{en:'h',de:'Std.'},
 l_per_column:{en:'Show per column',de:'Pro Spalte anzeigen'},
 chk_hr_hour:{en:'Hour (15h)',de:'Stunde (15h)'},
 chk_mini_icon:{en:'Mini weather icon',de:'Mini-Wettersymbol'},
 chk_hr_temp:{en:'Temperature (24)',de:'Temperatur (24)'},
 chk_hr_pop:{en:'Rain probability (20%)',de:'Regenwahrscheinlichkeit (20%)'},
 l_text_colour:{en:'Text colour',de:'Textfarbe'},
 l_rain_colour:{en:'Rain% colour',de:'Regen-%-Farbe'},
 h_wx_e:{en:'E &middot; Daily strip',de:'E &middot; Tagesleiste'},
 chk_daily:{en:'Show daily strip',de:'Tagesleiste anzeigen'},
 chk_dy_day:{en:'Weekday (Fr)',de:'Wochentag (Fr)'},
 chk_dy_temps:{en:'Max/min (24/16)',de:'Max/Min (24/16)'},
 chk_dy_pop:{en:'Rain probability (30%)',de:'Regenwahrscheinlichkeit (30%)'},
 h_wx_f:{en:'F &middot; Temperature trend',de:'F &middot; Temperaturverlauf'},
 chk_trend:{en:'12h temperature sparkline',de:'12-Stunden-Temperaturkurve'},
 chk_trend_labels:{en:'Min/max labels',de:'Min/Max-Beschriftung'},
 hint_trend:{en:'Blocks stack top&rarr;bottom (A&rarr;F) and auto-fit 240&times;240. Turning many blocks on at big sizes can overflow; the gaps tighten and the lowest block may clip.',de:'Blöcke stapeln sich von oben nach unten (A&rarr;F) und passen sich an 240&times;240 an. Viele Blöcke in großer Größe können überlaufen; die Abstände werden enger und der unterste Block kann abgeschnitten werden.'},
 h_usage:{en:'Claude usage',de:'Claude-Auslastung'},
 l_usage_url:{en:'Usage daemon URL',de:'URL des Auslastungs-Daemons'},
 l_usage_refresh:{en:'Refresh data (s)',de:'Daten aktualisieren (s)'},
 hint_usage:{en:'Runs on the PC-side <a href="https://github.com/giovi321/clawdmeter-daemon" target="_blank">clawdmeter-daemon</a>, which reads your Claude usage and sends it here. <b>Pull:</b> set the Usage URL to the daemon. <b>Push:</b> leave it blank and run the daemon with <code>--push-to &lt;hostname&gt;.local</code> (for networks where the device cannot reach the PC). Running several SelfScreens? Give each a unique hostname in the WiFi tab so every PC pushes to its own device. Idle animation plays until data arrives.',de:'Läuft auf dem PC über den <a href="https://github.com/giovi321/clawdmeter-daemon" target="_blank">clawdmeter-daemon</a>, der deine Claude-Auslastung ausliest und hierher schickt. <b>Pull:</b> trage die Auslastungs-URL des Daemons ein. <b>Push:</b> lasse sie leer und starte den Daemon mit <code>--push-to &lt;hostname&gt;.local</code> (für Netzwerke, in denen das Gerät den PC nicht erreicht). Mehrere SelfScreens? Gib jedem im WLAN-Tab einen eigenen Hostnamen, damit jeder PC an sein eigenes Gerät sendet. Bis Daten ankommen, läuft eine Leerlauf-Animation.'},
 h_update_gh:{en:'Update from GitHub',de:'Update von GitHub'},
 l_installed:{en:'Installed:',de:'Installiert:'},
 btn_check_latest:{en:'Check for latest',de:'Auf neueste prüfen'},
 btn_update_now:{en:'Update now',de:'Jetzt aktualisieren'},
 hint_update_gh:{en:'Pulls the newest release straight from <a id="repoLink" href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">the GitHub repo</a>. HTTPS OTA is tight on the ESP8266; if it fails, use the manual upload below.',de:'Holt die neueste Version direkt aus <a id="repoLink" href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">dem GitHub-Repo</a>. HTTPS-OTA ist auf dem ESP8266 knapp; falls es fehlschlägt, nutze den manuellen Upload unten.'},
 h_manual_ota:{en:'Manual update (OTA)',de:'Manuelles Update (OTA)'},
 btn_upload_flash:{en:'Upload & flash',de:'Hochladen & flashen'},
 hint_manual_ota:{en:'Upload a firmware.bin from the <a href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">releases page</a> or a local build. The device reboots when done.',de:'Lade eine firmware.bin von der <a href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">Releases-Seite</a> oder einem lokalen Build hoch. Das Gerät startet danach neu.'},
 h_backup:{en:'Settings backup',de:'Einstellungs-Backup'},
 btn_export:{en:'Export settings',de:'Einstellungen exportieren'},
 btn_import:{en:'Import & reboot',de:'Importieren & neu starten'},
 hint_backup:{en:'The export is the device <code>config.json</code>, including WiFi passwords in clear text; treat the file accordingly. Import applies everything and reboots.',de:'Der Export ist die <code>config.json</code> des Geräts, inklusive WLAN-Passwörter im Klartext; behandle die Datei entsprechend. Der Import übernimmt alles und startet neu.'},
 h_maintenance:{en:'Maintenance',de:'Wartung'},
 btn_reboot:{en:'Reboot',de:'Neu starten'},
 btn_factory:{en:'Factory reset',de:'Werksreset'},
 btn_save:{en:'Save settings',de:'Einstellungen speichern'},
 foot_based:{en:' · based on smalltv-mod',de:' · basiert auf smalltv-mod'},
 // colours (CLK_COL_* order)
 col_white:{en:'White',de:'Weiß'},
 col_teal:{en:'Teal',de:'Türkis'},
 col_green:{en:'Green',de:'Grün'},
 col_yellow:{en:'Yellow',de:'Gelb'},
 col_red:{en:'Red',de:'Rot'},
 col_blue:{en:'Blue',de:'Blau'},
 col_gray:{en:'Gray',de:'Grau'},
 col_selfblue_light:{en:'Self-Blue (light)',de:'Self-Blau (hell)'},
 col_selfblue_medium:{en:'Self-Blue (medium)',de:'Self-Blau (mittel)'},
 // status rows
 st_firmware:{en:'Firmware',de:'Firmware'},
 st_mode:{en:'Mode',de:'Modus'},
 st_network:{en:'Network',de:'Netzwerk'},
 st_ip:{en:'IP',de:'IP'},
 st_mdns:{en:'mDNS',de:'mDNS'},
 st_signal:{en:'Signal',de:'Signal'},
 st_heap:{en:'Free heap',de:'Freier Speicher'},
 st_uptime:{en:'Uptime',de:'Laufzeit'},
 st_reset:{en:'Last reset',de:'Letzter Reset'},
 setup_mode:{en:'setup mode',de:'Einrichtungsmodus'},
 // clock status line
 clock_ntp_off:{en:'Clock: NTP runs only when night mode is on',de:'Uhr: NTP läuft nur bei aktivem Nachtmodus'},
 clock_label:{en:'Clock:',de:'Uhr:'},
 synced:{en:'synced',de:'synchronisiert'},
 waiting_ntp:{en:'waiting for NTP...',de:'warte auf NTP...'},
 night_active:{en:'night mode active',de:'Nachtmodus aktiv'},
 night_waiting:{en:'night mode waiting for NTP',de:'Nachtmodus wartet auf NTP'},
 last_update:{en:'Last update:',de:'Letztes Update:'},
 // placeholders
 ph_ssid:{en:'SSID',de:'SSID'},
 ph_password:{en:'password',de:'Passwort'},
 ph_unchanged:{en:'(unchanged)',de:'(unverändert)'},
 ph_open:{en:'(open)',de:'(offen)'},
 // toasts & confirms
 t_saved_reboot:{en:'Saved — rebooting...',de:'Gespeichert — starte neu...'},
 t_saved:{en:'Saved',de:'Gespeichert'},
 t_wifi_saved:{en:'Saved, rebooting to connect...',de:'Gespeichert, starte neu zum Verbinden...'},
 t_max4:{en:'Max 4',de:'Max. 4'},
 t_rebooting:{en:'Rebooting...',de:'Starte neu...'},
 t_reset_rebooting:{en:'Reset, rebooting...',de:'Zurückgesetzt, starte neu...'},
 t_pick_json:{en:'Pick a config .json first',de:'Erst eine config-.json wählen'},
 t_not_json:{en:'Not valid JSON',de:'Kein gültiges JSON'},
 t_imported:{en:'Imported, rebooting...',de:'Importiert, starte neu...'},
 t_import_failed:{en:'Import failed',de:'Import fehlgeschlagen'},
 t_pick_bin:{en:'Pick a .bin first',de:'Erst eine .bin wählen'},
 c_selfupdate:{en:'Download and flash the latest release from GitHub? The device reboots if it succeeds.',de:'Neueste Version von GitHub laden und flashen? Bei Erfolg startet das Gerät neu.'},
 c_import:{en:'Apply this configuration and reboot?',de:'Diese Konfiguration übernehmen und neu starten?'},
 c_reboot:{en:'Reboot device?',de:'Gerät neu starten?'},
 c_factory:{en:'Erase ALL settings and reboot?',de:'ALLE Einstellungen löschen und neu starten?'},
 // scan
 scan_scanning:{en:'Scanning...',de:'Suche...'},
 scan_none:{en:'No networks found',de:'Keine Netzwerke gefunden'},
 // github update messages
 upd_checking:{en:'Checking GitHub...',de:'Prüfe GitHub...'},
 upd_check_failed:{en:'Check failed',de:'Prüfung fehlgeschlagen'},
 upd_unknown:{en:'unknown',de:'unbekannt'},
 upd_ver:{en:'Version',de:'Version'},
 upd_available:{en:'is available',de:'ist verfügbar'},
 upd_installed:{en:'installed',de:'installiert'},
 upd_uptodate:{en:'Up to date',de:'Aktuell'},
 upd_downloading:{en:'Downloading and flashing... this can take a couple of minutes and the device may reboot twice.',de:'Lade herunter und flashe... das kann ein paar Minuten dauern und das Gerät startet evtl. zweimal neu.'},
 upd_updated_to:{en:'Updated to',de:'Aktualisiert auf'},
 upd_failed:{en:'Update failed',de:'Update fehlgeschlagen'},
 upd_cant_start:{en:'Could not start update',de:'Update konnte nicht gestartet werden'},
 // OTA upload
 ota_uploading:{en:'Uploading',de:'Lade hoch'},
 ota_done:{en:'Done. Rebooting...',de:'Fertig. Starte neu...'},
 ota_failed:{en:'Failed',de:'Fehlgeschlagen'},
 ota_error:{en:'Upload error',de:'Upload-Fehler'}
};
function t(k){var e=I18N[k];return e?(e[LANG]!=null?e[LANG]:e.en):k}
var COLKEYS=['col_white','col_teal','col_green','col_yellow','col_red','col_blue','col_gray','col_selfblue_light','col_selfblue_medium'];
function colOpts(){var h='';for(var i=0;i<COLKEYS.length;i++)h+='<option value="'+i+'">'+t(COLKEYS[i])+'</option>';return h}
function applyLang(lang){
 LANG=(lang==='de')?'de':'en';
 try{localStorage.setItem('ss_lang',LANG)}catch(e){}
 document.documentElement.lang=LANG;
 var sel=$('langSel'); if(sel)sel.value=LANG;
 document.querySelectorAll('[data-i18n]').forEach(function(el){var v=t(el.getAttribute('data-i18n'));if(v!=null)el.textContent=v});
 document.querySelectorAll('[data-i18n-html]').forEach(function(el){var v=t(el.getAttribute('data-i18n-html'));if(v!=null)el.innerHTML=v});
 document.querySelectorAll('[data-i18n-ph]').forEach(function(el){var v=t(el.getAttribute('data-i18n-ph'));if(v!=null)el.placeholder=v});
 fillColors(true);
 var ap=$('apPass'); if(ap&&C&&('apPassSet' in C))ap.placeholder=C.apPassSet?t('ph_unchanged'):t('ph_open');
 document.querySelectorAll('#wifiTable tr').forEach(function(tr){
  var ws=tr.querySelector('.ws'); if(ws)ws.placeholder=t('ph_ssid');
  var wp=tr.querySelector('.wp'); if(wp)wp.placeholder=(wp.dataset.passset==='1')?t('ph_unchanged'):t('ph_password');});
 if(window._lastStatus)renderStatus(window._lastStatus);
}
function initLang(){
 var saved=null;try{saved=localStorage.getItem('ss_lang')}catch(e){}
 var lang=saved||(((navigator.language||'en').toLowerCase().indexOf('de')===0)?'de':'en');
 var sel=$('langSel'); if(sel)sel.onchange=function(){applyLang(sel.value)};
 applyLang(lang);
}

// tabs
document.querySelectorAll('nav button').forEach(function(b){b.onclick=function(){
 document.querySelectorAll('nav button').forEach(function(x){x.classList.remove('active')});
 document.querySelectorAll('.tab').forEach(function(x){x.classList.remove('active')});
 b.classList.add('active');$(b.dataset.t).classList.add('active');
}});

// IANA -> POSIX TZ. The device stores/uses the POSIX rule; this map lives in the
// browser so the firmware carries no tz database.
var TZMAP={
 '':'UTC0','UTC':'UTC0',
 'Europe/London':'GMT0BST,M3.5.0/1,M10.5.0','Europe/Dublin':'GMT0IST,M3.5.0/1,M10.5.0',
 'Europe/Lisbon':'WET0WEST,M3.5.0/1,M10.5.0',
 'Europe/Rome':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Paris':'CET-1CEST,M3.5.0,M10.5.0/3',
 'Europe/Berlin':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Madrid':'CET-1CEST,M3.5.0,M10.5.0/3',
 'Europe/Amsterdam':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Brussels':'CET-1CEST,M3.5.0,M10.5.0/3',
 'Europe/Zurich':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Vienna':'CET-1CEST,M3.5.0,M10.5.0/3',
 'Europe/Warsaw':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Prague':'CET-1CEST,M3.5.0,M10.5.0/3',
 'Europe/Stockholm':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Oslo':'CET-1CEST,M3.5.0,M10.5.0/3',
 'Europe/Copenhagen':'CET-1CEST,M3.5.0,M10.5.0/3',
 'Europe/Athens':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Helsinki':'EET-2EEST,M3.5.0/3,M10.5.0/4',
 'Europe/Bucharest':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Kyiv':'EET-2EEST,M3.5.0/3,M10.5.0/4',
 'Europe/Istanbul':'<+03>-3','Europe/Moscow':'MSK-3',
 'America/New_York':'EST5EDT,M3.2.0,M11.1.0','America/Toronto':'EST5EDT,M3.2.0,M11.1.0',
 'America/Chicago':'CST6CDT,M3.2.0,M11.1.0','America/Denver':'MST7MDT,M3.2.0,M11.1.0',
 'America/Phoenix':'MST7','America/Los_Angeles':'PST8PDT,M3.2.0,M11.1.0',
 'America/Anchorage':'AKST9AKDT,M3.2.0,M11.1.0','America/Sao_Paulo':'<-03>3',
 'America/Mexico_City':'CST6','America/Bogota':'<-05>5','America/Argentina/Buenos_Aires':'<-03>3',
 'Asia/Dubai':'<+04>-4','Asia/Karachi':'PKT-5','Asia/Kolkata':'IST-5:30',
 'Asia/Dhaka':'<+06>-6','Asia/Bangkok':'<+07>-7','Asia/Jakarta':'WIB-7',
 'Asia/Shanghai':'CST-8','Asia/Hong_Kong':'HKT-8','Asia/Singapore':'<+08>-8',
 'Asia/Taipei':'CST-8','Asia/Tokyo':'JST-9','Asia/Seoul':'KST-9',
 'Australia/Perth':'AWST-8','Australia/Sydney':'AEST-10AEDT,M10.1.0,M4.1.0/3',
 'Australia/Adelaide':'ACST-9:30ACDT,M10.1.0,M4.1.0/3','Australia/Brisbane':'AEST-10',
 'Pacific/Auckland':'NZST-12NZDT,M9.5.0,M4.1.0/3','Pacific/Honolulu':'HST10'};
function fillTz(){var s=$('tz');if(!s)return;var keys=Object.keys(TZMAP).filter(function(k){return k!==''});
 keys.sort();s.innerHTML='<option value="">UTC</option>'+keys.map(function(k){return '<option value="'+k+'">'+k+'</option>'}).join('');}

// Weather + clock colour <select>s share one option list (CLK_COL_* order),
// built from the dict so they follow the language. Rebuild preserves the value.
function fillColors(force){document.querySelectorAll('select.wxc').forEach(function(s){
 if(force||!s.options.length){var v=s.value;s.innerHTML=colOpts();if(v!=='')s.value=v;}});}
function wxSet(id,v){var e=$(id);if(!e)return;if(e.type==='checkbox')e.checked=!!v;else{e.value=v;e.dispatchEvent(new Event('input'));}}
// Layout presets only prefill the controls; the real state is the saved settings.
function wxApplyPreset(p){if(!p)return;
 var base={wxShowTemp:1,wxShowBigIcon:1,wxShowCond:1,wxShowFeels:0,wxShowHum:0,wxShowWind:0,wxShowPrecip:0,wxShowHourly:0,wxShowDaily:0,wxShowTrend:0,wxTrendLabels:0};
 var P={minimal:{},
  stunden:{wxShowHourly:1,wxHourlyCount:4,wxHourlyStep:1,wxHrHour:1,wxHrIcon:1,wxHrTemp:1,wxHrPop:1},
  tage:{wxShowDaily:1,wxDailyCount:3,wxDyDay:1,wxDyIcon:1,wxDyTemps:1,wxDyPop:1},
  detail:{wxShowFeels:1,wxShowHum:1,wxShowWind:1,wxShowPrecip:1,wxShowHourly:1,wxHourlyCount:3,wxHourlyStep:1,wxHrHour:1,wxHrIcon:1,wxHrTemp:1,wxHrPop:1},
  alles:{wxShowFeels:1,wxShowHum:1,wxShowWind:1,wxShowPrecip:1,wxShowHourly:1,wxHourlyCount:4,wxHourlyStep:1,wxHrHour:1,wxHrIcon:1,wxHrTemp:1,wxHrPop:1,wxShowDaily:1,wxDailyCount:3,wxDyDay:1,wxDyIcon:1,wxDyTemps:1,wxDyPop:1,wxShowTrend:1,wxTrendLabels:1}};
 var m=Object.assign({},base,P[p]||{});
 for(var k in m)wxSet(k,m[k]);
}
function modeChanged(){if(!$('mode'))return;
 $('carouselRow').style.display=$('mode').value==='carousel'?'block':'none';}
function loadConfig(){return j('/api/config').then(function(c){C=c;
 var u=c.usage||{};
 // shared
 ['apSsid','apPass','hostname'].forEach(function(k){$(k).value=c[k]!=null?c[k]:''});
 renderWifi(c.wifi||(c.staSsid?[{ssid:c.staSsid,passSet:c.staPassSet}]:[]));
 $('brightness').value=c.brightness; $('brVal').textContent=c.brightness;
 $('rotation').value=c.rotation;
 $('autoBrightness').checked=!!c.autoBrightness;
 $('backlightInverted').checked=!!c.backlightInverted;
 // header chip = which chip this firmware was built for
 var chipName={esp8266:'ESP8266',esp32c2:'ESP32-C2',esp32:'ESP32'}[c.chip]||'';
 var chE=$('chip'); if(chE&&chipName){chE.textContent=chipName;chE.style.display='inline-block';}
 // clock slice
 fillTz(); var ck=c.clock||{};
 if(ck.tz && !(ck.tz in TZMAP)){var _ts=$('tz'); if(_ts){var _o=document.createElement('option');_o.value=ck.tz;_o.textContent=ck.tz;_ts.appendChild(_o);}}
 sv('tz',ck.tz||''); sc('nightEnabled',!!ck.nightEnabled);
 sv('nightStart',ck.nightStart||'22:00'); sv('nightEnd',ck.nightEnd||'07:00');
 sv('nightLevel',ck.nightLevel!=null?ck.nightLevel:0); $('nlVal')&&($('nlVal').textContent=(ck.nightLevel!=null?ck.nightLevel:0));
 $('mode').value=c.mode||'clock'; modeChanged();
 sv('carouselSec',c.carouselSec||30);
 sc('carouselClock',c.carouselClock!==false);
 sc('carouselWeather',c.carouselWeather!==false);
 sc('carouselUsage',c.carouselUsage!==false);
 sv('carouselOrderClock',c.carouselOrderClock!=null?c.carouselOrderClock:1);
 sv('carouselOrderWeather',c.carouselOrderWeather!=null?c.carouselOrderWeather:2);
 sv('carouselOrderUsage',c.carouselOrderUsage!=null?c.carouselOrderUsage:3);
 // clock face slice
 fillColors();
 var cf=c.clockFace||{};
 sc('clk24',cf.hour24!==false); sc('clkSeconds',!!cf.showSeconds); sc('clkWeekday',cf.showWeekday!==false);
 sv('clkDateFmt',cf.dateFormat!=null?cf.dateFormat:4);
 sv('clkTimeColor',cf.timeColor!=null?cf.timeColor:0);
 sv('clkDateColor',cf.dateColor!=null?cf.dateColor:0);
 sv('clkWeekdayColor',cf.weekdayColor!=null?cf.weekdayColor:0);
 sv('clkLineColor',cf.lineColor!=null?cf.lineColor:7);
 var _sz=function(id,v,mx){var e=$(id);if(!e)return;e.value=(v!=null?v:4);e.dispatchEvent(new Event('input'));};
 _sz('clkTimeSize',cf.timeSize,6);_sz('clkWeekdaySize',cf.weekdaySize,5);_sz('clkDateSize',cf.dateSize,5);
 // weather slice
 var wx=c.weather||{};
 sv('wxLat',wx.lat!=null?wx.lat:53.55); sv('wxLon',wx.lon!=null?wx.lon:9.99);
 sv('wxUnit',wx.unitF?1:0); sv('wxWindUnit',wx.windUnit!=null?wx.windUnit:0);
 sv('wxPrecipPct',wx.precipPct?1:0); sv('wxRefresh',wx.refreshSec!=null?wx.refreshSec:600);
 sc('wxShowTemp',wx.showTemp!==false); sc('wxShowBigIcon',wx.showBigIcon!==false); sc('wxShowCond',wx.showCond!==false);
 sc('wxShowFeels',!!wx.showFeels); sc('wxShowHum',!!wx.showHum); sc('wxShowWind',!!wx.showWind); sc('wxShowPrecip',!!wx.showPrecip);
 sc('wxShowHourly',wx.showHourly!==false); sc('wxShowDaily',!!wx.showDaily); sc('wxShowTrend',!!wx.showTrend);
 sc('wxHrHour',wx.hrHour!==false); sc('wxHrIcon',wx.hrIcon!==false); sc('wxHrTemp',wx.hrTemp!==false); sc('wxHrPop',wx.hrPop!==false);
 sc('wxDyDay',wx.dyDay!==false); sc('wxDyIcon',wx.dyIcon!==false); sc('wxDyTemps',wx.dyTemps!==false); sc('wxDyPop',wx.dyPop!==false);
 sc('wxTrendLabels',!!wx.trendLabels);
 sv('wxTempColor',wx.tempColor!=null?wx.tempColor:0);
 sv('wxBigIconColor',wx.bigIconColor!=null?wx.bigIconColor:3);
 sv('wxCondColor',wx.condColor!=null?wx.condColor:7);
 sv('wxDetailColor',wx.detailColor!=null?wx.detailColor:6);
 sv('wxHourlyColor',wx.hourlyColor!=null?wx.hourlyColor:0); sv('wxHourlyPop',wx.hourlyPop!=null?wx.hourlyPop:1);
 sv('wxDailyColor',wx.dailyColor!=null?wx.dailyColor:0); sv('wxDailyPop',wx.dailyPop!=null?wx.dailyPop:1);
 sv('wxTrendColor',wx.trendColor!=null?wx.trendColor:1);
 var _wsz=function(id,v,dv){var e=$(id);if(!e)return;e.value=(v!=null?v:dv);e.dispatchEvent(new Event('input'));};
 _wsz('wxTempSize',wx.tempSize,5);_wsz('wxCondSize',wx.condSize,3);_wsz('wxDetailSize',wx.detailSize,1);
 _wsz('wxHourlySize',wx.hourlySize,0);_wsz('wxDailySize',wx.dailySize,0);
 _wsz('wxHourlyCount',wx.hourlyCount,4);_wsz('wxHourlyStep',wx.hourlyStep,1);_wsz('wxDailyCount',wx.dailyCount,3);
 // usage slice
 sv('usageUrl',u.usageUrl);
 sv('usagePollSec',u.pollSec);
 var ap=$('apPass'); if(ap)ap.placeholder=c.apPassSet?t('ph_unchanged'):t('ph_open');
})}

function esc(s){return (''+(s==null?'':s)).replace(/[<>&"]/g,function(c){return {'<':'&lt;','>':'&gt;','&':'&amp;','"':'&quot;'}[c]})}

function collect(){
 var o={mode:gv('mode'),
  carouselSec:parseInt(gv('carouselSec'))||30,
  carouselClock:gc('carouselClock'),
  carouselWeather:gc('carouselWeather'),
  carouselUsage:gc('carouselUsage'),
  carouselOrderClock:parseInt(gv('carouselOrderClock'))||1,
  carouselOrderWeather:parseInt(gv('carouselOrderWeather'))||2,
  carouselOrderUsage:parseInt(gv('carouselOrderUsage'))||3,
  brightness:parseInt(gv('brightness'))||0,
  rotation:parseInt(gv('rotation')),
  autoBrightness:gc('autoBrightness'),
  backlightInverted:gc('backlightInverted'),
  hostname:gv('hostname'), apSsid:gv('apSsid'), apPass:gv('apPass'),
  wifi:collectWifi()};
 // clock face slice
 if($('clk24')) o.clockFace={hour24:gc('clk24'),showSeconds:gc('clkSeconds'),showWeekday:gc('clkWeekday'),
  dateFormat:parseInt(gv('clkDateFmt'))||0,timeColor:parseInt(gv('clkTimeColor'))||0,
  dateColor:parseInt(gv('clkDateColor'))||0,weekdayColor:parseInt(gv('clkWeekdayColor'))||0,
  lineColor:parseInt(gv('clkLineColor'))||0,
  timeSize:parseInt(gv('clkTimeSize'))||0,weekdaySize:parseInt(gv('clkWeekdaySize'))||0,
  dateSize:parseInt(gv('clkDateSize'))||0};
 // weather slice
 if($('wxLat')) o.weather={lat:parseFloat(gv('wxLat')),lon:parseFloat(gv('wxLon')),
  unitF:gv('wxUnit')==='1',windUnit:parseInt(gv('wxWindUnit'))||0,precipPct:gv('wxPrecipPct')==='1',
  refreshSec:parseInt(gv('wxRefresh'))||600,
  showTemp:gc('wxShowTemp'),showBigIcon:gc('wxShowBigIcon'),showCond:gc('wxShowCond'),
  showFeels:gc('wxShowFeels'),showHum:gc('wxShowHum'),showWind:gc('wxShowWind'),showPrecip:gc('wxShowPrecip'),
  showHourly:gc('wxShowHourly'),showDaily:gc('wxShowDaily'),showTrend:gc('wxShowTrend'),
  hourlyCount:parseInt(gv('wxHourlyCount'))||4,hourlyStep:parseInt(gv('wxHourlyStep'))||1,
  hrHour:gc('wxHrHour'),hrIcon:gc('wxHrIcon'),hrTemp:gc('wxHrTemp'),hrPop:gc('wxHrPop'),
  dailyCount:parseInt(gv('wxDailyCount'))||3,
  dyDay:gc('wxDyDay'),dyIcon:gc('wxDyIcon'),dyTemps:gc('wxDyTemps'),dyPop:gc('wxDyPop'),
  trendLabels:gc('wxTrendLabels'),
  tempSize:parseInt(gv('wxTempSize'))||0,condSize:parseInt(gv('wxCondSize'))||0,detailSize:parseInt(gv('wxDetailSize'))||0,
  hourlySize:parseInt(gv('wxHourlySize'))||0,dailySize:parseInt(gv('wxDailySize'))||0,
  tempColor:parseInt(gv('wxTempColor'))||0,bigIconColor:parseInt(gv('wxBigIconColor'))||0,
  condColor:parseInt(gv('wxCondColor'))||0,detailColor:parseInt(gv('wxDetailColor'))||0,
  hourlyColor:parseInt(gv('wxHourlyColor'))||0,hourlyPop:parseInt(gv('wxHourlyPop'))||0,
  dailyColor:parseInt(gv('wxDailyColor'))||0,dailyPop:parseInt(gv('wxDailyPop'))||0,
  trendColor:parseInt(gv('wxTrendColor'))||0};
 // usage slice
 if($('usage')) o.usage={usageUrl:gv('usageUrl'), pollSec:parseInt(gv('usagePollSec'))||0};
 // clock slice
 if($('tz')){var _tzn=gv('tz'); var _tzp=(_tzn in TZMAP)?TZMAP[_tzn]:((C.clock&&C.clock.tz===_tzn&&C.clock.tzPosix)?C.clock.tzPosix:'UTC0');
  o.clock={tz:_tzn,tzPosix:_tzp,
  nightEnabled:gc('nightEnabled'),nightStart:gv('nightStart')||'22:00',
  nightEnd:gv('nightEnd')||'07:00',nightLevel:parseInt(gv('nightLevel'))||0};}
 return o;
}
function saveAll(){j('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(collect())})
 .then(function(r){toast(r.reboot?t('t_saved_reboot'):t('t_saved'));if(r.reboot)setTimeout(function(){location.reload()},6000);loadStatus()})}

function saveWifi(){
 var o={wifi:collectWifi()};
 j('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(o)}).then(function(){
  toast(t('t_wifi_saved'));j('/api/reboot',{method:'POST'});
 });
}

// wifi networks (up to 4)
function renderWifi(arr){var tb=$('wifiTable');if(!tb)return;tb.innerHTML='';arr.forEach(addWifiRow);if(!arr.length)addWifiRow({})}
function addWifiRow(o){var tb=$('wifiTable');var tr=document.createElement('tr');tr.className='symrow';
 tr.innerHTML='<td style="width:44%"><input class="ws" type="text" autocomplete="off" placeholder="'+t('ph_ssid')+'" value="'+esc(o.ssid||'')+'"></td>'+
  '<td><input class="wp" type="password" autocomplete="off" data-passset="'+(o.passSet?'1':'0')+'" placeholder="'+(o.passSet?t('ph_unchanged'):t('ph_password'))+'"></td>'+
  '<td style="width:34px"><button class="btn sec" style="padding:6px 10px" onclick="this.closest(\'tr\').remove()">&times;</button></td>';
 tb.appendChild(tr);}
function addWifi(){if(document.querySelectorAll('#wifiTable tr').length>=4){toast(t('t_max4'));return}addWifiRow({})}
function collectWifi(){var w=[];document.querySelectorAll('#wifiTable tr').forEach(function(tr){
 var s=tr.querySelector('.ws').value.trim();if(!s)return;
 var e={ssid:s};var p=tr.querySelector('.wp').value;if(p)e.pass=p;w.push(e);});return w}
function scanPick(ssid){var rows=document.querySelectorAll('#wifiTable tr');var tr=null;
 for(var i=0;i<rows.length;i++){if(!rows[i].querySelector('.ws').value.trim()){tr=rows[i];break}}
 if(!tr){if(rows.length>=4){toast(t('t_max4'));return}addWifiRow({});tr=$('wifiTable').lastChild}
 tr.querySelector('.ws').value=ssid;tr.querySelector('.wp').focus();}

// wifi scan
function scan(){$('scanList').innerHTML='<div class="muted">'+t('scan_scanning')+'</div>';
 j('/api/scan').then(function(l){var h='';l.sort(function(a,b){return b.rssi-a.rssi});
  l.forEach(function(n){h+='<div class="net" onclick="scanPick(this.dataset.s)" data-s="'+
   esc(n.ssid)+'"><span>'+(n.enc?'🔒 ':'')+esc(n.ssid)+'</span><span class="muted">'+n.rssi+' dBm</span></div>'});
  $('scanList').innerHTML=h||'<div class="muted">'+t('scan_none')+'</div>';})}

// status
function renderStatus(s){window._lastStatus=s;
 $('dot').className='dot'+(s.connected?' ok':'');
 $('hi').textContent=s.mode==='ap'?t('setup_mode'):(s.ip||'');
 var cn=$('clockNow'); if(cn){var ne=!!(C.clock&&C.clock.nightEnabled);var ns=s.night?'  · '+t('night_active'):(s.nightHeld?'  · '+t('night_waiting'):'');cn.textContent=!ne?t('clock_ntp_off'):(t('clock_label')+' '+(s.synced?(s.time||t('synced'))+(s.tz?' ('+s.tz+')':''):t('waiting_ntp'))+ns);}
 var fw=$('fwVer'); if(fw)fw.textContent=s.fw+' '+s.version;
 // Surface the result of a boot-time GitHub update (ESP8266) once on first load,
 // so a failure that happened across the reboot is visible even if the original
 // Update tab was closed. Don't clobber an in-progress check/update message.
 if(!window._otaShown){window._otaShown=1;var gm=$('ghMsg');if(gm&&!gm.textContent&&s.updateMsg&&s.updateMsg!=='updating...')gm.textContent=t('last_update')+' '+s.updateMsg}
 var fv=$('footVer'); if(fv)fv.textContent=' v'+s.version;
 if(s.repo){var rl=$('repoLink'); if(rl)rl.href=s.repo+'/releases'; var fr=$('footRepo'); if(fr)fr.href=s.repo;}
 $('statusBox').innerHTML=
  kv(t('st_firmware'),s.fw+' '+s.version)+kv(t('st_mode'),s.mode.toUpperCase())+
  kv(t('st_network'),s.ssid||'-')+kv(t('st_ip'),s.ip||'-')+kv(t('st_mdns'),'http://'+(C.hostname||'smalltv')+'.local')+
  kv(t('st_signal'),s.rssi?s.rssi+' dBm':'-')+
  kv(t('st_heap'),s.heap+' B')+kv(t('st_uptime'),fmtUp(s.uptime))+kv(t('st_reset'),s.reset||'-');
}
function loadStatus(){return j('/api/status').then(renderStatus)}
function kv(k,v){return '<div class="kv"><span class="muted">'+k+'</span><b>'+v+'</b></div>'}
function fmtUp(s){var d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60);
 return (d?d+'d ':'')+(h?h+'h ':'')+m+'m'}

// GitHub self-update
function checkUpdate(){$('ghMsg').textContent=t('upd_checking');$('chkBtn').disabled=true;
 j('/api/checkupdate').then(function(u){$('chkBtn').disabled=false;
  if(!u.ok){$('ghMsg').textContent=t('upd_check_failed')+': '+(u.error||t('upd_unknown'));return}
  if(u.newer){$('ghMsg').innerHTML=t('upd_ver')+' <b>'+u.latest+'</b> '+t('upd_available')+' ('+t('upd_installed')+' '+u.current+').';$('ghUpBtn').disabled=false}
  else{$('ghMsg').textContent=t('upd_uptodate')+' ('+u.current+').';$('ghUpBtn').disabled=true}
 }).catch(function(){$('chkBtn').disabled=false;$('ghMsg').textContent=t('upd_check_failed')})}
function selfUpdate(){if(!confirm(t('c_selfupdate')))return;
 $('ghUpBtn').disabled=true;$('chkBtn').disabled=true;
 $('ghMsg').textContent=t('upd_downloading');
 // Installed version, read synchronously from the already-loaded status so the
 // poller below can recognise success (new version) without racing a fetch.
 var cur=(($('fwVer').textContent||'').trim().split(' ').pop())||'';
 j('/api/selfupdate',{method:'POST'}).then(function(){
  var n=0;var tm=setInterval(function(){n++;
   j('/api/status').then(function(s){
    if(cur&&s.version&&s.version!==cur){clearInterval(tm);$('ghMsg').textContent=t('upd_updated_to')+' '+s.version+'.';$('chkBtn').disabled=false;return}
    var m=s.updateMsg||'';
    if(m&&m!=='starting...'&&m!=='updating...'){clearInterval(tm);$('ghMsg').textContent=t('upd_failed')+': '+m;$('chkBtn').disabled=false}
   }).catch(function(){});
   if(n>100)clearInterval(tm);
  },3000);
 }).catch(function(){$('ghMsg').textContent=t('upd_cant_start');$('chkBtn').disabled=false})}

// settings backup
function importCfg(){var f=$('cfgFile').files[0];if(!f){toast(t('t_pick_json'));return}
 var r=new FileReader();
 r.onload=function(){var txt=r.result;
  try{JSON.parse(txt)}catch(e){toast(t('t_not_json'));return}
  if(!confirm(t('c_import')))return;
  j('/api/import',{method:'POST',headers:{'Content-Type':'application/json'},body:txt})
   .then(function(){toast(t('t_imported'));setTimeout(function(){location.reload()},8000)})
   .catch(function(){toast(t('t_import_failed'))});
 };
 r.readAsText(f);}

// maintenance
function reboot(){if(confirm(t('c_reboot')))j('/api/reboot',{method:'POST'}).then(function(){toast(t('t_rebooting'))})}
function factory(){if(confirm(t('c_factory')))j('/api/factory',{method:'POST'}).then(function(){toast(t('t_reset_rebooting'))})}

// OTA
function upload(){var f=$('fw').files[0];if(!f){toast(t('t_pick_bin'));return}
 var fd=new FormData();fd.append('firmware',f,f.name);
 var x=new XMLHttpRequest();x.open('POST','/update');
 $('upBtn').disabled=true;
 x.upload.onprogress=function(e){if(e.lengthComputable){var p=Math.round(e.loaded/e.total*100);$('upBar').style.width=p+'%';$('upMsg').textContent=t('ota_uploading')+' '+p+'%'}};
 x.onload=function(){$('upBtn').disabled=false;if(x.status==200){$('upMsg').textContent=t('ota_done');$('upBar').style.width='100%';setTimeout(function(){location.reload()},9000)}else{$('upMsg').textContent=t('ota_failed')+': '+x.responseText}};
 x.onerror=function(){$('upBtn').disabled=false;$('upMsg').textContent=t('ota_error')};
 x.send(fd);
}

initLang();
loadConfig().then(loadStatus);
setInterval(loadStatus,5000);
</script>
</body></html>)HTMLPAGE";
