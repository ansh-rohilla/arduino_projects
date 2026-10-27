#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ========== WIFI & CREDENTIALS ==========
#if __has_include("secrets.h")
#include "secrets.h"
#elif __has_include("secrets.example.h")
#include "secrets.example.h"
#else
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
#endif

// ========== DHT11 ==========
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ========== OLED ==========
#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET
);

WebServer server(80);

// ========== SENSOR DATA ==========
float temperature = NAN;
float humidity = NAN;
String roomStatus = "Starting";
unsigned long lastRead = 0;

// ========== PHONE-CONTROLLED SETTINGS ==========
float warningThreshold = 30.0;
bool alertsEnabled = true;
bool oledEnabled = true;
bool compactDisplay = false;

// ========== DASHBOARD ==========
const char webpage[] PROGMEM = R"ATMOSPHERE(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Atmosphere | Smart Room</title>
<style>
:root {
 --bg:#09111f;--panel:#142138;--border:#2a3a54;
 --text:#f1f5f9;--muted:#96a7bf;--blue:#60a5fa;
 --teal:#2dd4bf;--green:#4ade80;--amber:#fbbf24;--red:#fb7185;
}
*{box-sizing:border-box}
body{
 margin:0;padding:24px 15px 45px;color:var(--text);
 background:radial-gradient(ellipse at top,#1b2d4c,var(--bg) 65%);
 font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif
}
main{max-width:1000px;margin:auto}
header,.row,.heading{
 display:flex;align-items:center;justify-content:space-between;gap:12px
}
header{margin-bottom:24px;flex-wrap:wrap}
h1{font-size:29px;margin:0}
h2{font-size:18px;margin:0 0 15px}
h3{font-size:15px;margin:0}
p{color:var(--muted);font-size:13px}
.sub{color:var(--muted);font-size:12px;margin-top:6px}
.live{padding:9px 13px;border:1px solid var(--border);
 border-radius:30px;font-size:12px}
.dot{display:inline-block;width:8px;height:8px;border-radius:50%;
 background:var(--green);margin-right:7px;animation:pulse 1.6s infinite}
.dot.off{background:var(--red)}
@keyframes pulse{50%{opacity:.4;transform:scale(.8)}}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px}
.card{
 min-width:0;background:linear-gradient(145deg,#182942,#111c30);
 border:1px solid var(--border);border-radius:18px;padding:20px;
 margin-bottom:14px
}
.value{font-size:34px;font-weight:750;letter-spacing:-1px;margin:13px 0}
.blue{color:var(--blue)}.teal{color:var(--teal)}
.muted{color:var(--muted);font-size:12px}
.section{margin:27px 0 12px;font-size:11px;letter-spacing:2px;
 color:var(--muted);font-weight:700;text-transform:uppercase}
canvas{width:100%;height:175px;display:block}
.chartHead{display:flex;justify-content:space-between;gap:8px;align-items:start}
input[type=number],select{
 background:#0c1729;color:var(--text);border:1px solid #40516d;
 border-radius:10px;padding:11px;width:100%;font-size:15px
}
label{display:block;color:#d5deeb;font-size:13px;margin:15px 0 8px}
button{
 border:0;border-radius:10px;padding:12px 15px;cursor:pointer;
 background:#2563eb;color:white;font-size:14px;font-weight:650
}
button:active{transform:scale(.98)}
.secondary{background:#263852}
.danger{background:#9f334b}
.control{margin-bottom:16px}
.toggleRow{display:flex;justify-content:space-between;align-items:center;gap:12px}
.switch{position:relative;display:inline-block;width:48px;height:27px;flex-shrink:0}
.switch input{opacity:0;width:0;height:0}
.track{position:absolute;inset:0;border-radius:30px;background:#475569;cursor:pointer}
.track:before{content:"";position:absolute;width:19px;height:19px;
 left:4px;top:4px;background:white;border-radius:50%;transition:.2s}
.switch input:checked+.track{background:#16a34a}
.switch input:checked+.track:before{transform:translateX(21px)}
.full{width:100%}
.notice{border:1px solid var(--border);background:#0c1729;
 padding:12px;border-radius:10px;color:var(--muted);font-size:12px;
 line-height:1.6;margin-top:12px}
#message{min-height:20px;color:var(--teal);font-size:13px;margin-top:10px}
footer{color:var(--muted);font-size:12px;margin-top:24px}
@media(max-width:560px){
 body{padding:18px 12px 35px}h1{font-size:24px}
 .grid{grid-template-columns:1fr}.card{padding:17px}
 .value{font-size:31px}header{align-items:flex-start}
}
</style>
</head>
<body>
<main>
<header>
 <div>
  <h1>Atmosphere</h1>
  <div class="sub">Smart Room Environment Monitor</div>
 </div>
 <div class="live"><span id="dot" class="dot"></span><span id="conn">Connecting</span></div>
</header>

<div class="section">Live environment</div>
<div class="grid">
 <div class="card">
  <div class="muted">Temperature</div>
  <div class="value blue"><span id="temp">--</span> °C</div>
  <div class="muted">DHT11 sensor</div>
 </div>
 <div class="card">
  <div class="muted">Relative humidity</div>
  <div class="value teal"><span id="hum">--</span>%</div>
  <div class="muted">DHT11 sensor</div>
 </div>
</div>

<div class="card">
 <div class="row">
  <div><h3>Room status</h3><div class="sub">Based on your alert settings</div></div>
  <strong id="status">Waiting...</strong>
 </div>
 <p id="statusMsg">Waiting for sensor data</p>
</div>

<div class="section">Environmental history</div>
<div class="grid">
 <div class="card">
  <div class="chartHead"><h3>Temperature</h3><strong id="tNow" class="blue">--</strong></div>
  <canvas id="tc"></canvas>
  <div class="muted" id="tRange">Min -- / Max --</div>
 </div>
 <div class="card">
  <div class="chartHead"><h3>Humidity</h3><strong id="hNow" class="teal">--</strong></div>
  <canvas id="hc"></canvas>
  <div class="muted" id="hRange">Min -- / Max --</div>
 </div>
</div>

<div class="section">Phone control panel</div>
<div class="card">
 <h2>Temperature alerts</h2>
 <div class="control">
  <label for="threshold">Warning threshold (°C)</label>
  <input id="threshold" type="number" min="15" max="50" step="0.5" value="30">
  <div class="sub">Valid range: 15°C to 50°C</div>
 </div>
 <div class="toggleRow">
  <div><strong>Enable temperature alerts</strong><div class="sub">Show a warning when threshold is reached</div></div>
  <label class="switch"><input id="alerts" type="checkbox" checked><span class="track"></span></label>
 </div>
</div>

<div class="card">
 <h2>OLED display settings</h2>
 <div class="toggleRow">
  <div><strong>OLED display</strong><div class="sub">Turn the physical display on or off</div></div>
  <label class="switch"><input id="oled" type="checkbox" checked><span class="track"></span></label>
 </div>
 <label for="displayMode">Display layout</label>
 <select id="displayMode">
  <option value="normal">Normal — larger text</option>
  <option value="compact">Compact — extra information</option>
 </select>
 <button id="save" class="full" style="margin-top:18px">Apply settings to ESP32</button>
 <div id="message" role="status"></div>
</div>

<div class="card">
 <h2>Dashboard settings</h2>
 <label for="refresh">Webpage refresh interval</label>
 <select id="refresh">
  <option value="1000">1 second</option>
  <option value="2000" selected>2 seconds</option>
  <option value="5000">5 seconds</option>
  <option value="10000">10 seconds</option>
 </select>
 <button id="applyRefresh" class="full" style="margin-top:12px">Apply refresh interval</button>
 <div class="notice">Refresh interval changes how often this webpage requests data. The ESP32 sensor itself continues sampling approximately every 2 seconds.</div>
</div>

<div class="card">
 <h2>Chart history</h2>
 <div class="row">
  <div><strong id="samples">0 samples</strong><div class="sub">Stored in this browser tab only</div></div>
  <button id="reset" class="danger">Reset charts</button>
 </div>
 <div class="notice">Resetting clears the graphs in this browser. It does not erase sensor data stored elsewhere.</div>
</div>

<footer>Atmosphere · ESP32 + DHT11 · Local Wi-Fi control</footer>
</main>

<script>
const $ = id => document.getElementById(id);
let settings = {threshold:30,alerts:true,oled:true,compact:false};
let temps = [], hums = [];
let refreshMs = 2000, timer = null;

function message(text, bad=false) {
 $('message').textContent=text;
 $('message').style.color=bad?'#fb7185':'#2dd4bf';
}

function connection(ok) {
 $('conn').textContent=ok?'Device connected':'Reconnecting...';
 $('dot').classList.toggle('off',!ok);
}

function renderStatus(d) {
 let status, color, desc;
 if(settings.alerts && d.temperature >= settings.threshold) {
  status='WARNING'; color='#fb7185';
  desc='Temperature has reached your configured threshold.';
 } else if(d.temperature >= 30) {
  status='WARM'; color='#fbbf24'; desc='Temperature is 30°C or higher.';
 } else if(d.temperature < 18) {
  status='COOL'; color='#60a5fa'; desc='Temperature is below 18°C.';
 } else {
  status='NORMAL'; color='#4ade80'; desc='Temperature is below the warning threshold.';
 }
 $('status').textContent=status;
 $('status').style.color=color;
 $('statusMsg').textContent=settings.alerts?desc:
  'Temperature alerts are disabled.';
}

function draw(id, arr, color, isTemp) {
 const c=$(id), w=c.clientWidth, h=175;
 if(!w)return;
 const ratio=window.devicePixelRatio||1;
 c.width=w*ratio;c.height=h*ratio;
 const xctx=c.getContext('2d');
 xctx.setTransform(ratio,0,0,ratio,0,0);
 xctx.clearRect(0,0,w,h);
 const left=30,right=8,top=12,bottom=20;
 const cw=w-left-right,ch=h-top-bottom;
 let min,max;
 if(isTemp && arr.length) {
  min=Math.floor((Math.min(...arr)-2)/5)*5;
  max=Math.ceil((Math.max(...arr)+2)/5)*5;
  if(max-min<10){min-=5;max+=5;}
 } else {min=0;max=100;}
 const y=v=>top+ch-(v-min)/(max-min)*ch;
 xctx.font='10px sans-serif';xctx.textAlign='right';
 xctx.textBaseline='middle';xctx.strokeStyle='#293b55';
 xctx.fillStyle='#91a3bc';
 for(let i=0;i<=4;i++){
  let yy=top+ch*i/4;
  xctx.beginPath();xctx.moveTo(left,yy);xctx.lineTo(w-right,yy);xctx.stroke();
  xctx.fillText((max-(max-min)*i/4).toFixed(0),left-5,yy);
 }
 if(!arr.length)return;
 const x=i=>left+(arr.length===1?cw/2:i*cw/(arr.length-1));
 xctx.beginPath();
 arr.forEach((v,i)=>i?xctx.lineTo(x(i),y(v)):xctx.moveTo(x(i),y(v)));
 xctx.lineTo(x(arr.length-1),top+ch);xctx.lineTo(x(0),top+ch);
 xctx.closePath();xctx.globalAlpha=.13;xctx.fillStyle=color;xctx.fill();xctx.globalAlpha=1;
 xctx.beginPath();
 arr.forEach((v,i)=>i?xctx.lineTo(x(i),y(v)):xctx.moveTo(x(i),y(v)));
 xctx.strokeStyle=color;xctx.lineWidth=2.5;xctx.lineJoin='round';xctx.stroke();
 xctx.beginPath();xctx.arc(x(arr.length-1),y(arr[arr.length-1]),3.5,0,Math.PI*2);
 xctx.fillStyle=color;xctx.fill();
}

function updateCharts(){
 draw('tc',temps,'#60a5fa',true);
 draw('hc',hums,'#2dd4bf',false);
 if(temps.length) $('tRange').textContent=
  'Min '+Math.min(...temps).toFixed(1)+' / Max '+Math.max(...temps).toFixed(1);
 if(hums.length) $('hRange').textContent=
  'Min '+Math.min(...hums).toFixed(1)+' / Max '+Math.max(...hums).toFixed(1);
 $('samples').textContent=temps.length+' samples';
}

async function loadSettings(){
 try {
  const r=await fetch('/settings',{cache:'no-store'});
  if(!r.ok)throw Error('Settings unavailable');
  settings=await r.json();
  $('threshold').value=settings.threshold;
  $('alerts').checked=settings.alerts;
  $('oled').checked=settings.oled;
  $('displayMode').value=settings.compact?'compact':'normal';
 } catch(e) { message('Could not load device settings.',true); }
}

async function applySettings(){
 const threshold=Number($('threshold').value);
 if(!Number.isFinite(threshold)||threshold<15||threshold>50){
  message('Enter a threshold between 15 and 50°C.',true);return;
 }
 const body=new URLSearchParams({
  threshold:String(threshold),
  alerts:$('alerts').checked?'1':'0',
  oled:$('oled').checked?'1':'0',
  compact:$('displayMode').value==='compact'?'1':'0'
 });
 try {
  const r=await fetch('/settings',{
   method:'POST',
   headers:{'Content-Type':'application/x-www-form-urlencoded'},
   body
  });
  const data=await r.json();
  if(!r.ok)throw Error(data.error||'Settings rejected');
  settings=data;
  message('Settings applied successfully.');
  if(Number.isFinite(lastTemp)) renderStatus({temperature:lastTemp});
 } catch(e) { message('Failed to apply settings: '+e.message,true); }
}

let lastTemp=NaN;
async function refreshData(){
 try {
  const r=await fetch('/data',{cache:'no-store'});
  if(!r.ok)throw Error('Data unavailable');
  const d=await r.json();
  if(!Number.isFinite(Number(d.temperature))||
     !Number.isFinite(Number(d.humidity)))throw Error('Invalid readings');
  lastTemp=Number(d.temperature);
  const t=lastTemp,h=Number(d.humidity);
  $('temp').textContent=t.toFixed(1);
  $('hum').textContent=h.toFixed(1);
  $('tNow').textContent=t.toFixed(1)+'°C';
  $('hNow').textContent=h.toFixed(1)+'%';
  temps.push(t);hums.push(h);
  if(temps.length>120)temps.shift();
  if(hums.length>120)hums.shift();
  renderStatus({temperature:t});
  connection(true);
  updateCharts();
 } catch(e) { connection(false); }
}

function setRefresh(){
 refreshMs=Number($('refresh').value)||2000;
 if(timer)clearInterval(timer);
 timer=setInterval(refreshData,refreshMs);
}

$('save').addEventListener('click',applySettings);
$('applyRefresh').addEventListener('click',()=>{
 setRefresh();message('Dashboard refresh interval updated.');
});
$('reset').addEventListener('click',()=>{
 temps=[];hums=[];updateCharts();message('Chart history cleared.');
});
window.addEventListener('resize',updateCharts);

loadSettings().then(()=>refreshData());
setRefresh();
</script>
</body>
</html>
)ATMOSPHERE";

// ========== STATUS ==========
String getRoomStatus() {
  if (alertsEnabled && !isnan(temperature) &&
      temperature >= warningThreshold) return "WARNING";
  if (isnan(temperature)) return "WAITING";
  if (temperature >= 30.0) return "WARM";
  if (temperature < 18.0) return "COOL";
  return "NORMAL";
}

// ========== OLED ==========
void updateOLED() {
  if (!oledEnabled) {
    display.clearDisplay();
    display.display();
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (compactDisplay) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ATMOSPHERE");
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    display.setCursor(0, 16);
    display.print("Temp: ");
    if (isnan(temperature)) display.print("--");
    else display.print(temperature, 1);
    display.println(" C");

    display.setCursor(0, 29);
    display.print("Hum: ");
    if (isnan(humidity)) display.print("--");
    else display.print(humidity, 1);
    display.println("%");

    display.setCursor(0, 43);
    display.print("Limit: ");
    display.print(warningThreshold, 1);
    display.println(" C");

    display.setCursor(0, 55);
    display.print("Status: ");
    display.print(getRoomStatus());
  } else {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ROOM ENVIRONMENT");
    display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

    display.setCursor(0, 19);
    display.print("Temp: ");
    if (isnan(temperature)) display.print("--");
    else display.print(temperature, 1);
    display.println(" C");

    display.setCursor(0, 35);
    display.print("Humidity: ");
    if (isnan(humidity)) display.print("--");
    else display.print(humidity, 1);
    display.println("%");

    display.drawLine(0, 49, 127, 49, SSD1306_WHITE);
    display.setCursor(0, 54);
    display.print(getRoomStatus());
  }

  display.display();
}

// ========== JSON HELPERS ==========
void sendSettings() {
  String json = "{\"threshold\":";
  json += String(warningThreshold, 1);
  json += ",\"alerts\":";
  json += alertsEnabled ? "true" : "false";
  json += ",\"oled\":";
  json += oledEnabled ? "true" : "false";
  json += ",\"compact\":";
  json += compactDisplay ? "true" : "false";
  json += "}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

// ========== WEB ROUTES ==========
void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html", webpage);
}

void handleData() {
  if (isnan(temperature) || isnan(humidity)) {
    server.send(503, "application/json",
                "{\"error\":\"Sensor readings unavailable\"}");
    return;
  }

  String json = "{\"temperature\":";
  json += String(temperature, 1);
  json += ",\"humidity\":";
  json += String(humidity, 1);
  json += ",\"status\":\"";
  json += getRoomStatus();
  json += "\"}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void handleGetSettings() {
  sendSettings();
}

void handlePostSettings() {
  // Validate all fields before changing any settings.
  if (!server.hasArg("threshold") ||
      !server.hasArg("alerts") ||
      !server.hasArg("oled") ||
      !server.hasArg("compact")) {
    server.send(400, "application/json",
                "{\"error\":\"Missing setting\"}");
    return;
  }

  String thresholdArg = server.arg("threshold");
  String alertsArg = server.arg("alerts");
  String oledArg = server.arg("oled");
  String compactArg = server.arg("compact");

  if (thresholdArg.length() == 0 || thresholdArg.length() > 6 ||
      (alertsArg != "0" && alertsArg != "1") ||
      (oledArg != "0" && oledArg != "1") ||
      (compactArg != "0" && compactArg != "1")) {
    server.send(400, "application/json",
                "{\"error\":\"Invalid setting format\"}");
    return;
  }

  char* endPtr;
  float newThreshold = strtof(thresholdArg.c_str(), &endPtr);

  if (endPtr == thresholdArg.c_str() || *endPtr != '\0' ||
      !isfinite(newThreshold) ||
      newThreshold < 15.0 || newThreshold > 50.0) {
    server.send(400, "application/json",
                "{\"error\":\"Threshold must be 15 to 50 C\"}");
    return;
  }

  warningThreshold = newThreshold;
  alertsEnabled = (alertsArg == "1");
  oledEnabled = (oledArg == "1");
  compactDisplay = (compactArg == "1");

  updateOLED();
  sendSettings();
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED initialization failed!");
    while (true) delay(1000);
  }

  dht.begin();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("ATMOSPHERE");
  display.setCursor(0, 28);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi...");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.on("/settings", HTTP_GET, handleGetSettings);
    server.on("/settings", HTTP_POST, handlePostSettings);

    server.onNotFound([]() {
      server.send(404, "text/plain", "Not found");
    });

    server.begin();
    Serial.println("Web server started.");
  } else {
    Serial.println("\nWi-Fi failed. Check credentials.");
  }

  lastRead = millis() - 2000;
}

// ========== MAIN LOOP ==========
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }

  if (lastRead == 0 || millis() - lastRead >= 2000) {
    lastRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      temperature = t;
      humidity = h;

      Serial.printf(
        "Temperature: %.1f C | Humidity: %.1f %% | Status: %s\n",
        temperature, humidity, getRoomStatus().c_str()
      );

      updateOLED();
    } else {
      Serial.println("DHT11 read failed; retaining last valid readings.");
    }
  }

  delay(2);
}
