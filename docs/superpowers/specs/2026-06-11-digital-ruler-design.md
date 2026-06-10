# Desain: Digital Ruler (Workshop 10 Alat)

**Tanggal:** 2026-06-11
**Status:** Disetujui untuk implementasi

## Tujuan

Mistar digital berbasis **ESP32 + HC-SR04 (ultrasonik) + OLED SSD1306 + tombol
S1**. Saat S1 ditekan, alat mengukur jarak dan menampilkannya di OLED **serta**
mengirimkannya ke website lewat MQTT. Untuk **workshop 10 alat identik**.
Protokol & struktur sama dengan proyek smarthome (broker lokal Mosquitto).

## Keputusan

| Topik | Keputusan |
|---|---|
| Mode pengukuran | **On-demand**: ukur & publish saat tombol S1 ditekan/ditahan |
| Jumlah alat | 10 (workshop), `DEVICE_ID` 01–10, topic ber-namespace |
| Broker | Mosquitto lokal `192.168.4.180` (mqtt `1883`, ws `9001`) |
| WiFi | Hardcoded AP `R2C` |
| Web | Tampilan hasil pengukuran terakhir (retained); tema **teal/hijau** |
| Offline | `mqtt.js` ditanam lokal + tersedia file mandiri (tanpa internet) |

## Hardware (pin sesuai skematik EasyEDA)

| Fungsi | Pin |
|---|---|
| Tombol S1 (INPUT_PULLUP, tekan = LOW) | 5 |
| OLED SDA | 6 |
| OLED SCL | 7 |
| HC-SR04 TRIG | 4 |
| HC-SR04 ECHO | 3 |
| OLED | SSD1306 128×64, I2C 0x3C |

## Diagram

```
[Website (http lokal)] --ws:9001--> [Mosquitto LAN] <--tcp:1883-- [ESP32 x10]
   pilih alat 01..10                192.168.4.180                  HC-SR04 + OLED
   tampil jarak terakhir                                           ukur saat S1
```

## Skema Topic MQTT

Awalan per alat: `<NS>/ruler-NN/`

| Topic | Arah | Payload | Sifat |
|---|---|---|---|
| `distance` | ESP → web | jarak cm, mis. `12.3` | retained |
| `status` | ESP → web | `online` / `offline` | retained (LWT) |

Alat mem-publish `distance` selama S1 ditahan (di-throttle ~6×/detik). Nilai
terakhir retained → web yang baru dibuka langsung menampilkan bacaan terakhir.
Bacaan di luar jangkauan (tanpa echo) tidak di-publish; OLED menampilkan
"di luar jangkauan".

## Firmware

Library: Adafruit_GFX, Adafruit_SSD1306, PubSubClient (Wire/WiFi bawaan).
Alur: WiFi (R2C) → MQTT connect (LWT) → loop: baca S1 → bila ditekan, ukur
(`pulseIn` dengan timeout), tampilkan OLED, publish; bila lepas, layar standby.
`pulseIn` diberi timeout agar tidak nge-block saat tak ada echo.

## Website (tema teal/hijau)

- Pemilih alat 01–10 + indikator online/offline.
- Panel utama: **angka jarak besar** + satuan, dengan **bar mistar** ber-tick
  yang penanda-nya bergerak sesuai jarak (skala 0–`scaleMax` cm).
- Statistik sesi: Terakhir / Minimum / Maksimum + tombol Reset.
- MQTT.js over WSS→ws lokal, tanpa backend.

## Lingkup

**Termasuk**: firmware, website (+ file mandiri), dok setup, dokumen desain.
**Tidak termasuk**: histori tersimpan, kalibrasi multi-titik, app native.
