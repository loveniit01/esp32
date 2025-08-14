#include <WiFi.h>
#include <WebServer.h>
#include <AceButton.h>
#include <EEPROM.h>
using namespace ace_button;

// Relay & Button pins
const int relayPins[] = {23, 19, 18, 5};
const int buttonPins[] = {13, 12, 14, 27};
const int numRelays = 4;
bool relayStates[numRelays] = {false, false, false, false};

const int eepromLed = 2;   //D2

const int EEPROM_SIZE = 64;
const int EEPROM_FLAG_ADDR = 0;
const int EEPROM_STATE_ADDR = 1;
bool eepromRestoreFlag = false;

// AceButton objects
ButtonConfig buttonConfigs[numRelays];
AceButton* buttons[numRelays];



// WebServer instance
WebServer server(80);

// Fixed IP configuration
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// WiFi credentials
const char* ssid = "**";  //WiFi Name
const char* password = "**";  // WiFi Password

// ========== EEPROM ==========

void saveRelayStates() {
  for (int i = 0; i < numRelays; i++) {
    EEPROM.write(EEPROM_STATE_ADDR + i, relayStates[i]);
  }
  EEPROM.commit();
}

void loadRelayStates() {
  for (int i = 0; i < numRelays; i++) {
    relayStates[i] = EEPROM.read(EEPROM_STATE_ADDR + i);
  }
}

void saveRestoreFlag() {
  EEPROM.write(EEPROM_FLAG_ADDR, eepromRestoreFlag);
  EEPROM.commit();
}

void loadRestoreFlag() {
  eepromRestoreFlag = EEPROM.read(EEPROM_FLAG_ADDR);
}

void handleStatus() {
  String json = "{";
  for (int i = 0; i < numRelays; i++) {
    json += "\"relay" + String(i) + "\":" + (relayStates[i] ? "true" : "false") + ",";
  }
  json += "\"eeprom\":" + String(eepromRestoreFlag ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  if (server.hasArg("relay")) {
    int relay = server.arg("relay").toInt();
    if (relay >= 0 && relay < 4) {
      relayStates[relay] = !relayStates[relay];
      digitalWrite(relayPins[relay], relayStates[relay] ? LOW : HIGH);
      saveRelayStates();
    }
  }
  handleStatus();
}

void handleAllOff() {
  for (int i = 0; i < numRelays; i++) {
    relayStates[i] = false;
    digitalWrite(relayPins[i], HIGH);
  }
  saveRelayStates();
  handleStatus();
}

void handleEEPROMFlag() {
  eepromRestoreFlag = !eepromRestoreFlag;
  saveRestoreFlag();
  handleStatus();
}

// ========== Button Handlers (Latched Mode) ==========

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      relayStates[0] = true;
      digitalWrite(relayPins[0], LOW);
      break;
    case AceButton::kEventReleased:
      relayStates[0] = false;
      digitalWrite(relayPins[0], HIGH);
      break;
  }
}

void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      relayStates[1] = true;
      digitalWrite(relayPins[1], LOW);
      break;
    case AceButton::kEventReleased:
      relayStates[1] = false;
      digitalWrite(relayPins[1], HIGH);
      break;
  }
}

void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      relayStates[2] = true;
      digitalWrite(relayPins[2], LOW);
      break;
    case AceButton::kEventReleased:
      relayStates[2] = false;
      digitalWrite(relayPins[2], HIGH);
      break;
  }
}

void button4Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      relayStates[3] = true;
      digitalWrite(relayPins[3], LOW);
      break;
    case AceButton::kEventReleased:
      relayStates[3] = false;
      digitalWrite(relayPins[3], HIGH);
      break;
  }
}




// ========== Web Handlers ==========

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Home Automation</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #f0f0f0; }
    h2 { text-align: center; padding: 20px 0; color: #333; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; padding: 0 20px; }
    .card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.2); text-align: center; }
    .btn { padding: 10px 20px; border: none; border-radius: 8px; font-size: 16px; cursor: pointer; width: 100%; }
    .on { background: #28a745; color: white; }
    .off { background: #dc3545; color: white; }
    .alloff-btn { margin: 20px; padding: 15px; background: #007bff; color: white; border: none; border-radius: 8px; font-size: 18px; width: calc(100% - 40px); cursor: pointer; }
    .toggle-flag { margin: 20px; padding: 15px; background: #6c757d; color: white; border: none; border-radius: 8px; font-size: 18px; width: calc(100% - 40px); cursor: pointer; }
    .footer { text-align: center; padding: 15px; font-size: 14px; color: #666; }
  </style>
</head>
<body>
  <h2>ESP32 Home Automation</h2>
  <div class="grid">
    <div class="card"><button id="btn0" class="btn on" onclick="toggleRelay(0)">Relay 1</button></div>
    <div class="card"><button id="btn1" class="btn on" onclick="toggleRelay(1)">Relay 2</button></div>
    <div class="card"><button id="btn2" class="btn on" onclick="toggleRelay(2)">Relay 3</button></div>
    <div class="card"><button id="btn3" class="btn on" onclick="toggleRelay(3)">Relay 4</button></div>
  </div>
  <button class="alloff-btn" onclick="allOff()">ALL OFF</button>

  <button class="toggle-flag" onclick="toggleEEPROMFlag()">EEPROM Restore: <span id="flagState">{{EEPROM_FLAG}}</span></button>

  <div class="footer">Created by Tech Studycell</div>

<script>
function toggleRelay(id) {
  fetch('/toggle?relay=' + id).then(fetchStatus);
}

function allOff() {
  fetch('/alloff').then(fetchStatus);
}

function toggleEEPROMFlag() {
  fetch('/eepromflag').then(fetchStatus);
}

function fetchStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      for (let i = 0; i < 4; i++) {
        let btn = document.getElementById('btn' + i);
        if (data['relay' + i]) {
          btn.classList.remove('on');
          btn.classList.add('off');
          btn.textContent = 'Relay ' + (i + 1) + ' (ON)';
        } else {
          btn.classList.remove('off');
          btn.classList.add('on');
          btn.textContent = 'Relay ' + (i + 1) + ' (OFF)';
        }
      }
      document.getElementById('flagState').textContent = data['eeprom'] ? 'ON' : 'OFF';
    });
}

// Auto-refresh status every 2 seconds
setInterval(fetchStatus, 2000);
fetchStatus();
</script>
</body>
</html>
)rawliteral";

  html.replace("{{EEPROM_FLAG}}", eepromRestoreFlag ? "ON" : "OFF");
  server.send(200, "text/html", html);
}




// ========== Setup ==========

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // Load EEPROM restore flag & relay states
  loadRestoreFlag();
  if (eepromRestoreFlag) {
    loadRelayStates();
    Serial.println("Restored relay states from EEPROM");
  }

  // Connect WiFi with fixed IP
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("ESP IP: "); Serial.println(WiFi.localIP());

  // Initialize relay pins
  for (int i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], relayStates[i] ? LOW : HIGH);
  }

  // Initialize buttons
  for (int i = 0; i < numRelays; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttons[i] = new AceButton(&buttonConfigs[i]);
    buttons[i]->init(buttonPins[i]);
  }

  pinMode(eepromLed, OUTPUT); // Initialize LED
  digitalWrite(eepromLed, LOW);

  // Attach button handlers (latched mode)
  buttonConfigs[0].setEventHandler(button1Handler);
  buttonConfigs[1].setEventHandler(button2Handler);
  buttonConfigs[2].setEventHandler(button3Handler);
  buttonConfigs[3].setEventHandler(button4Handler);

  // Web routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);
  server.on("/alloff", handleAllOff);
  server.on("/eepromflag", handleEEPROMFlag);

  server.begin();
  Serial.println("Web Server started");
}

// ========== Loop ==========

void loop() {
  server.handleClient();

  // Check buttons
  for (uint8_t i = 0; i < numRelays; i++) {
    buttons[i]->check();
  }
  digitalWrite(eepromLed, eepromRestoreFlag);
}


