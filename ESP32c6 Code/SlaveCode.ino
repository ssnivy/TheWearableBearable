#include <WiFi.h>
#include <esp_now.h>

#define LED_PIN LED_BUILTIN
#define EXTRA_LED 18   // GPIO18 mirrors built-in LED

#define LED_ON  LOW    // built-in LED is active-LOW
#define LED_OFF HIGH

bool blinkMode = false;
unsigned long lastBeat = 0;
unsigned long beatInterval = 500;

// pulse timing
unsigned long pulseStart = 0;
bool pulseActive = false;

// ---- ESP-NOW command structure (MUST MATCH MASTER) ----
typedef struct {
  uint8_t cmd;
  uint8_t bpm;
} Command;

Command incomingCommand;

// ---- ESP-NOW receive callback ----
void OnDataRecv(const esp_now_recv_info_t*,
                const uint8_t* data,
                int len) {

  if (len != sizeof(Command)) {
    Serial.println("[ERROR] Invalid packet size");
    return;
  }

  memcpy(&incomingCommand, data, sizeof(Command));

  Serial.print("[RECV] CMD=0x");
  Serial.print(incomingCommand.cmd, HEX);
  Serial.print(" BPM=");
  Serial.println(incomingCommand.bpm);

  // ---- BLINK COMMAND ----
  if (incomingCommand.cmd == 0x02) {

    if (incomingCommand.bpm == 0) {
      blinkMode = false;
      digitalWrite(LED_PIN, LED_OFF);
      digitalWrite(EXTRA_LED, LOW);
      Serial.println("[STOP] LEDs OFF");
      return;
    }

    uint8_t bpm = incomingCommand.bpm;
    if (bpm > 200) bpm = 200;

    beatInterval = 60000 / bpm;
    lastBeat = millis();

    blinkMode = true;
    pulseActive = false;

    digitalWrite(LED_PIN, LED_OFF);
    digitalWrite(EXTRA_LED, LOW);

    Serial.print("[SYNC] Interval = ");
    Serial.print(beatInterval);
    Serial.println(" ms");
  }
}

// ---- SETUP ----
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 SLAVE STARTED ===");

  pinMode(LED_PIN, OUTPUT);
  pinMode(EXTRA_LED, OUTPUT);

  digitalWrite(LED_PIN, LED_OFF);
  digitalWrite(EXTRA_LED, LOW);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("[READY] Waiting for ESP-NOW beats");
}

// ---- LOOP ----
void loop() {
  unsigned long now = millis();

  // ---- START BEAT ----
  if (blinkMode && !pulseActive && now - lastBeat >= beatInterval) {
    lastBeat = now;

    digitalWrite(LED_PIN, LED_ON);
    digitalWrite(EXTRA_LED, HIGH);

    pulseStart = now;
    pulseActive = true;

    Serial.println("[BEAT] LED ON");
  }

  // ---- END PULSE (100 ms) ----
  if (pulseActive && now - pulseStart >= 100) {
    digitalWrite(LED_PIN, LED_OFF);
    digitalWrite(EXTRA_LED, LOW);
    pulseActive = false;

    Serial.println("[BEAT] LED OFF");
  }
}
