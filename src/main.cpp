#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Pinos dos MOSFETs
const int pinR = 25;
const int pinG = 26;
const int pinB = 27;

// Variáveis de estado voláteis
volatile int currentMode = -1;    
volatile int valR = 0;
volatile int valG = 0;
volatile int valB = 0;

TaskHandle_t TaskBluetooth;

// --- PROTÓTIPOS DE FUNÇÃO ---
void setColor(int r, int g, int b);
void bluetoothTaskCode(void * parameter);
void partyPattern();
void cyclePattern();

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("====== TELEMÓVEL LIGADO COM SUCESSO! ======");
  } else if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("====== TELEMÓVEL DESLIGADO! ======");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  
  setColor(0, 0, 0);
  
  SerialBT.register_callback(btCallback);
  SerialBT.begin("NexusLight"); 
  
  // DEBUG: Confirmação de arranque
  Serial.println("=========================================");
  Serial.println("NexusLight Iniciado! Bluetooth a aguardar.");
  Serial.println("=========================================");

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
// CORE 0: GESTÃO BLUETOOTH
// ==========================================================
void bluetoothTaskCode(void * parameter) {
  for(;;) { 
    if (SerialBT.available()) {
      String message = SerialBT.readStringUntil('\n'); 
      message.trim(); 

      // DEBUG: Mostrar a mensagem exata que chegou
      Serial.print("-> MSG Recebida: [");
      Serial.print(message);
      Serial.println("]");

      if (message == "TOGGLE") {
        Serial.println("Comando: TOGGLE");
        if (currentMode != -1) {
          currentMode = -1;  // Se estava ligado (Festa, Ciclo ou RGB), desliga
        } else {
          currentMode = 0;   // Se estava desligado, VAI SEMPRE PARA O MODO RGB (0)
        }
      }
      else if (message == "M-1") {
        Serial.println("Comando: M-1 (Forçar OFF pelo arranque)");
        currentMode = -1;
      }
      else if (message == "CICLO") {
        Serial.println("Comando: CICLO");
        currentMode = 1;
      }
      else if (message == "FESTA") {
        Serial.println("Comando: FESTA");
        currentMode = 2;
      }
      else if (message.startsWith("R")) {
        valR = message.substring(1).toInt();
        if (currentMode != -1) currentMode = 0; // Só passa a RGB se não estiver em OFF
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
        Serial.println("Aviso: Comando desconhecido ou vazio!");
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

// ==========================================================
// CORE 1: GESTÃO DAS LUZES
// ==========================================================
void loop() {
  // DEBUG: Print periódico do estado (a cada 2 segundos) para não afogar o terminal
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 2000) {
      Serial.printf("Estado Atual -> Modo: %d | R: %d, G: %d, B: %d\n", currentMode, valR, valG, valB);
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

// ---------------- FUNÇÕES DE ANIMAÇÃO ----------------

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