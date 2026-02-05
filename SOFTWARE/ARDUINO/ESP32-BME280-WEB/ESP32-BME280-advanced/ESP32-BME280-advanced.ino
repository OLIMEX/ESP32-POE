/*
  =====================================================================
  PROJECT: ESP32 Real-Time Environmental Dashboard (Web Graph Station)
  =====================================================================

  This project turns an ESP32 board into a standalone WiFi
  environmental monitoring station with an advanced real-time
  graphical web dashboard.

  The ESP32 reads data from a BME280 environmental sensor and hosts
  a live interactive website — no cloud, no external server, no PC
  software required.

  Everything runs directly on the ESP32.

  =====================================================================
  WHAT THE DEVICE MEASURES

      • Temperature (°C or °F)
      • Relative Humidity (%)
      • Air Pressure (hPa or PSIA)

  =====================================================================
  WHAT YOU SEE IN THE WEB BROWSER

  When you open the ESP32's IP address, you get a full dashboard with:

      • Live sensor values (big numbers)
      • Scrolling historical graphs
      • Mouse hover crosshair with time + value
      • Real clock time OR elapsed time display
      • Min/Max markers automatically detected
      • Grid and axis label toggles
      • Unit switching (°C/°F, hPa/PSIA)
      • Custom graph colors

  Important:
  The ESP32 ONLY sends sensor data.
  ALL graphs and UI are drawn inside your browser using JavaScript.

  =====================================================================
  SUPPORTED ESP32 BOARDS (AUTO PIN CONFIGURATION)

      • Olimex ESP32-POE
      • Olimex ESP32-POE-ISO
      • Olimex ESP32-POE2
      • Olimex ESP32-EVB
      • Olimex ESP32-C3-DevKit-Lipo

  The sketch automatically selects the correct I2C pins depending on
  the board you choose in Arduino IDE.

  If you select the wrong board → wrong pins → sensor not detected.

  =====================================================================
  SENSOR USED

      Olimex MOD-BME280
      Measures temperature, humidity, and pressure via I2C.

  =====================================================================
  SOFTWARE YOU MUST INSTALL (ONE TIME)

  1) Install Arduino IDE
     https://www.arduino.cc/en/software

  2) Install ESP32 board support:
     https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html

     After installation:
     Tools → Board → Select your ESP32 board

  3) Install required libraries:
     Sketch → Include Library → Manage Libraries

     Install these:

        • ESPAsyncWebServer
        • AsyncTCP
        • Adafruit BME280
        • Adafruit Unified Sensor

  =====================================================================
  SPECIAL NOTE FOR ESP32-C3-DevKit-Lipo

  In Arduino IDE:
      Tools → "USB CDC On Boot" → ENABLE

  Without this, the Serial Monitor will not work.

  =====================================================================
  HARDWARE CONNECTION (I2C SENSOR)

  - ESP32-POE / POE2 / POE-ISO / EVB
      Just plug MOD-BME280 into the UEXT connector.

  - ESP32-C3-DevKit-Lipo (NO UEXT connector!)
      Use wires:

         BME280  →  ESP32-C3
         3.3V    →  3V3
         GND     →  GND
         SDA     →  GPIO 8
         SCL     →  GPIO 9

  =====================================================================
  HOW THE SYSTEM WORKS (STEP-BY-STEP)

  1. ESP32 starts
  2. I2C bus is initialized
  3. BME280 sensor is detected
  4. ESP32 connects to your WiFi
  5. ESP32 starts a web server
  6. Your browser connects to the ESP32
  7. ESP32 sends new sensor data every 2 seconds
  8. Browser updates graphs in real time

  =====================================================================
  HOW TO USE (BEGINNER GUIDE)

  STEP 1 — Enter your WiFi credentials in this code:
      #define WIFI_SSID     "YourWiFiName"
      #define WIFI_PASSWORD "YourWiFiPassword"

  STEP 2 — Upload the sketch to your ESP32

  STEP 3 — Open Serial Monitor
      Tools → Serial Monitor
      Baud rate: 115200

  STEP 4 — Wait for connection
      You will see:
          WiFi Connected!
          IP Address: 192.168.x.xxx

  STEP 5 — Open a web browser and type:
      http://192.168.x.xxx

  That page is hosted directly by your ESP32.

  =====================================================================
  TROUBLESHOOTING

  "BME280 NOT FOUND!"
      → Wrong board selected
      → Wrong wiring (C3 board)
      → Sensor not connected

  No IP address appears
      → Wrong WiFi password
      → Router too far away

  Page opens but no graphs
      → Refresh browser
      → Check Serial Monitor for sensor data

  =====================================================================
  BEGINNER NOTES

  • No Internet is required after WiFi connection.
  • Data resets every time ESP32 restarts.
  • Browser does all graph drawing, ESP32 only sends numbers.
  • You can open the dashboard on phone, tablet, or PC.

  =====================================================================
*/


#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

/* ================= USER WIFI SETTINGS ================= */

#define WIFI_SSID     "WIFI-NAME"
#define WIFI_PASSWORD "WIFI-PASSWORD"

/* ============================================================
   I2C PIN AUTO-CONFIGURATION BASED ON SELECTED BOARD
   ============================================================ */

#if defined(ARDUINO_ESP32_POE) || defined(ARDUINO_ESP32_POE_ISO) || defined(ARDUINO_ESP32_POE2)
// Olimex ESP32-POE / POE-ISO / POE2 (UEXT boards)

  #define SDA_PIN 13
  #define SCL_PIN 16
  #define BOARD_NAME "ESP32-POE family"

#elif defined(ARDUINO_ESP32_EVB)
// Olimex ESP32-EVB

  #define SDA_PIN 13
  #define SCL_PIN 16
  #define BOARD_NAME "ESP32-EVB"

#elif defined(ARDUINO_ESP32C3_DEVKIT_LIPO)
// Olimex ESP32-C3-DevKit-Lipo

  #define SDA_PIN 8
  #define SCL_PIN 9
  #define BOARD_NAME "ESP32-C3-DevKit-Lipo"

#else
  #warning "Board not recognized — using default I2C pins!"
  #define SDA_PIN 21
  #define SCL_PIN 22
  #define BOARD_NAME "Unknown board (default pins)"
#endif

#define BME_ADDR 0x77


Adafruit_BME280 bme;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

/* ===================== WEB PAGE ===================== */
String webpage(){
return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
:root{ --bg:#0e0e0e; --panel:#181818; --text:#ffffff;}
body{margin:0;background:var(--bg);color:var(--text);font-family:Arial;overflow:hidden}
#settings{position:fixed;left:0;top:0;height:100%;width:260px;background:var(--panel);padding:12px;border-right:2px solid #333;box-sizing:border-box;}
.ctrl{width:100%;margin:4px 0;height:24px}
.section{margin-bottom:10px}
#divider{position:fixed;left:260px;top:0;height:100%;width:10px;background:#333;cursor:pointer;display:flex;align-items:center;justify-content:center}
#content{margin-left:280px;height:100vh;display:flex;flex-direction:column;justify-content:space-evenly;padding:8px;box-sizing:border-box;}
.card{background:#1a1a1a;border-radius:10px;flex:1;margin:6px 0;padding:6px;display:flex;flex-direction:column}
.title{text-align:center;font-size:15px}
.value{text-align:center;font-size:20px;font-weight:bold}
.graphWrap{position:relative;flex:1}
canvas{position:absolute;left:0;top:0;width:100%;height:100%;border-radius:6px}
.base{background:#111;z-index:0}
.overlay{z-index:1}
</style>
</head>
<body>
<div id="settings">
<h3>Settings</h3>
<div class="section">
<label>Time mode
<select id="timeMode" class="ctrl">
<option value="clock" selected>Real</option>
<option value="elapsed">Elapsed</option>
</select></label>
</div>
<div class="section">
<label>Master Grid<select id="gridMaster" class="ctrl"><option selected>on</option><option>off</option></select></label>
<label>Show Y labels<select id="yLabels" class="ctrl"><option selected>on</option><option>off</option></select></label>
<label>Show X labels<select id="xLabels" class="ctrl"><option selected>on</option><option>off</option></select></label>
<label>Min/Max markers<select id="extrema" class="ctrl"><option selected>on</option><option>off</option></select></label>
</div>
<div class="section">
<label>Temp unit<select id="tempUnit" class="ctrl"><option value="C" selected>°C</option><option value="F">°F</option></select></label>
<label>Pressure unit<select id="presUnit" class="ctrl"><option value="hPa" selected>hPa</option><option value="PSIA">PSIA</option></select></label>
</div>
<div class="section">
<label>Temp color<input type="color" id="cTemp" class="ctrl" value="#ff5050"></label>
<label>Hum color<input type="color" id="cHum" class="ctrl" value="#4da6ff"></label>
<label>Pres color<input type="color" id="cPres" class="ctrl" value="#66ff66"></label>
</div>
</div>
<div id="divider" onclick="togglePanel()">📌</div>
<div id="content">
<div class="card"><div class="title">Current Temperature</div><div class="value" id="tv">-- °C</div>
<div class="graphWrap"><canvas id="tBase" class="base"></canvas><canvas id="tOver" class="overlay"></canvas></div></div>
<div class="card"><div class="title">Current Relative Humidity</div><div class="value" id="hv">-- %</div>
<div class="graphWrap"><canvas id="hBase" class="base"></canvas><canvas id="hOver" class="overlay"></canvas></div></div>
<div class="card"><div class="title">Current Pressure</div><div class="value" id="pv">-- hPa</div>
<div class="graphWrap"><canvas id="pBase" class="base"></canvas><canvas id="pOver" class="overlay"></canvas></div></div>
</div>
<script>
let ws = new WebSocket('ws://'+location.host+'/ws');

let T=[],H=[],P=[],TIME=[];
let panel=true;

function togglePanel(){
 panel=!panel;
 settings.style.display=panel?"block":"none";
 divider.style.left=panel?"260px":"0";
 content.style.marginLeft=panel?"280px":"10px";
}

/*FIX: format elapsed time as hh:mm:ss */
function formatElapsed(ms){
 let s=Math.floor(ms/1000);
 let h=Math.floor(s/3600);
 let m=Math.floor((s%3600)/60);
 let sec=s%60;
 return String(h).padStart(2,'0')+":"+
        String(m).padStart(2,'0')+":"+
        String(sec).padStart(2,'0');
}

function convertData(data,type){
 let d=data.slice();
 if(type=="T" && tempUnit.value=="F") d=d.map(v=>v*9/5+32);
 if(type=="P" && presUnit.value=="PSIA") d=d.map(v=>v*0.0145038);
 return d;
}

function drawBase(canvas,data,color,type){
 let ctx=canvas.getContext("2d");
 let w=canvas.width=canvas.offsetWidth;
 let h=canvas.height=canvas.offsetHeight;
 ctx.fillStyle="#111";ctx.fillRect(0,0,w,h);
 if(data.length<2)return;

 let d=convertData(data,type);
 let min=Math.min(...d),max=Math.max(...d);
 if(max===min)max+=1;
 let pad=30;

 if(gridMaster.value=="on"){
  ctx.strokeStyle="#222";
  for(let i=0;i<5;i++){
   let y=pad+i*(h-2*pad)/4;
   ctx.beginPath();ctx.moveTo(40,y);ctx.lineTo(w-10,y);ctx.stroke();
  }
 }

 if(yLabels.value=="on"){
  ctx.fillStyle="#888";ctx.font="11px Arial";
  for(let i=0;i<5;i++){
   let val=max-i*(max-min)/4;
   let y=pad+i*(h-2*pad)/4;
   ctx.fillText(val.toFixed(1),2,y+4);
  }
 }

 if(xLabels.value=="on"){
  ctx.fillStyle="#888";ctx.font="11px Arial";
  for(let i=0;i<data.length;i+=Math.ceil(data.length/5)){
   let x=40+i*(w-60)/(data.length-1);
   let tLabel=timeMode.value=="elapsed"
     ? formatElapsed(TIME[i]-TIME[0])     // ★ FIX
     : new Date(TIME[i]).toLocaleTimeString();
   ctx.fillText(tLabel,x-20,h-5);
  }
 }

 ctx.strokeStyle=color;ctx.beginPath();
 d.forEach((v,i)=>{
  let x=40+i*(w-60)/(d.length-1);
  let y=h-pad-(v-min)*(h-2*pad)/(max-min);
  i?ctx.lineTo(x,y):ctx.moveTo(x,y);
 });
 ctx.stroke();

 if(extrema.value=="on"){
  let minI=d.indexOf(min),maxI=d.indexOf(max);
  ctx.fillStyle="#fff";ctx.font="11px Arial";
  ctx.fillText(`Min: ${min.toFixed(2)} @ ${new Date(TIME[minI]).toLocaleTimeString()}`,w-170,15);
  ctx.fillText(`Max: ${max.toFixed(2)} @ ${new Date(TIME[maxI]).toLocaleTimeString()}`,w-170,30);
 }
}

function setupHover(baseCanvas,overlayCanvas,data,type){
 overlayCanvas.onmousemove=e=>{
  let ctx=overlayCanvas.getContext("2d");
  let w=overlayCanvas.width=overlayCanvas.offsetWidth;
  let h=overlayCanvas.height=overlayCanvas.offsetHeight;
  ctx.clearRect(0,0,w,h);

  let d=convertData(data,type);
  let min=Math.min(...d),max=Math.max(...d); if(max===min)max+=1;
  let pad=30;

  let rect=overlayCanvas.getBoundingClientRect();
  let x=e.clientX-rect.left;
  let i=Math.round((x-40)*(d.length-1)/(w-60));
  if(i>=0&&i<d.length){
   let y=h-pad-(d[i]-min)*(h-2*pad)/(max-min);
   ctx.strokeStyle="#fff";
   ctx.beginPath();ctx.moveTo(40,y);ctx.lineTo(w-10,y);ctx.stroke();
   ctx.beginPath();ctx.moveTo(x,pad);ctx.lineTo(x,h-pad);ctx.stroke();
   ctx.fillStyle="#fff";ctx.font="12px Arial";
   ctx.fillText(d[i].toFixed(2),x+5,y-5);
   let tLabel=timeMode.value=="elapsed"
     ? formatElapsed(TIME[i]-TIME[0])   // ★ FIX
     : new Date(TIME[i]).toLocaleTimeString();
   ctx.fillText(tLabel,x+5,y+15);
  }
 };
}

ws.onmessage=e=>{
 let j=JSON.parse(e.data);
 T.push(j.t);H.push(j.h);P.push(j.p);TIME.push(Date.now());

 tv.innerHTML=tempUnit.value=="C"?j.t.toFixed(1)+" °C":(j.t*9/5+32).toFixed(1)+" °F";
 hv.innerHTML=j.h.toFixed(1)+" %";
 pv.innerHTML=presUnit.value=="hPa"?j.p.toFixed(1)+" hPa":(j.p*0.0145038).toFixed(2)+" PSIA";

 drawBase(tBase,T,cTemp.value,"T");
 drawBase(hBase,H,cHum.value,"H");
 drawBase(pBase,P,cPres.value,"P");

 setupHover(tBase,tOver,T,"T");
 setupHover(hBase,hOver,H,"H");
 setupHover(pBase,pOver,P,"P");
};
</script>

</body>
</html>
)rawliteral";
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client,
               AwsEventType type, void * arg, uint8_t *data, size_t len) {
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=================================");
  Serial.println("ENVIRONMENT DASHBOARD STARTING");
  Serial.print("Board detected: "); Serial.println(BOARD_NAME);
  Serial.print("I2C SDA pin: "); Serial.println(SDA_PIN);
  Serial.print("I2C SCL pin: "); Serial.println(SCL_PIN);
  Serial.println("=================================");

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bme.begin(BME_ADDR)) {
    Serial.println("BME280 NOT FOUND!");
    while(1);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", webpage());
  });

  server.begin();
}

unsigned long lastSend = 0;

void loop() {
  ws.cleanupClients();   // FIX: prevents memory growth over time

  if (millis() - lastSend > 2000) {
    lastSend = millis();

    float t = bme.readTemperature();
    float h = bme.readHumidity();
    float p = bme.readPressure() / 100.0F;

    Serial.printf("T: %.2f °C | H: %.2f %% | P: %.2f hPa\n", t, h, p);

    String json = "{\"t\":"+String(t)+",\"h\":"+String(h)+",\"p\":"+String(p)+"}";
    ws.textAll(json);
  }
}
