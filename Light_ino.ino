 #include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "Nation01_Control";
const char* password = "642135OT"; // Change this!

// Pin definitions
const int LED1_PIN = 27;
const int LED2_PIN = 26;
const int LED3_PIN = 25;
const int LDR_PIN = 34;

// Variables
bool led1State = false;
bool led2State = false;
bool led3State = false;
bool autoMode = false;
const int NIGHT_THRESHOLD = 800;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Set pin modes
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  // Start WiFi
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Setup server routes
  server.…
 Required Libraries for ESP32 Web Server
#include <WiFi.h> // For ESP32 Wi-Fi functionalities
#include <AsyncTCP.h> // Asynchronous TCP library, a dependency for ESPAsyncWebServer
#include <ESPAsyncWebServer.h> // Asynchronous Web Server library for ESP32

// --- Wi-Fi Configuration ---
// Define the SSID (network name) for your ESP32's Access Point
const char* ssid = "TechGulu_Lights";
// Define the password for your ESP32's Access Point. IMPORTANT: Change this to a strong, unique password!
const char* password = "your_strong_password"; // <--- CHANGE THIS PASSWORD!

// --- LED Pin Definitions (Based on Schematic) ---
// These GPIO pins control the BASE of the NPN transistors that switch the LEDs.
const int LED1_CTRL_PIN = 18;  // GPIO18 → Q1 base
const int LED2_CTRL_PIN = 19;  // GPIO19 → Q2 base
const int LED3_CTRL_PIN = 21;  // GPIO21 → Q3 base
Hide quoted text


// --- LDR Pin Definition (Based on Schematic) ---
const int LDR_PIN = 34; // Connected to GPIO 34 for analog reading

// --- Automatic Control Settings ---
// Threshold for determining night/day based on LDR reading.
// Values range from 0 (darkest) to 4095 (brightest) for a 12-bit ADC.
// You will likely need to CALIBRATE this value for your specific LDR and environment.
// Lower value = darker for "night" detection.
const int NIGHT_THRESHOLD = 800; // Example: Below 800 is considered night. ADJUST THIS!

// Delay between automatic light checks (in milliseconds)
const long AUTO_CHECK_INTERVAL = 10000; // Check every 10 seconds

// --- Web Server Object ---
// Create an instance of the AsyncWebServer on port 80 (standard HTTP port)
AsyncWebServer server(80);

// --- LED State Variables ---
// Boolean variables to keep track of the current state of each LED (true for ON, false for OFF)
bool led1State = false;
bool led2State = false;
bool led3State = false;

// --- Automatic Mode State Variable ---
bool autoModeEnabled = false;

// --- Timing Variable for Automatic Control ---
unsigned long lastAutoCheckMillis = 0;

// --- Helper function to set LED state (turns transistor ON/OFF) ---
void setLED(int pin, bool state) {
  // Since we are driving NPN transistors in common-emitter configuration:
  // HIGH on base turns transistor ON -> LED ON
  // LOW on base turns transistor OFF -> LED OFF
  digitalWrite(pin, state ? HIGH : LOW);
}

// --- HTML Content for the Web Dashboard ---
String getDashboardHtml() {
  String html = R"rawliteral(void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
 <script>
            // Function to send a request to the ESP32 to toggle an LED
            async function toggleLED(ledNum) {
                const button = document.getElementById(led${ledNum}Button);
                const statusIndicator = document.getElementById(led${ledNum}Status);

                try {
                    const response = await fetch(/led${ledNum}/toggle);
                    const data = await response.text();
                    console.log(Response for LED${ledNum}:, data);
                    updateUI(ledNum, data.includes("ON")); // Update UI based on response
                } catch (error) {
                    console.error('Error toggling LED:', error);
                    alert('Could not connect to ESP32. Please ensure it is powered on and you are connected to its Wi-Fi network.');
                }
            }

            // Function to toggle Automatic Mode
            async function toggleAutoMode() {
                const button = document.getElementById('autoModeButton');
                const statusIndicator = document.getElementById('autoModeStatus');

                try {
                    const response = await fetch('/automode/toggle');
                    const data = await response.json(); // Expecting JSON: { "autoModeEnabled": true/false }
                    console.log('Auto Mode Toggled:', data);
                    updateAutoModeUI(data.autoModeEnabled);
                } catch (error) {
                    console.error('Error toggling Auto Mode:', error);
                    alert('Could not connect to ESP32 to toggle Auto Mode.');
                }
            }

            // Function to fetch the current status of all LEDs and Auto Mode from the ESP32
            async function updateAllStatus() {
                try {
                    const response = await fetch('/status');
                    const data = await response.json();
                    console.log("Current System Status:", data);

                    updateUI(1, data.led1);
                    updateUI(2, data.led2);
                    updateUI(3, data.led3);
                    updateAutoModeUI(data.autoModeEnabled);
                    document.getElementById('ldrValue').innerText = data.ldrValue; // Update LDR value

                } catch (error) {
                    console.error('Error fetching system status:', error);
                }
            }

            // Helper function to update the UI elements for a specific LED
            function updateUI(ledNum, state) {
                const button = document.getElementById(led${ledNum}Button);
                const statusIndicator = document.getElementById(led${ledNum}Status);

                if (state) { // If state is true (LED is ON)
                    button.classList.add('on');
                    statusIndicator.classList.remove('off');
                    statusIndicator.classList.add('on');
                } else { // If state is false (LED is OFF)
                    button.classList.remove('on');
                    statusIndicator.classList.remove('on');
                    statusIndicator.classList.add('off');
                }
            }

            // Helper function to update the UI for Automatic Mode
            function updateAutoModeUI(state) {
                const button = document.getElementById('autoModeButton');
                const statusIndicator = document.getElementById('autoModeStatus');

                if (state) {
                    button.classList.add('on');
                    button.innerText = "Automatic Mode: ON";
                    statusIndicator.classList.remove('off');
                    statusIndicator.classList.add('on');
                } else {
                    button.classList.remove('on');
                    button.innerText = "Automatic Mode: OFF";
                    statusIndicator.classList.remove('on');
                    statusIndicator.classList.add('off');
                }
            }

            // Call updateAllStatus when the window finishes loading
            window.onload = function() {
                updateAllStatus();
                // Periodically refresh the entire system status to keep the dashboard in sync
                setInterval(updateAllStatus, 3000); // Refresh every 3 seconds
            };
        </script>
    </body>
    </html>
  )rawliteral";
  return html;
}

// --- Arduino Setup Function ---
void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting ESP32 Intelligent Lighting System...");

  // Set LED control pins as OUTPUTs
  pinMode(LED1_CTRL_PIN, OUTPUT);
  pinMode(LED2_CTRL_PIN, OUTPUT);
  pinMode(LED3_CTRL_PIN, OUTPUT);
 
  // Set LDR pin as INPUT (implicitly done for analogRead, but good practice)
  pinMode(LDR_PIN, INPUT);

  // Initialize all LEDs to OFF state
  setLED(LED1_CTRL_PIN, LOW);
  setLED(LED2_CTRL_PIN, LOW);
  setLED(LED3_CTRL_PIN, LOW);
  led1State = false;
  led2State = false;
  led3State = false;

  // Start the ESP32 in Access Point (AP) mode
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point (AP) IP address: ");
  Serial.println(IP);
  Serial.print("Connect to Wi-Fi network: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.println("Then open a web browser and go to the IP address above.");

  // --- Web Server Route Handlers ---

  // Route for the root URL ("/") - serves the main HTML dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Client requested root URL '/'");
    request->send(200, "text/html", getDashboardHtml());
  });

  // Route to toggle LED 1
  server.on("/led1/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    led1State = !led1State;
    setLED(LED1_CTRL_PIN, led1State);
    Serial.printf("LED 1 toggled to: %s\n", led1State ? "ON" : "OFF");
    request->send(200, "text/plain", led1State ? "LED1_ON" : "LED1_OFF");
  });

  // Route to toggle LED 2
  server.on("/led2/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    led2State = !led2State;
    setLED(LED2_CTRL_PIN, led2State);
    Serial.printf("LED 2 toggled to: %s\n", led2State ? "ON" : "OFF");
    request->send(200, "text/plain", led2State ? "LED2_ON" : "LED2_OFF");
  });

  // Route to toggle LED 3
  server.on("/led3/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    led3State = !led3State;
    setLED(LED3_CTRL_PIN, led3State);
    Serial.printf("LED 3 toggled to: %s\n", led3State ? "ON" : "OFF");
    request->send(200, "text/plain", led3State ? "LED3_ON" : "LED3_OFF");
  });

  // Route to toggle Automatic Mode
  server.on("/automode/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    autoModeEnabled = !autoModeEnabled;
    Serial.printf("Automatic Mode toggled to: %s\n", autoModeEnabled ? "ENABLED" : "DISABLED");
    String jsonResponse = "{ \"autoModeEnabled\": " + String(autoModeEnabled ? "true" : "false") + " }";
    request->send(200, "application/json", jsonResponse);
  });

  // Route to get the current status of all LEDs, Auto Mode, and LDR value as JSON
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    // Serial.println("Client requested system status '/status'"); // Uncomment for more verbose logging
    int ldrValue = analogRead(LDR_PIN); // Read LDR value
   
    String jsonResponse = "{";
    jsonResponse += "\"led1\":" + String(led1State ? "true" : "false") + ",";
    jsonResponse += "\"led2\":" + String(led2State ? "true" : "false") + ",";
    jsonResponse += "\"led3\":" + String(led3State ? "true" : "false") + ",";
    jsonResponse += "\"autoModeEnabled\":" + String(autoModeEnabled ? "true" : "false") + ",";
    jsonResponse += "\"ldrValue\":" + String(ldrValue);
    jsonResponse += "}";
    request->send(200, "application/json", jsonResponse);
  });

  // Start the web server
  server.begin();
  Serial.println("Web server started.");
}

// --- Arduino Loop Function ---
// This function runs repeatedly after setup()
void loop() {
  // Check for automatic light control if enabled
  if (autoModeEnabled) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastAutoCheckMillis >= AUTO_CHECK_INTERVAL) {
      lastAutoCheckMillis = currentMillis;

      int ldrValue = analogRead(LDR_PIN);
      Serial.printf("LDR Value: %d, Threshold: %d\n", ldrValue, NIGHT_THRESHOLD);

      if (ldrValue < NIGHT_THRESHOLD) { // It's dark (LDR value is low)
        Serial.println("It's NIGHT - Turning ALL LEDs ON automatically.");
        if (!led1State) { led1State = true; setLED(LED1_CTRL_PIN, HIGH); }
        if (!led2State) { led2State = true; setLED(LED2_CTRL_PIN, HIGH); }
        if (!led3State) { led3State = true; setLED(LED3_CTRL_PIN, HIGH); }
      } else { // It's bright (LDR value is high)
        Serial.println("It's DAY - Turning ALL LEDs OFF automatically.");
        if (led1State) { led1State = false; setLED(LED1_CTRL_PIN, LOW); }
        if (led2State) { led2State = false; setLED(LED2_CTRL_PIN, LOW); }
        if (led3State) { led3State = false; setLED(LED3_CTRL_PIN, LOW); }
      }
    }
  }
}
Show quoted text
