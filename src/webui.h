// webui.h — single-page config UI served from PROGMEM
//
// Tabs: shared Status/WiFi/Display/Update plus the Clock and Usage feature tabs.
// The config JSON mirrors the nested Settings layout:
// { ..shared.., clockFace:{...}, usage:{...} }.
#pragma once
#include <Arduino.h>

static const char WEBUI_HTML[] PROGMEM = R"HTMLPAGE(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>SelfScreen</title>
<link rel="icon" type="image/png" href="data:image/png;base64,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">
<style>
:root{--bg:#0e1116;--card:#171c24;--mut:#8b96a5;--fg:#e6edf3;--acc:#3fb950;--acc2:#2f81f7;--red:#f85149;--bd:#262d38}
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif;background:var(--bg);color:var(--fg);font-size:15px}
header{padding:14px 16px;border-bottom:1px solid var(--bd);display:flex;align-items:center;gap:10px}
header h1{font-size:17px;margin:0;font-weight:600}
header .dot{width:9px;height:9px;border-radius:50%;background:var(--mut)}
header .dot.ok{background:var(--acc)}
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
<header><span id="dot" class="dot"></span><h1>SelfScreen</h1><span id="chip" class="chip" style="display:none"></span><span id="hi" class="muted"></span></header>
<nav>
 <button data-t="status" class="active">Status</button>
 <button data-t="wifi">WiFi</button>
 <button data-t="display">Display</button>
 <button data-t="clock">Clock</button>
 <button data-t="weather">Weather</button>
 <button data-t="usage">Usage</button>
 <button data-t="update">Update</button>
</nav>
<main>
 <!-- STATUS -->
 <section id="status" class="tab active">
  <div class="card"><h2>Device</h2><div id="statusBox" class="muted">Loading...</div></div>
 </section>

 <!-- WIFI -->
 <section id="wifi" class="tab">
  <div class="card"><h2>Saved networks</h2>
   <button class="btn sec" onclick="scan()">Scan networks</button>
   <div id="scanList"></div>
   <table id="wifiTable"></table>
   <button class="btn sec" style="margin-top:10px" onclick="addWifi()">+ Add network</button>
   <div style="margin-top:14px"><button class="btn" onclick="saveWifi()">Save &amp; connect (reboots)</button></div>
   <small class="hint">2.4&nbsp;GHz only. Up to 4 networks; at boot the device joins the strongest one it can see. Tap a scan result to fill a row. Leave a password blank to keep the stored one.</small>
  </div>
  <div class="card"><h2>Device name</h2>
   <label>Hostname</label><input id="hostname" type="text" placeholder="smalltv">
   <small class="hint">Reachable as <code>http://&lt;hostname&gt;.local</code> via mDNS. Running several SelfScreens? Give each its own name (<code>selfscreen-desk</code>, <code>selfscreen-shelf</code>) so browsers and the clawdmeter daemon's <code>--push-to</code> reach the right device. Saving a new name reboots the device.</small>
  </div>
  <div class="card"><h2>Setup hotspot (AP)</h2>
   <label>AP name</label><input id="apSsid" type="text">
   <label>AP password <span class="muted">(blank = open, else min 8 chars)</span></label>
   <input id="apPass" type="text" placeholder="(unchanged)">
   <small class="hint">The AP appears when no WiFi is configured or the connection fails.</small>
  </div>
 </section>

 <!-- DISPLAY (shared) -->
 <section id="display" class="tab">
  <div class="card"><h2>Mode</h2>
   <label>What this device shows</label>
   <select id="mode" onchange="modeChanged()">
    <option value="clock">Clock</option>
    <option value="weather">Weather</option>
    <option value="usage">Claude usage</option>
    <option value="carousel">Carousel (rotate modes)</option>
   </select>
   <div id="carouselRow">
    <label>Switch mode every (s)</label><input id="carouselSec" type="number" min="5" max="3600">
    <div class="chk"><input id="carouselClock" type="checkbox"><label>Clock</label></div>
    <div class="chk"><input id="carouselWeather" type="checkbox"><label>Weather</label></div>
    <div class="chk"><input id="carouselUsage" type="checkbox"><label>Claude usage</label></div>
   </div>
   <small class="hint">Pick the active feature, then configure it in its own tab. Carousel rotates through the ticked features.</small>
  </div>
  <div class="card"><h2>Screen</h2>
   <label>Brightness: <span id="brVal"></span>%</label>
   <input id="brightness" type="range" min="0" max="100" oninput="brVal.textContent=this.value">
   <div class="chk"><input id="autoBrightness" type="checkbox"><label>Auto-brightness (light sensor on A0)</label></div>
   <label>Orientation</label>
   <select id="rotation"><option value="0">0&deg;</option><option value="1">90&deg;</option>
    <option value="2">180&deg;</option><option value="3">270&deg;</option></select>
   <div class="chk"><input id="backlightInverted" type="checkbox"><label>Backlight is active-low (try if screen stays dark)</label></div>
  </div>
  <div class="card"><h2>Clock &amp; night mode</h2>
   <label>Timezone</label>
   <select id="tz"></select>
   <div class="muted" id="clockNow" style="margin:8px 0">Clock: -</div>
   <div class="chk"><input id="nightEnabled" type="checkbox"><label>Dim or blank the screen on a nightly schedule</label></div>
   <div class="row">
    <div><label>From</label><input id="nightStart" type="time"></div>
    <div><label>To</label><input id="nightEnd" type="time"></div>
   </div>
   <label>Night brightness: <span id="nlVal"></span>% <span class="muted">(0 = screen off)</span></label>
   <input id="nightLevel" type="range" min="0" max="100" oninput="nlVal.textContent=this.value">
   <small class="hint">Needs internet once to set the clock over NTP (no on-screen clock, this just drives the schedule). While the window is active it overrides the brightness and auto-brightness above. Times are local to the selected timezone; DST is handled automatically. After a reboot the schedule resumes once the clock re-syncs, so the screen may show normal brightness for a few seconds.</small>
  </div>
 </section>

 <!-- CLOCK (feature) -->
 <section id="clock" class="tab">
  <div class="card"><h2>Clock face</h2>
   <div class="chk"><input id="clk24" type="checkbox"><label>24-hour time (off = 12-hour with AM/PM)</label></div>
   <div class="chk"><input id="clkSeconds" type="checkbox"><label>Show seconds</label></div>
   <div class="chk"><input id="clkWeekday" type="checkbox"><label>Show weekday (Montag, Dienstag, ...)</label></div>
   <label>Date format</label>
   <select id="clkDateFmt">
    <option value="4">5. August 2026 (Deutsch)</option>
    <option value="0">DD.MM.YYYY</option>
    <option value="1">YYYY-MM-DD</option>
    <option value="2">DD.MM</option>
    <option value="3">Off (no date)</option>
   </select>
   <label>Element sizes</label>
   <div class="row">
    <div><label class="muted">Time <span id="clkTimeSizeV"></span></label>
     <input id="clkTimeSize" type="range" min="0" max="6" oninput="clkTimeSizeV.textContent=(+this.value+1)+'/7'"></div>
    <div><label class="muted">Weekday <span id="clkWeekdaySizeV"></span></label>
     <input id="clkWeekdaySize" type="range" min="0" max="5" oninput="clkWeekdaySizeV.textContent=(+this.value+1)+'/6'"></div>
    <div><label class="muted">Date <span id="clkDateSizeV"></span></label>
     <input id="clkDateSize" type="range" min="0" max="5" oninput="clkDateSizeV.textContent=(+this.value+1)+'/6'"></div>
   </div>
   <label>Element colours</label>
   <div class="row">
    <div><label class="muted">Time</label>
     <select id="clkTimeColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
    <div><label class="muted">Weekday</label>
     <select id="clkWeekdayColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
   </div>
   <div class="row">
    <div><label class="muted">Date</label>
     <select id="clkDateColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
    <div><label class="muted">Separator line</label>
     <select id="clkLineColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
   </div>
   <small class="hint">Every element is independent: drag a slider for its size and pick its colour. The clock is driven by NTP, which the device syncs automatically whenever the clock face is shown (or night mode is on). Set your timezone in the <b>Display</b> tab. Until the first sync lands the screen shows <code>--:--</code>.</small>
  </div>
 </section>

 <!-- WEATHER (feature) -->
 <section id="weather" class="tab">
  <div class="card"><h2>Location &amp; data</h2>
   <div class="row">
    <div><label>Latitude</label><input id="wxLat" type="number" step="0.01" min="-90" max="90"></div>
    <div><label>Longitude</label><input id="wxLon" type="number" step="0.01" min="-180" max="180"></div>
   </div>
   <label>Units</label>
   <select id="wxUnit"><option value="0">&deg;C (Celsius)</option><option value="1">&deg;F (Fahrenheit)</option></select>
   <label>Refresh weather (s)</label><input id="wxRefresh" type="number" min="60" max="21600">
   <small class="hint">Data from <a href="https://open-meteo.com" target="_blank">Open-Meteo</a> over plain HTTP (no API key). Default location is Hamburg (53.55, 9.99). Condition text is German. Until the first fetch lands the screen shows <code>--</code>.</small>
  </div>
  <div class="card"><h2>Elements</h2>
   <div class="chk"><input id="wxShowTemp" type="checkbox"><label>Temperature</label></div>
   <div class="chk"><input id="wxShowCond" type="checkbox"><label>Condition text (Klar, Bewoelkt, Regen, ...)</label></div>
   <div class="chk"><input id="wxShowPrecip" type="checkbox"><label>Precipitation (mm)</label></div>
   <div class="chk"><input id="wxShowTrend" type="checkbox"><label>12h temperature trend (sparkline)</label></div>
   <label>Element sizes</label>
   <div class="row">
    <div><label class="muted">Temperature <span id="wxTempSizeV"></span></label>
     <input id="wxTempSize" type="range" min="0" max="6" oninput="wxTempSizeV.textContent=(+this.value+1)+'/7'"></div>
    <div><label class="muted">Condition <span id="wxCondSizeV"></span></label>
     <input id="wxCondSize" type="range" min="0" max="5" oninput="wxCondSizeV.textContent=(+this.value+1)+'/6'"></div>
    <div><label class="muted">Precip <span id="wxPrecipSizeV"></span></label>
     <input id="wxPrecipSize" type="range" min="0" max="5" oninput="wxPrecipSizeV.textContent=(+this.value+1)+'/6'"></div>
   </div>
   <label>Element colours</label>
   <div class="row">
    <div><label class="muted">Temperature</label>
     <select id="wxTempColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
    <div><label class="muted">Condition</label>
     <select id="wxCondColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
   </div>
   <div class="row">
    <div><label class="muted">Precipitation</label>
     <select id="wxPrecipColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
    <div><label class="muted">Trend line</label>
     <select id="wxTrendColor">
      <option value="0">White</option><option value="1">Teal</option><option value="2">Green</option>
      <option value="3">Yellow</option><option value="4">Red</option><option value="5">Blue</option><option value="6">Gray</option>
      <option value="7">Self-Blau (hell)</option><option value="8">Self-Blau (mittel)</option>
     </select></div>
   </div>
   <small class="hint">Every element is independent: toggle it, drag its size slider and pick its colour. Temperature uses the big number font (with a drawn &deg; ring); condition and precipitation use the proportional font.</small>
  </div>
 </section>

 <!-- USAGE (feature) -->
 <section id="usage" class="tab">
  <div class="card"><h2>Claude usage</h2>
   <label>Usage daemon URL</label>
   <input id="usageUrl" type="url" placeholder="http://192.168.1.10:8787/">
   <label>Refresh data (s)</label><input id="usagePollSec" type="number" min="10" max="3600">
   <small class="hint">Runs on the PC-side <a href="https://github.com/giovi321/clawdmeter-daemon" target="_blank">clawdmeter-daemon</a>, which reads your Claude usage and sends it here. <b>Pull:</b> set the Usage URL to the daemon. <b>Push:</b> leave it blank and run the daemon with <code>--push-to &lt;hostname&gt;.local</code> (for networks where the device cannot reach the PC). Running several SelfScreens? Give each a unique hostname in the WiFi tab so every PC pushes to its own device. Idle animation plays until data arrives.</small>
  </div>
 </section>

 <!-- UPDATE -->
 <section id="update" class="tab">
  <div class="card"><h2>Update from GitHub</h2>
   <div class="muted">Installed: <b id="fwVer">-</b></div>
   <div style="margin-top:10px">
    <button class="btn sec" onclick="checkUpdate()" id="chkBtn">Check for latest</button>
    <button class="btn" style="margin-left:8px" onclick="selfUpdate()" id="ghUpBtn" disabled>Update now</button>
   </div>
   <div id="ghMsg" class="muted" style="margin-top:8px"></div>
   <small class="hint">Pulls the newest release straight from <a id="repoLink" href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">the GitHub repo</a>. HTTPS OTA is tight on the ESP8266; if it fails, use the manual upload below.</small>
  </div>
  <div class="card"><h2>Manual update (OTA)</h2>
   <input id="fw" type="file" accept=".bin">
   <div style="margin-top:12px"><button class="btn" onclick="upload()" id="upBtn">Upload &amp; flash</button></div>
   <div class="bar"><div id="upBar"></div></div>
   <div id="upMsg" class="muted" style="margin-top:8px"></div>
   <small class="hint">Upload a firmware.bin from the <a href="https://github.com/s3lfcod3r/SelfScreen/releases" target="_blank">releases page</a> or a local build. The device reboots when done.</small>
  </div>
  <div class="card"><h2>Settings backup</h2>
   <button class="btn sec" onclick="location.href='/api/export'">Export settings</button>
   <input id="cfgFile" type="file" accept=".json,application/json" style="margin-top:10px">
   <div style="margin-top:10px"><button class="btn" onclick="importCfg()">Import &amp; reboot</button></div>
   <small class="hint">The export is the device's <code>config.json</code>, including WiFi passwords in clear text; treat the file accordingly. Import applies everything and reboots.</small>
  </div>
  <div class="card"><h2>Maintenance</h2>
   <button class="btn sec" onclick="reboot()">Reboot</button>
   <button class="btn danger" style="margin-left:8px" onclick="factory()">Factory reset</button>
  </div>
 </section>
</main>

<div style="text-align:center;padding:0 0 16px"><button class="btn" onclick="saveAll()">Save settings</button></div>
<div style="text-align:center;padding:0 0 24px;font-size:12px">
 <a id="footRepo" href="https://github.com/s3lfcod3r/SelfScreen" target="_blank" style="color:var(--acc2);text-decoration:none">GitHub: s3lfcod3r/SelfScreen</a>
 <span class="muted"> · based on smalltv-mod</span>
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
 // clock face slice
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
 sv('wxUnit',wx.unitF?1:0); sv('wxRefresh',wx.refreshSec!=null?wx.refreshSec:600);
 sc('wxShowTemp',wx.showTemp!==false); sc('wxShowCond',wx.showCond!==false);
 sc('wxShowPrecip',wx.showPrecip!==false); sc('wxShowTrend',wx.showTrend!==false);
 sv('wxTempColor',wx.tempColor!=null?wx.tempColor:0);
 sv('wxCondColor',wx.condColor!=null?wx.condColor:7);
 sv('wxPrecipColor',wx.precipColor!=null?wx.precipColor:6);
 sv('wxTrendColor',wx.trendColor!=null?wx.trendColor:1);
 var _wsz=function(id,v,dv){var e=$(id);if(!e)return;e.value=(v!=null?v:dv);e.dispatchEvent(new Event('input'));};
 _wsz('wxTempSize',wx.tempSize,5);_wsz('wxCondSize',wx.condSize,4);_wsz('wxPrecipSize',wx.precipSize,2);
 // usage slice
 sv('usageUrl',u.usageUrl);
 sv('usagePollSec',u.pollSec);
 var ap=$('apPass'); if(ap)ap.placeholder=c.apPassSet?'(unchanged)':'(open)';
})}

function esc(s){return (''+(s==null?'':s)).replace(/[<>&"]/g,function(c){return {'<':'&lt;','>':'&gt;','&':'&amp;','"':'&quot;'}[c]})}

function collect(){
 var o={mode:gv('mode'),
  carouselSec:parseInt(gv('carouselSec'))||30,
  carouselClock:gc('carouselClock'),
  carouselWeather:gc('carouselWeather'),
  carouselUsage:gc('carouselUsage'),
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
  unitF:gv('wxUnit')==='1',refreshSec:parseInt(gv('wxRefresh'))||600,
  showTemp:gc('wxShowTemp'),showCond:gc('wxShowCond'),showPrecip:gc('wxShowPrecip'),showTrend:gc('wxShowTrend'),
  tempSize:parseInt(gv('wxTempSize'))||0,condSize:parseInt(gv('wxCondSize'))||0,precipSize:parseInt(gv('wxPrecipSize'))||0,
  tempColor:parseInt(gv('wxTempColor'))||0,condColor:parseInt(gv('wxCondColor'))||0,
  precipColor:parseInt(gv('wxPrecipColor'))||0,trendColor:parseInt(gv('wxTrendColor'))||0};
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
 .then(function(r){toast(r.reboot?'Saved — rebooting...':'Saved');if(r.reboot)setTimeout(function(){location.reload()},6000);loadStatus()})}

function saveWifi(){
 var o={wifi:collectWifi()};
 j('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(o)}).then(function(){
  toast('Saved, rebooting to connect...');j('/api/reboot',{method:'POST'});
 });
}

// wifi networks (up to 4)
function renderWifi(arr){var t=$('wifiTable');if(!t)return;t.innerHTML='';arr.forEach(addWifiRow);if(!arr.length)addWifiRow({})}
function addWifiRow(o){var t=$('wifiTable');var tr=document.createElement('tr');tr.className='symrow';
 tr.innerHTML='<td style="width:44%"><input class="ws" type="text" autocomplete="off" placeholder="SSID" value="'+esc(o.ssid||'')+'"></td>'+
  '<td><input class="wp" type="password" autocomplete="off" placeholder="'+(o.passSet?'(unchanged)':'password')+'"></td>'+
  '<td style="width:34px"><button class="btn sec" style="padding:6px 10px" onclick="this.closest(\'tr\').remove()">&times;</button></td>';
 t.appendChild(tr);}
function addWifi(){if(document.querySelectorAll('#wifiTable tr').length>=4){toast('Max 4');return}addWifiRow({})}
function collectWifi(){var w=[];document.querySelectorAll('#wifiTable tr').forEach(function(tr){
 var s=tr.querySelector('.ws').value.trim();if(!s)return;
 var e={ssid:s};var p=tr.querySelector('.wp').value;if(p)e.pass=p;w.push(e);});return w}
function scanPick(ssid){var rows=document.querySelectorAll('#wifiTable tr');var tr=null;
 for(var i=0;i<rows.length;i++){if(!rows[i].querySelector('.ws').value.trim()){tr=rows[i];break}}
 if(!tr){if(rows.length>=4){toast('Max 4');return}addWifiRow({});tr=$('wifiTable').lastChild}
 tr.querySelector('.ws').value=ssid;tr.querySelector('.wp').focus();}

// wifi scan
function scan(){$('scanList').innerHTML='<div class="muted">Scanning...</div>';
 j('/api/scan').then(function(l){var h='';l.sort(function(a,b){return b.rssi-a.rssi});
  l.forEach(function(n){h+='<div class="net" onclick="scanPick(this.dataset.s)" data-s="'+
   esc(n.ssid)+'"><span>'+(n.enc?'🔒 ':'')+esc(n.ssid)+'</span><span class="muted">'+n.rssi+' dBm</span></div>'});
  $('scanList').innerHTML=h||'<div class="muted">No networks found</div>';})}

// status
function loadStatus(){j('/api/status').then(function(s){
 $('dot').className='dot'+(s.connected?' ok':'');
 $('hi').textContent=s.mode==='ap'?'setup mode':(s.ip||'');
 var cn=$('clockNow'); if(cn){var ne=!!(C.clock&&C.clock.nightEnabled);var ns=s.night?'  · night mode active':(s.nightHeld?'  · night mode waiting for NTP':'');cn.textContent=!ne?'Clock: NTP runs only when night mode is on':('Clock: '+(s.synced?(s.time||'synced')+(s.tz?' ('+s.tz+')':''):'waiting for NTP...')+ns);}
 var fw=$('fwVer'); if(fw)fw.textContent=s.fw+' '+s.version;
 // Surface the result of a boot-time GitHub update (ESP8266) once on first load,
 // so a failure that happened across the reboot is visible even if the original
 // Update tab was closed. Don't clobber an in-progress check/update message.
 if(!window._otaShown){window._otaShown=1;var gm=$('ghMsg');if(gm&&!gm.textContent&&s.updateMsg&&s.updateMsg!=='updating...')gm.textContent='Last update: '+s.updateMsg}
 var fv=$('footVer'); if(fv)fv.textContent=' v'+s.version;
 if(s.repo){var rl=$('repoLink'); if(rl)rl.href=s.repo+'/releases'; var fr=$('footRepo'); if(fr)fr.href=s.repo;}
 $('statusBox').innerHTML=
  kv('Firmware',s.fw+' '+s.version)+kv('Mode',s.mode.toUpperCase())+
  kv('Network',s.ssid||'-')+kv('IP',s.ip||'-')+kv('mDNS','http://'+(C.hostname||'smalltv')+'.local')+
  kv('Signal',s.rssi?s.rssi+' dBm':'-')+
  kv('Free heap',s.heap+' B')+kv('Uptime',fmtUp(s.uptime))+kv('Last reset',s.reset||'-');
})}
function kv(k,v){return '<div class="kv"><span class="muted">'+k+'</span><b>'+v+'</b></div>'}
function fmtUp(s){var d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60);
 return (d?d+'d ':'')+(h?h+'h ':'')+m+'m'}

// GitHub self-update
function checkUpdate(){$('ghMsg').textContent='Checking GitHub...';$('chkBtn').disabled=true;
 j('/api/checkupdate').then(function(u){$('chkBtn').disabled=false;
  if(!u.ok){$('ghMsg').textContent='Check failed: '+(u.error||'unknown');return}
  if(u.newer){$('ghMsg').innerHTML='Version <b>'+u.latest+'</b> is available (installed '+u.current+').';$('ghUpBtn').disabled=false}
  else{$('ghMsg').textContent='Up to date ('+u.current+').';$('ghUpBtn').disabled=true}
 }).catch(function(){$('chkBtn').disabled=false;$('ghMsg').textContent='Check failed'})}
function selfUpdate(){if(!confirm('Download and flash the latest release from GitHub? The device reboots if it succeeds.'))return;
 $('ghUpBtn').disabled=true;$('chkBtn').disabled=true;
 $('ghMsg').textContent='Downloading and flashing... this can take a couple of minutes and the device may reboot twice.';
 // Installed version, read synchronously from the already-loaded status so the
 // poller below can recognise success (new version) without racing a fetch.
 var cur=(($('fwVer').textContent||'').trim().split(' ').pop())||'';
 j('/api/selfupdate',{method:'POST'}).then(function(){
  var n=0;var t=setInterval(function(){n++;
   j('/api/status').then(function(s){
    if(cur&&s.version&&s.version!==cur){clearInterval(t);$('ghMsg').textContent='Updated to '+s.version+'.';$('chkBtn').disabled=false;return}
    var m=s.updateMsg||'';
    if(m&&m!=='starting...'&&m!=='updating...'){clearInterval(t);$('ghMsg').textContent='Update failed: '+m;$('chkBtn').disabled=false}
   }).catch(function(){});
   if(n>100)clearInterval(t);
  },3000);
 }).catch(function(){$('ghMsg').textContent='Could not start update';$('chkBtn').disabled=false})}

// settings backup
function importCfg(){var f=$('cfgFile').files[0];if(!f){toast('Pick a config .json first');return}
 var r=new FileReader();
 r.onload=function(){var txt=r.result;
  try{JSON.parse(txt)}catch(e){toast('Not valid JSON');return}
  if(!confirm('Apply this configuration and reboot?'))return;
  j('/api/import',{method:'POST',headers:{'Content-Type':'application/json'},body:txt})
   .then(function(){toast('Imported, rebooting...');setTimeout(function(){location.reload()},8000)})
   .catch(function(){toast('Import failed')});
 };
 r.readAsText(f);}

// maintenance
function reboot(){if(confirm('Reboot device?'))j('/api/reboot',{method:'POST'}).then(function(){toast('Rebooting...')})}
function factory(){if(confirm('Erase ALL settings and reboot?'))j('/api/factory',{method:'POST'}).then(function(){toast('Reset, rebooting...')})}

// OTA
function upload(){var f=$('fw').files[0];if(!f){toast('Pick a .bin first');return}
 var fd=new FormData();fd.append('firmware',f,f.name);
 var x=new XMLHttpRequest();x.open('POST','/update');
 $('upBtn').disabled=true;
 x.upload.onprogress=function(e){if(e.lengthComputable){var p=Math.round(e.loaded/e.total*100);$('upBar').style.width=p+'%';$('upMsg').textContent='Uploading '+p+'%'}};
 x.onload=function(){$('upBtn').disabled=false;if(x.status==200){$('upMsg').textContent='Done. Rebooting...';$('upBar').style.width='100%';setTimeout(function(){location.reload()},9000)}else{$('upMsg').textContent='Failed: '+x.responseText}};
 x.onerror=function(){$('upBtn').disabled=false;$('upMsg').textContent='Upload error'};
 x.send(fd);
}

loadConfig().then(loadStatus);
setInterval(loadStatus,5000);
</script>
</body></html>)HTMLPAGE";
