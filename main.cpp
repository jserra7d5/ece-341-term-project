#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

#define LM75A_ADDR 0x48  // Default I2C address of LM75A

// AP Credentials
const char* ssid = "ESP32_LM75A_AP";
const char* password = "12345678";

// Web server on port 80
WebServer server(80);

// Function to read temperature
float readTemperature() {
    Wire.beginTransmission(LM75A_ADDR);
    Wire.write(0x00);  
    Wire.endTransmission(false);
    Wire.requestFrom(LM75A_ADDR, 2);

    if (Wire.available() < 2) return NAN;  

    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();

    int16_t rawTemp = (msb << 8) | (lsb & 0xE0);
    rawTemp >>= 5;

    return rawTemp * 0.125;  
}

// Serve the main HTML page
void handleRoot() {
    String html = R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>ESP32 Temperature</title>
            <style>
                body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
                h1 { color: #333; }
                .temp-box { font-size: 24px; padding: 10px; border: 1px solid #333; display: inline-block; margin-top: 20px; }
            </style>
            <script>
                function updateTemperature() {
                    fetch('/temperature')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById("tempC").innerText = data.celsius + " °C";
                        document.getElementById("tempF").innerText = data.fahrenheit + " °F";
                    })
                    .catch(error => console.error("Error fetching temperature:", error));
                }
                setInterval(updateTemperature, 1000); // Update every second
            </script>
        </head>
        <body>
            <h1>ESP32 LM75A Temperature</h1>
            <div class="temp-box">
                <p><strong>Temperature (Celsius):</strong> <span id="tempC">Loading...</span></p>
                <p><strong>Temperature (Fahrenheit):</strong> <span id="tempF">Loading...</span></p>
            </div>
        </body>
        </html>
    )rawliteral";

    server.send(200, "text/html", html);
}

// Serve temperature as JSON
void handleTemperature() {
    float tempC = readTemperature();
    float tempF = (tempC * 9.0 / 5.0) + 32.0;

    String json = "{\"celsius\": \"" + String(tempC) + "\", \"fahrenheit\": \"" + String(tempF) + "\"}";
    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    
    Serial.println("ESP32 AP Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Define routes
    server.on("/", handleRoot);
    server.on("/temperature", handleTemperature);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}
