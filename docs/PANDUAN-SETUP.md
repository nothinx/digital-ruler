# Panduan Setup Digital Ruler (Workshop)

Memakai **broker lokal (Mosquitto)** di LAN — semua di jaringan WiFi `R2C`.
Urutan: **(1) Broker lokal → (2) Firmware → (3) Website → (4) Uji**.

> **SYARAT JARINGAN.** ESP, komputer broker, dan perangkat yang membuka website
> harus terhubung ke **WiFi yang sama (`R2C`)**.

---

## 1. Broker lokal (Mosquitto)

> **Cara cepat (Docker Desktop):** dari folder proyek jalankan `docker compose up -d`.
> Broker (1883 + 9001) dan website (`http://<IP-PC>:8080`) langsung jalan dengan konfigurasi
> di bawah — tidak perlu memasang Mosquitto. Hentikan dengan `docker compose down`.

Pastikan dua listener aktif:

| Klien | Host | Port | Protokol |
|---|---|---|---|
| ESP32 | `192.168.4.180` | **1883** | MQTT plain (TCP) |
| Website | `192.168.4.180` | **9001** | MQTT over WebSocket (ws) |

`mosquitto.conf` minimal:
```conf
listener 1883
protocol mqtt

listener 9001
protocol websockets

allow_anonymous true
```
Jalankan `mosquitto -c mosquitto.conf -v`. Izinkan port 1883 & 9001 di firewall.

> Ganti IP `192.168.4.180` bila IP komputer broker berbeda — harus sama di
> `MQTT_HOST` (firmware) dan `host` (web). Cek IP: `ipconfig`/`ip a`/`ifconfig`.

---

## 2. Firmware ESP32

> **Pilih folder sesuai board:**
> - **ESP32-C3** → `firmware/ruler/` (sketch `ruler.ino`)
> - **ESP32 DevKit biasa** → `firmware/ruler-esp32/` (sketch `ruler-esp32.ino`)
>
> Logika sketch identik; hanya pin di `config.h` yang beda (lihat 2.3).

### 2.1 Library (Arduino IDE → Manage Libraries)
- **Adafruit GFX Library**
- **Adafruit SSD1306**
- **PubSubClient** (Nick O'Leary)

### 2.2 Konfigurasi (`config.h` di folder firmware yang dipilih)
```c
#define DEVICE_ID 1               // GANTI 1..10 untuk tiap alat
#define MQTT_HOST "192.168.4.180" // IP broker lokal
#define WIFI_SSID "R2C"
#define WIFI_PASS "juarajuara"
```
> Alamat OLED `0x3C` dan `TOPIC_NS` sudah terisi. WiFi hardcoded → langsung
> connect. Pin sudah sesuai board masing-masing (lihat tabel 2.3).

### 2.3 Wiring (per varian board)
| Komponen | ESP32-C3 (`firmware/ruler/`) | ESP32 DevKit (`firmware/ruler-esp32/`) |
|---|---|---|
| Tombol S1 → GND | GPIO 5 | GPIO 5 |
| HC-SR04 TRIG / ECHO | GPIO 4 / 3 | GPIO 3 / 2 |
| OLED SDA / SCL | GPIO 6 / 7 | GPIO 21 / 22 |
| OLED & HC-SR04 VCC/GND | 3V3/5V & GND | 3V3/5V & GND |

> ⚠️ ESP32 DevKit klasik: **GPIO 6–11 dipakai flash internal**, jangan dipakai;
> karena itu I2C OLED ada di 21/22 (default I2C board tersebut).

### 2.4 Flash
1. Buka sketch sesuai board: `firmware/ruler/ruler.ino` (C3) atau
   `firmware/ruler-esp32/ruler-esp32.ino` (DevKit) — `config.h` ikut otomatis.
2. Pilih **Board** ESP32 yang sesuai + **Port**, klik **Upload**.
3. Serial Monitor (115200): muncul `[wifi] tersambung` lalu `[mqtt] tersambung`.
4. OLED menampilkan "Tekan & tahan S1 untuk mengukur".

**Untuk 10 alat:** ulangi 2.2 & 2.4, cukup ganti `DEVICE_ID` (01…10).

---

## 3. Website (HTTP LOKAL)

> ⚠️ Jangan pakai URL https/GitHub Pages — broker lokal `ws://` diblokir dari
> halaman https. Buka via **http lokal**.

### 3.1 Konfigurasi (`web/config.js`)
```js
host: "192.168.4.180",
port: 9001,
namespace: "r2c-rul-6e8cb4",  // HARUS sama dengan TOPIC_NS firmware
scaleMax: 100,                // skala bar mistar (cm)
```

### 3.2 Jalankan
Dari folder `web/`, di komputer yang terhubung `R2C`:
```bash
python -m http.server 8000
```
Buka `http://localhost:8000` (atau `http://192.168.4.180:8000` dari perangkat lain).

### 3.3 Opsi: file mandiri (1 file, offline)
`web/ruler-standalone.html` berisi semua (HTML+CSS+JS+library). Bagikan, dobel-klik
di laptop → langsung jalan tanpa server. Bila `config.js` diubah, bangun ulang:
`python build-standalone.py`.

> Library `mqtt.js` sudah ditanam lokal → website jalan **tanpa internet**.

---

## 4. Uji

### 4.1 Dengan alat
- Status broker pojok kanan atas → **Tersambung** (hijau).
- Pilih **Alat 01** → indikator **Online**.
- **Tekan & tahan S1** di alat → OLED menampilkan jarak, dan **angka di web ikut
  berubah** real-time. Lepas → nilai terakhir tetap tampil (retained).
- Min/Maks terisi otomatis; tombol **Reset** mengembalikan ke nilai terakhir.

### 4.2 Tanpa hardware (cek alur MQTT)
Pakai **MQTT Explorer** → connect `192.168.4.180:1883` → publish ke
`<NS>/ruler-01/distance` payload `15.0` → angka di web jadi 15.0 cm.

---

## Troubleshooting

| Gejala | Penyebab & solusi |
|---|---|
| Web "Error koneksi" / "Menyambung…" terus | Buka via **http lokal** (bukan https). Cek `host`/`port 9001`. Pastikan listener 9001 jalan & firewall mengizinkan. |
| Web tak update saat S1 ditekan | `namespace` (web) ≠ `TOPIC_NS` (firmware), atau ESP belum konek broker. |
| OLED blank / "Gagal inisialisasi" | Cek wiring SDA=6/SCL=7 & alamat `0x3C` (sebagian modul `0x3D`). |
| Jarak selalu "Di luar jangkauan" | Cek wiring TRIG=4/ECHO=3 & catu daya HC-SR04 (butuh 5V). |
| ESP tak connect WiFi | SSID/password `R2C`/`juarajuara` salah, atau AP belum nyala. |
| Indikator Offline padahal alat hidup | Beda nomor alat dipilih, atau ESP belum konek broker. |
