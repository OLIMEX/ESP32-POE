#include <ETH.h>
#include <WebServer.h>
#include <Update.h>
#include <FS.h>

// Define your firmware version
#define FIRMWARE_VERSION "1.0.0"

WebServer server(80);

// HTML page with OTA upload, progress bar, firmware version and IP
const char* uploadPage = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>ESP32 OTA Update</title>
    <style>
      body { font-family: Arial; margin: 20px; }
      #progress-container { width: 100%; background: #eee; margin-top: 10px; }
      #progress-bar { width: 0%; height: 20px; background: #4caf50; }
    </style>
  </head>
  <body>
    <h1>ESP32 OTA Update via Ethernet</h1>
    <p><strong>Device IP:</strong> <span id="ip"></span></p>
    <p><strong>Firmware Version:</strong> <span id="fwver"></span></p>
    <form id="uploadForm">
      <input type="file" id="firmware" name="firmware">
      <input type="submit" value="Upload">
    </form>
    <p>File: <span id="fileName">None</span>, Size: <span id="fileSize">0</span> bytes</p>
    <div id="progress-container">
      <div id="progress-bar"></div>
    </div>
    <p id="status"></p>

    <script>
      document.getElementById('ip').textContent = window.location.hostname;
      document.getElementById('fwver').textContent = "%VERSION%";

      const form = document.getElementById('uploadForm');
      const progressBar = document.getElementById('progress-bar');
      const statusText = document.getElementById('status');
      const fileNameSpan = document.getElementById('fileName');
      const fileSizeSpan = document.getElementById('fileSize');

      document.getElementById('firmware').addEventListener('change', function(e) {
        if(this.files.length > 0){
          fileNameSpan.textContent = this.files[0].name;
          fileSizeSpan.textContent = this.files[0].size;
        }
      });

      form.addEventListener('submit', function(e) {
        e.preventDefault();
        const file = document.getElementById('firmware').files[0];
        if (!file) return alert("Please select a file!");

        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/update', true);

        xhr.upload.onprogress = function(e) {
          if (e.lengthComputable) {
            const percent = (e.loaded / e.total) * 100;
            progressBar.style.width = percent + '%';
            statusText.textContent = 'Uploading: ' + Math.round(percent) + '%';
          }
        };

        xhr.onload = function() {
          if (xhr.status === 200) {
            statusText.textContent = 'Upload complete! Device will reboot.';
            setTimeout(() => { location.reload(); }, 3000);
          } else {
            statusText.textContent = 'Upload failed!';
          }
        };

        const formData = new FormData();
        formData.append('firmware', file);
        xhr.send(formData);
      });
    </script>
  </body>
</html>
)rawliteral";

// Replace placeholder with firmware version
String generatePage() {
  String page = uploadPage;
  page.replace("%VERSION%", FIRMWARE_VERSION);
  return page;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Starting Ethernet...");

  pinMode(17, OUTPUT);
  digitalWrite(17, HIGH);  // enable external clock 

  // PHY reset sequence
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);   // Hold PHY in reset
  delay(300);              // Keep it low for 100ms
  digitalWrite(12, HIGH);  // Release PHY reset
  delay(50);               // Wait 50ms for PHY clock to stabilize

  // Start Ethernet: PHY reset = 12, PHY clock = 17
  ETH.begin(ETH_PHY_LAN8720, 0, 23, 18, 12, ETH_CLOCK_GPIO17_OUT);

  // Wait for Ethernet IP
  Serial.print("Waiting for Ethernet IP");
  while (ETH.localIP() == INADDR_NONE) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ETH IP: ");
  Serial.println(ETH.localIP());

  // Serve main HTML page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", generatePage());
  });

  // OTA update endpoint
  server.on("/update", HTTP_POST, []() {
    if (Update.hasError()) {
      server.send(200, "text/plain", "Update Failed!");
    } else {
      server.send(200, "text/plain", "Update Success! Rebooting...");
      delay(1000);
      ESP.restart();
    }
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update Start: %s\n", upload.filename.c_str());
      if (!Update.begin()) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) Serial.printf("Update Success: %u bytes\n", upload.totalSize);
      else Update.printError(Serial);
    }
  });

  server.begin();
  Serial.println("HTTP OTA server started");
}

void loop() {
  server.handleClient();
}
