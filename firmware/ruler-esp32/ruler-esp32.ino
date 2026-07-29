// =====================================================================
//  Digital Ruler — Firmware ESP32 DevKit (klasik / WROOM-32)
//  HC-SR04 (ultrasonik) + OLED SSD1306 + tombol S1 + MQTT (broker lokal)
//
//  Varian untuk board ESP32 DevKit BIASA. Untuk ESP32-C3 pakai sketch di
//  folder firmware/ruler/. Logika IDENTIK; hanya pin (config.h) yang beda.
//
//  Saat S1 ditekan: ukur jarak, tampilkan di OLED, publish ke MQTT.
//  Antar-unit hanya DEVICE_ID di config.h yang beda.
//
//  Library (Library Manager):
//    - Adafruit GFX Library
//    - Adafruit SSD1306
//    - PubSubClient        (Nick O'Leary)
//  Wire, WiFi, WiFiClient = bawaan core ESP32.
// =====================================================================
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "config.h"

// ---------------------------------------------------------------------
//  Global
// ---------------------------------------------------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClient       netClient;
PubSubClient     mqtt(netClient);

char deviceTag[12];        // "ruler-01"
char baseTopic[64];        // "<NS>/ruler-01"

unsigned long lastTick = 0;
bool needStandby = true;

// ---------------------------------------------------------------------
//  Sensor ultrasonik
// ---------------------------------------------------------------------
float measureDistance() {              // cm, atau -1 bila tak valid
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long dur = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);  // timeout: tak nge-block
  if (dur == 0) return -1.0;
  float cm = (dur * 0.0343) / 2.0;
  if (cm <= 0 || cm > MAX_VALID_CM) return -1.0;
  return cm;
}

// ---------------------------------------------------------------------
//  OLED
// ---------------------------------------------------------------------
void drawSplash(const char* msg) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 6);
  display.println("Digital Ruler");
  display.print(deviceTag);
  display.setCursor(0, 44);
  display.println(msg);
  display.display();
}

void drawReading(float cm) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Jarak  ");
  display.print(deviceTag);
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  display.setTextSize(3);
  display.setCursor(0, 24);
  display.print(cm, 1);
  display.setTextSize(2);
  display.print(" cm");
  display.display();
}

void drawOutOfRange() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(deviceTag);
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(8, 28);
  display.println("Di luar jangkauan");
  display.setCursor(8, 42);
  display.println("arahkan ke objek");
  display.display();
}

void drawStandby() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(deviceTag);
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(6, 26);
  display.println("Tekan & tahan S1");
  display.setCursor(6, 40);
  display.println("untuk mengukur");
  display.setCursor(0, 56);
  display.print(mqtt.connected() ? "MQTT: tersambung" : "MQTT: ...");
  display.display();
}

// ---------------------------------------------------------------------
//  MQTT
// ---------------------------------------------------------------------
void publishDistance(float cm) {
  char topic[80];
  sprintf(topic, "%s/distance", baseTopic);
  char payload[16];
  snprintf(payload, sizeof(payload), "%.1f", cm);
  mqtt.publish(topic, payload, true);   // retained: web baru langsung lihat
}

void mqttConnect() {
  char statusTopic[72];
  sprintf(statusTopic, "%s/status", baseTopic);

  const char* user = strlen(MQTT_USER) ? MQTT_USER : nullptr;
  const char* pass = strlen(MQTT_PASS) ? MQTT_PASS : nullptr;

  char clientId[40];
  sprintf(clientId, "%s-%06X", deviceTag, (uint32_t)(ESP.getEfuseMac() & 0xFFFFFF));

  while (!mqtt.connected()) {
    Serial.print("[mqtt] menghubungkan... ");
    if (mqtt.connect(clientId, user, pass, statusTopic, 1, true, "offline")) {
      Serial.println("tersambung");
      mqtt.publish(statusTopic, "online", true);   // retained
    } else {
      Serial.printf("gagal rc=%d, coba lagi 3s\n", mqtt.state());
      delay(3000);
    }
  }
}

// ---------------------------------------------------------------------
//  Setup & loop
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  sprintf(deviceTag, "ruler-%02d", DEVICE_ID);
  sprintf(baseTopic, "%s/%s", TOPIC_NS, deviceTag);
  Serial.printf("\n=== Digital Ruler %s ===\n", deviceTag);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Gagal inisialisasi OLED"));
    for (;;) delay(1000);
  }
  drawSplash("Menyalakan...");
  delay(800);

  // WiFi (AP lokal, kredensial tetap di config.h)
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  drawSplash("Sambung WiFi...");
  Serial.printf("[wifi] menyambung ke %s", WIFI_SSID);
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" gagal, restart...");
    drawSplash("WiFi gagal :(");
    delay(2000);
    ESP.restart();
  }
  Serial.printf("\n[wifi] tersambung: %s\n", WiFi.localIP().toString().c_str());

  // MQTT
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(256);
  drawSplash("Sambung broker...");
  mqttConnect();

  drawSplash("Siap!");
  delay(600);
}

void loop() {
  if (!mqtt.connected()) mqttConnect();
  mqtt.loop();

  bool pressed = (digitalRead(BUTTON_PIN) == LOW);
  unsigned long now = millis();

  if (pressed) {
    if (now - lastTick >= MEASURE_INTERVAL_MS) {
      lastTick = now;
      float cm = measureDistance();
      if (cm >= 0) {
        drawReading(cm);
        publishDistance(cm);
      } else {
        drawOutOfRange();
      }
    }
    needStandby = true;            // tandai supaya digambar ulang saat dilepas
  } else {
    if (needStandby) {
      drawStandby();
      needStandby = false;
    }
  }
  delay(10);
}
