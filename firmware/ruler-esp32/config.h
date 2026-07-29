// =====================================================================
//  config.h  —  Digital Ruler: pengaturan per-alat & broker
//  VARIAN BOARD: ESP32 DevKit BIASA (klasik / WROOM-32).
//  Untuk ESP32-C3 gunakan folder firmware/ruler/ (pin beda).
//  Ganti nilai di bawah lalu flash. Hanya DEVICE_ID yang beda tiap unit.
// =====================================================================
#ifndef CONFIG_H
#define CONFIG_H

// --- Identitas alat: GANTI angka ini 1..10 untuk tiap unit ------------
#define DEVICE_ID 1               // -> topic "<NS>/ruler-01/..."

// --- Prefix topic UNIK (samakan dengan web/config.js) ----------------
#define TOPIC_NS "r2c-rul-6e8cb4"

// --- Broker MQTT LOKAL (Mosquitto di LAN, plain/tanpa TLS) ------------
#define MQTT_HOST "192.168.4.180" // IP broker lokal
#define MQTT_PORT 1883            // plain MQTT (web socket pakai 9001)
#define MQTT_USER ""              // broker lokal anonim: kosongkan
#define MQTT_PASS ""              // broker lokal anonim: kosongkan

// --- WiFi (AP lokal Tenda, kredensial tetap utk semua alat) ----------
#define WIFI_SSID "R2C"
#define WIFI_PASS "juarajuara"

// --- Pin untuk ESP32 DevKit BIASA ------------------------------------
//  Catatan: GPIO 6-11 di ESP32 klasik dipakai flash internal -> JANGAN
//  dipakai. I2C default board ini SDA=21, SCL=22.
#define BUTTON_PIN 5              // S1 (INPUT_PULLUP, tekan = LOW)
#define SDA_PIN    21             // OLED SDA (I2C default ESP32)
#define SCL_PIN    22             // OLED SCL (I2C default ESP32)
#define TRIG_PIN   3              // HC-SR04 TRIG
#define ECHO_PIN   2              // HC-SR04 ECHO

// --- OLED SSD1306 -----------------------------------------------------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C

// --- Pengukuran -------------------------------------------------------
#define MEASURE_INTERVAL_MS 150   // jeda publish saat S1 ditahan (~6x/dtk)
#define ECHO_TIMEOUT_US     25000 // ~4.3 m; di luar itu = tak ada echo
#define MAX_VALID_CM        400.0 // batas wajar HC-SR04

#endif // CONFIG_H
