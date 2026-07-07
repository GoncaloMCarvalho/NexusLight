#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// MOSFET Pins
const int pinR = 25;
const int pinG = 26;
const int pinB = 27;

// Volatile state variables (shared between cores)
volatile int currentMode = -1;    
volatile int valR = 0;
volatile int valG = 0;
volatile int valB = 0;

TaskHandle_t TaskBluetooth;

// --- FUNCTION PROTOTYPES ---
void setColor(int r, int g, int b);
void bluetoothTaskCode(void * parameter);
void partyPattern();
void cyclePattern();

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("====== PHONE CONNECTED SUCCESSFULLY! ======");
  } else if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("====== PHONE DISCONNECTED! ======");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  
  // Ensure lights are off on startup
  setColor(0, 0, 0);
  
  SerialBT.register_callback(btCallback);
  SerialBT.begin("NexusLight"); 
  
  // DEBUG: Startup confirmation
  Serial.println("=========================================");
  Serial.println("NexusLight Started! Waiting for Bluetooth.");
  Serial.println("=========================================");

  // Pin Bluetooth task to Core 0
  xTaskCreatePinnedToCore(
    bluetoothTaskCode, 
    "Task_Bluetooth",  
    10000,             
    NULL,              
    1,                 
    &TaskBluetooth,    
    0                  
  );
}

// ==========================================================
// CORE 0: BLUETOOTH MANAGEMENT
// ==========================================================
void bluetoothTaskCode(void * parameter) {
  for(;;) { 
    if (SerialBT.available()) {
      String message = SerialBT.readStringUntil('\n'); 
      message.trim(); 

      // DEBUG: Display the exact received message
      Serial.print("-> MSG Received: [");
      Serial.print(message);
      Serial.println("]");

      if (message == "TOGGLE") {
        Serial.println("Command: TOGGLE");
        if (currentMode != -1) {
          currentMode = -1;  // If it was ON (Party, Cycle, or RGB), turn it OFF
        } else {
          currentMode = 0;   // If it was OFF, ALWAYS GO TO RGB MODE (0)
        }
      }
      else if (message == "M-1") {
        Serial.println("Command: M-1 (Force OFF on app connection)");
        currentMode = -1;
      }
      else if (message == "CICLO") {
        Serial.println("Command: CYCLE");
        currentMode = 1;
      }
      else if (message == "FESTA") {
        Serial.println("Command: PARTY");
        currentMode = 2;
      }
      else if (message.startsWith("R")) {
        valR = message.substring(1).toInt();
        if (currentMode != -1) currentMode = 0; // Switch to RGB mode if not currently OFF
        Serial.print("Slider R: "); Serial.println(valR);
      }
      else if (message.startsWith("G")) {
        valG = message.substring(1).toInt();
        if (currentMode != -1) currentMode = 0;
        Serial.print("Slider G: "); Serial.println(valG);
      }
      else if (message.startsWith("B")) {
        valB = message.substring(1).toInt();
        if (currentMode != -1) currentMode = 0;
        Serial.print("Slider B: "); Serial.println(valB);
      }
      else {
        Serial.println("Warning: Unknown or empty command!");
      }
    }

    // Small delay to yield to FreeRTOS watchdog
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// ==========================================================
// CORE 1: LIGHT MANAGEMENT & ANIMATIONS
// ==========================================================
void loop() {
  // DEBUG: Periodic state print (every 2 seconds) to avoid flooding the terminal
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 2000) {
      Serial.printf("Current State -> Mode: %d | R: %d, G: %d, B: %d\n", currentMode, valR, valG, valB);
      lastDebugPrint = millis();
  }

  if (currentMode == -1) {
    setColor(0, 0, 0);
    delay(50);
  }
  else if (currentMode == 0) {
    setColor(valR, valG, valB);
    delay(20);
  } 
  else if (currentMode == 1) {
    cyclePattern();
  } 
  else if (currentMode == 2) {
    partyPattern();
  }
}

// ---------------- ANIMATION FUNCTIONS ----------------

void partyPattern() {
  setColor(random(0, 256), random(0, 256), random(0, 256));
  delay(150); 
}

void cyclePattern() {
  float time = millis() / 1000.0;
  int r = (sin(time) * 127) + 128;
  int g = (sin(time + 2.0) * 127) + 128;
  int b = (sin(time + 4.0) * 127) + 128;
  setColor(r, g, b);
  delay(20); 
}

void setColor(int r, int g, int b) {
  analogWrite(pinR, constrain(r, 0, 255));
  analogWrite(pinG, constrain(g, 0, 255));
  analogWrite(pinB, constrain(b, 0, 255));
}