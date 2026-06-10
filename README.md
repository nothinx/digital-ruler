# Digital Ruler — Workshop (10 Alat)

Mistar digital berbasis **ESP32 + HC-SR04 (ultrasonik) + OLED SSD1306 + tombol
S1**. Tekan S1 → alat mengukur jarak, tampil di OLED, **dan** dikirim ke website
lewat **MQTT (broker lokal Mosquitto)**. Untuk **workshop 10 alat identik**.

```
[Website (http lokal)] --ws:9001--> [Mosquitto LAN] <--tcp:1883-- [ESP32 x10]
   pilih alat 01..10                192.168.4.180                  HC-SR04 + OLED
   tampil jarak terakhir                                           ukur saat S1
```

> Semua perangkat (ESP, broker, browser) harus di **WiFi/LAN yang sama (`R2C`)**.
> Halaman dibuka via **http lokal**, bukan https (lihat Panduan Setup).

## Fitur
- Ukur jarak on-demand: tekan & tahan **S1**, jarak tampil di OLED + dikirim ke web.
- Web tampilkan **angka besar + bar mistar** + statistik **Min/Maks/Terakhir** (tema teal).
- 10 alat identik dibedakan `DEVICE_ID` (01–10), topic MQTT ber-namespace.
- Status online/offline real-time (MQTT Last Will).
- Offline-ready: `mqtt.js` ditanam lokal + tersedia **file mandiri** 1 file.

## Struktur
```
digital-ruler/
├── firmware/ruler/
│   ├── ruler.ino
│   └── config.h            # << DEVICE_ID + IP broker + WiFi + pin
├── web/
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   ├── config.js           # << host + namespace (samakan dgn firmware)
│   ├── mqtt.min.js          # library ditanam lokal (offline)
│   ├── build-standalone.py  # rakit -> ruler-standalone.html
│   └── ruler-standalone.html
├── docs/
│   ├── PANDUAN-SETUP.md
│   └── superpowers/specs/
└── README.md
```

## Mulai cepat
1. **Broker**: Mosquitto lokal jalan dengan listener `1883` (mqtt) + `9001` (websockets).
2. **Firmware**: isi `firmware/ruler/config.h`, ganti `DEVICE_ID` per alat, flash.
   Library: Adafruit GFX, Adafruit SSD1306, PubSubClient.
3. **Web**: jalankan `web/` via **http lokal** (`python -m http.server`) atau bagikan
   `ruler-standalone.html`. Buka dari perangkat di WiFi `R2C`.

➡️ Detail di **[docs/PANDUAN-SETUP.md](docs/PANDUAN-SETUP.md)**.

## Pin (sesuai skematik EasyEDA)
| Fungsi | Pin |  | Fungsi | Pin |
|---|---|---|---|---|
| Tombol S1 | 5 |  | TRIG | 4 |
| OLED SDA | 6 |  | ECHO | 3 |
| OLED SCL | 7 |  | OLED I2C | 0x3C |

## Topic MQTT
Awalan per alat: `<NS>/ruler-NN/`

| Topic | Arah | Payload |
|---|---|---|
| `distance` | ESP → web | jarak cm, mis. `12.3` (retained) |
| `status` | ESP → web | `online` / `offline` (retained, LWT) |
