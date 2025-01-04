#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ArduinoJson.h>

// Display configuration
#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANEL_CHAIN 1
#define BRIGHTNESS 90

// ESP32 S3 Zero pin defines
#define R1_PIN 1
#define G1_PIN 13
#define B1_PIN 2
#define R2_PIN 3
#define G2_PIN 12
#define B2_PIN 4
#define A_PIN 5
#define B_PIN 10
#define C_PIN 6
#define D_PIN 9
#define E_PIN -1 // required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32
#define LAT_PIN 8
#define OE_PIN 7
#define CLK_PIN 11

HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

// WiFi credentials
const char* WIFI_SSID = "myWiFi";
const char* WIFI_PASSWORD = "12345678";

// Global objects
WebServer server(80);
WebSocketsServer webSocket(81);
MatrixPanel_I2S_DMA* display = nullptr;

// Display state
String currentText = "";
int16_t scrollPosition = PANEL_RES_X;
int16_t minScrollPosition = 0;
uint16_t textColor;

// Web page
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>LED Matrix Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial; 
            text-align: center; 
            margin: 20px;
            background-color: #f0f0f0;
        }
        .container { 
            max-width: 600px; 
            margin: auto;
            display: flex;
            flex-direction: column;
            gap: 15px;
            padding: 20px;
        }
        .control-group {
            display: flex;
            flex-direction: column;
            gap: 10px;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        label {
            font-weight: bold;
            margin-bottom: 5px;
        }
        input[type="text"] {
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 16px;
            width: 100%;
            box-sizing: border-box;
        }
        input[type="color"] {
            width: 100%;
            height: 50px;
            padding: 5px;
            border: 1px solid #ddd;
            border-radius: 5px;
            cursor: pointer;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 15px;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        button:hover {
            background-color: #45a049;
        }
        h1 {
            color: #333;
            margin-bottom: 20px;
        }
        .status {
            font-size: 14px;
            color: #666;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>LED Matrix Control</h1>
        <div class="control-group">
            <label for="textInput">Display Text</label>
            <input type="text" id="textInput" placeholder="Enter text to display...">
        </div>
        
        <div class="control-group">
            <label for="colorPicker">Text Color</label>
            <input type="color" id="colorPicker" value="#ff0000">
        </div>
        
        <button onclick="sendText()">Update Display</button>
        <div id="status" class="status">Ready</div>
    </div>
    <script>
        let ws = new WebSocket('ws://' + window.location.hostname + ':81/');
        const status = document.getElementById('status');
        
        function sendText() {
            const text = document.getElementById('textInput').value;
            const color = document.getElementById('colorPicker').value;
            const rgb = hexToRgb(color);
            
            const data = {
                text: text,
                color: {
                    r: rgb.r,
                    g: rgb.g,
                    b: rgb.b
                }
            };
            
            ws.send(JSON.stringify(data));
            status.textContent = 'Sent: ' + text;
        }
        
        function hexToRgb(hex) {
            const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
            return result ? {
                r: parseInt(result[1], 16),
                g: parseInt(result[2], 16),
                b: parseInt(result[3], 16)
            } : null;
        }
        
        ws.onopen = () => {
            status.textContent = 'Connected';
            console.log('Connected to WebSocket');
        }
        ws.onerror = (error) => {
            status.textContent = 'Error: ' + error;
            console.log('WebSocket Error:', error);
        }
        ws.onclose = () => {
            status.textContent = 'Disconnected';
        }
        ws.onmessage = (e) => {
            console.log('Server:', e.data);
        }
    </script>
</body>
</html>
)rawliteral";

void displayError(const char* message) {
  display->clearScreen();
  display->setTextColor(textColor);
  display->setCursor(0, 1);
  display->print(message);
}

void setText(const String& text) {
  currentText = text;
  scrollPosition = PANEL_RES_X;

  int16_t x1, y1;
  uint16_t w, h;
  display->setTextSize(1);
  display->getTextBounds(currentText, 0, 0, &x1, &y1, &w, &h);
  minScrollPosition = -(w + 1);
}

void updateScroll() {
  display->clearScreen();
  display->setTextColor(textColor);
  display->setCursor(scrollPosition, 1);
  display->print(currentText);

  scrollPosition--;
  if (scrollPosition < minScrollPosition) {
    scrollPosition = PANEL_RES_X;
  }
}

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      String text = doc["text"].as<String>();
      int r = doc["color"]["r"];
      int g = doc["color"]["g"];
      int b = doc["color"]["b"];

      textColor = display->color565(r, g, b);
      setText(text);
    }
  }
}

bool initDisplay() {

  HUB75_I2S_CFG config(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  display = new MatrixPanel_I2S_DMA(config);

  if (!display->begin()) {
    Serial.println("Display initialization failed!");
    return false;
  }

  display->setBrightness8(BRIGHTNESS);
  display->setTextWrap(false);
  display->clearScreen();
  textColor = display->color565(255, 0, 0);  // Default red
  return true;
}

void setup() {
  Serial.begin(115200);

  // Initialize display
  if (!initDisplay()) {
    Serial.println("Matrix initialization failed!");
    return;
  }

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  // Setup web server
  server.on("/", handleRoot);
  server.begin();

  // Start WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  displayError("Ready!");
}

void loop() {
  webSocket.loop();
  server.handleClient();
  updateScroll();
  delay(50);
}
