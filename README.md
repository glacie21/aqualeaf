# AquaLeaf — Intelligent Plant Irrigation Platform

AquaLeaf adalah solusi IoT profesional untuk penyiraman tanaman otomatis menggunakan ESP8266 NodeMCU.
Platform ini memantau kelembaban tanah, mengendalikan pompa air lewat relay, mengirimkan data ke MQTT dan Blynk, serta menampilkan dashboard web realtime.

## Fitur Utama

- Otomatis penyiraman berdasarkan sensor soil moisture
- Relay kontrol pompa air dengan timeout safety dan cooldown anti-overwatering
- Auto reconnect WiFi dan fallback AP
- Publikasi data MQTT dan integrasi Blynk
- Logging serial profesional
- REST API sederhana untuk monitoring dan kontrol
- Arsitektur modular bersih sesuai SOLID

## Struktur Proyek

- `include/` : konfigurasi, pin, konstanta, kredensial
- `src/` : kode sumber modular
- `data/` : assets dashboard web untuk SPIFFS
- `docs/` : dokumentasi arsitektur, wiring, API, dan alur
- `test/` : folder placeholder untuk unit test

## Persiapan

1. Salin `include/Secrets.example.h` ke `include/Secrets.h`
2. Isi `WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_USERNAME`, `MQTT_PASSWORD`, dan `BLYNK_AUTH_TOKEN`
3. Upload file SPIFFS dengan `platformio run --target uploadfs`
4. Compile dan flash dengan `platformio run --target upload`

## Kompilasi

```bash
platformio run
```

## Upload Dashboard Web

```bash
platformio run --target uploadfs
```

## Unit Test

Jalankan unit test dengan PlatformIO:

```bash
platformio test -e nodemcu_v2
```

Jika ingin menggunakan environment lain, ganti nama environment sesuai `platformio.ini`.

## Penggunaan

1. Nyalakan NodeMCU dan sambungkan ke WiFi
2. Sistem membaca sensor setiap 2 detik
3. Pompa menyala saat kelembaban berada di bawah threshold rendah
4. Pompa mati saat kelembaban sudah di atas threshold tinggi
5. Data dikirim ke MQTT dan Blynk secara berkala

## Dashboard Web

Buka browser ke alamat IP NodeMCU untuk melihat status realtime, mengubah threshold, dan mengontrol pompa secara manual.

## Dokumentasi

Lihat `docs/architecture.md`, `docs/wiring-diagram.md`, `docs/api-documentation.md`, dan `docs/flowchart.md`.
