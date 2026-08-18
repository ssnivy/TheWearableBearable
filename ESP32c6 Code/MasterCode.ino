#include <WiFi.h>
#include <esp_now.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define LED_PIN LED_BUILTIN
#define MOTOR_PIN 18   // GPIO18 for vibration motor

#define LED_ON  LOW    // most ESP32 built-in LEDs are active-LOW
#define LED_OFF HIGH

bool blinkMode = true;
unsigned long lastBeat = 0;
unsigned long beatInterval = 500;

// motor pulse timing
unsigned long pulseStart = 0;
bool pulseActive = false;

// ---------------- ESP-NOW ----------------
typedef struct {
  uint8_t cmd;
  uint8_t bpm;
} Command;

Command command;

void addBroadcastPeer() {
  esp_now_peer_info_t peerInfo = {};
  memset(peerInfo.peer_addr, 0xFF, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

inline void sendCommand(uint8_t bpm) {
  command.cmd = 0x02;
  command.bpm = bpm;
  esp_now_send(
    (uint8_t*)"\xFF\xFF\xFF\xFF\xFF\xFF",
    (uint8_t*)&command,
    sizeof(command)
  );
}

// ---------------- BLE ----------------
#define SERVICE_UUID "00001234-0000-1000-8000-00805f9b34fb"
#define CHAR_UUID    "00005678-0000-1000-8000-00805f9b34fb"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

// Restart advertising on disconnect
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("[BLE] Client connected");
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("[BLE] Client disconnected");
    delay(100); // improves Android reliability
    BLEDevice::getAdvertising()->start();
  }
};

class BPMCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    if (c->getLength() != 1) return;

    uint8_t bpm = c->getData()[0];

    if (bpm == 0) {
      blinkMode = false;
      digitalWrite(LED_PIN, LED_OFF);
      digitalWrite(MOTOR_PIN, LOW);
      Serial.println("[BLE] LED & MOTOR OFF");
    } else {
      blinkMode = true;
      if (bpm > 200) bpm = 200;
      beatInterval = 60000 / bpm;
      lastBeat = millis();
      Serial.print("[BLE] BPM set to ");
      Serial.println(bpm);
    }

    sendCommand(bpm);
  }
};

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(21, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(21,LOW);

  // WiFi required for ESP-NOW
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed");
    return;
  }

  addBroadcastPeer();

  // BLE init
  BLEDevice::init("ESP32-BLE-BLINK");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new BPMCallback());

  pService->start();
  BLEDevice::getAdvertising()->start();

  lastBeat = millis();
  Serial.println("[MASTER READY]");
}

// ---------------- LOOP ----------------
void loop() {
  unsigned long now = millis();

  // Start beat
  if (blinkMode && now - lastBeat >= beatInterval) {
    lastBeat = now;

    digitalWrite(LED_PIN, LED_ON);
    digitalWrite(MOTOR_PIN, HIGH);

    pulseStart = now;
    pulseActive = true;

    Serial.println("[BEAT] LED & MOTOR ON");
  }

  // End pulse (non-blocking)
  if (pulseActive && now - pulseStart >= 100) {
    digitalWrite(LED_PIN, LED_OFF);
    digitalWrite(MOTOR_PIN, LOW);
    pulseActive = false;

    Serial.println("[BEAT] LED & MOTOR OFF");
  }
}
