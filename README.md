# WiFi-RGB-Matrix
## ESP32 LED Matrix WebSocket Controller

This project creates a web-controlled scrolling text display using an ESP32 and an HUB75 LED matrix panel. Users can change the displayed text and its color through a mobile-friendly web interface.

![LED Matrix Display](/api/placeholder/400/200)

## Features

- 🌐 Web-based control interface
- 📱 Mobile-friendly responsive design
- 🎨 Real-time color selection
- 📜 Smooth text scrolling
- 🔄 WebSocket communication for instant updates
- 💻 Easy to set up and modify

## Hardware Requirements

- ESP32 development board
- HUB75 LED Matrix Panel (64x32 pixels)
- 5V power supply suitable for your LED matrix
- USB cable for programming

## Software Requirements

### Required Libraries

1. **ESP32-HUB75-MatrixPanel-I2S-DMA** by mrfaptastic
   - Install via Arduino Library Manager
   - Handles LED matrix control

2. **WebSockets** by Markus Sattler
   - Install via Arduino Library Manager
   - Search for "WebSockets" (not "WebSocket")
   - Provides WebSocket server functionality

3. **ArduinoJson** by Benoit Blanchon
   - Install via Arduino Library Manager
   - Handles JSON parsing for WebSocket messages

Note: `WiFi` and `WebServer` libraries are included with the ESP32 board package and don't need separate installation.

## Installation

1. **Install Arduino IDE**
   - Download and install the latest version from [arduino.cc](https://www.arduino.cc/en/software)

2. **Install ESP32 Board Package**
   - Open Arduino IDE
   - Go to File > Preferences
   - Add this URL to Additional Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools > Board > Boards Manager
   - Search for "esp32" and install "ESP32 by Espressif Systems"

3. **Install Required Libraries**
   - Open Arduino IDE
   - Go to Tools > Manage Libraries
   - Install "WebSockets", "ArduinoJson" and "ESP32-HUB75-MatrixPanel-I2S-DMA"

4. **Configure the Project**
   - Clone this repository or download the code
   - Open the `.ino` file in Arduino IDE
   - Update WiFi credentials:
     ```cpp
     const char* WIFI_SSID = "your_wifi_name";
     const char* WIFI_PASSWORD = "your_wifi_password";
     ```

5. **Upload to ESP32**
   - Select your ESP32 board from Tools > Board menu
   - Select the correct port from Tools > Port
   - Click Upload button or press Ctrl+U

## Usage

1. After uploading, open Serial Monitor (Tools > Serial Monitor)
2. Note the IP address displayed after WiFi connection
3. Visit `http://[ESP32-IP-ADDRESS]` in your web browser
4. Enter text and select color in the interface
5. Click "Update Display" to send to the matrix

## Configuration

You can modify these constants in the code to match your setup:

```cpp
#define PANEL_RES_X 64      // Panel width
#define PANEL_RES_Y 32      // Panel height
#define PANEL_CHAIN 1       // Number of panels chained
#define BRIGHTNESS 90       // Display brightness (0-255)
```
Check [ESP32-HUB75-MatrixPanel-DMA project page ](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA) to see what's possible with the library.

## How It Works

1. **ESP32 Setup**
   - Initializes the LED matrix
   - Connects to WiFi
   - Starts WebSocket server
   - Serves web interface

2. **Web Interface**
   - Provides text input and color picker
   - Connects to ESP32 via WebSocket
   - Sends updates in real-time

3. **Communication Flow**
   - User enters text/selects color
   - Data sent via WebSocket as JSON
   - ESP32 updates matrix display
   - Text scrolls continuously

## Troubleshooting

- **Matrix Not Displaying**: Check power supply and connections
- **Can't Connect**: Verify WiFi credentials and ESP32 IP address
- **Compilation Errors**: Ensure all libraries are installed
- **Display Glitches**: Try adjusting brightness or power supply

## Credits

- LED Matrix library by mrcodetastic
- WebSockets library by Markus Sattler
- ArduinoJson library by Benoit Blanchon
