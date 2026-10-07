# AquaLeaf

AquaLeaf adalah platform irigasi tanaman otomatis berbasis IoT yang dirancang untuk menjaga kelembaban tanah secara real-time menggunakan ESP8266 NodeMCU. Sistem ini menggabungkan pembacaan sensor kelembaban, kontrol relay pompa air, komunikasi MQTT, integrasi Blynk, serta dashboard web untuk monitoring dan pengendalian.

## Ringkasan Proyek

AquaLeaf membantu pengguna mengelola penyiraman tanaman secara otomatis dan aman, dengan fitur seperti:

- Penyiraman otomatis berdasarkan nilai kelembaban tanah
- Kontrol pompa air dengan waktu aman dan cooldown anti-overwatering
- Auto reconnect WiFi dan fallback access point
- Integrasi MQTT dan Blynk untuk monitoring jarak jauh
- Dashboard web untuk kontrol manual dan pengaturan threshold
- Logging serial yang rapi untuk debugging dan monitoring sistem
- Arsitektur modular yang mudah dikembangkan dan dipelihara

## Fitur Utama

- Sensor soil moisture automatic irrigation
- Relay kontrol pompa air dengan timeout safety
- Fallback AP dan reconnect WiFi otomatis
- Publikasi data ke broker MQTT
- Integrasi Blynk untuk akses cepat
- REST API sederhana untuk monitoring dan kontrol
- Struktur kode modular sesuai prinsip SOLID

## Struktur Folder

```text
.
├── include/      # Konfigurasi pin, konstanta, dan kredensial
├── src/          # Kode sumber proyek
├── data/         # Asset dashboard web untuk SPIFFS
├── docs/         # Dokumentasi arsitektur, wiring, API, dan flowchart
├── test/         # Folder unit test
├── platformio.ini
├── README.md
└── scripts/      # Script pendukung proyek
```

## Persyaratan

- PlatformIO
- ESP8266 NodeMCU / board kompatibel
- Sensor kelembaban tanah
- Relay modul pompa air
- Koneksi WiFi dan broker MQTT
- Akun Blynk (opsional, jika ingin menggunakan integrasi Blynk)

## Persiapan Awal

1. Salin file `include/Secrets.example.h` menjadi `include/Secrets.h`
2. Isi variabel berikut:
   - `WIFI_SSID`
   - `WIFI_PASSWORD`
   - `MQTT_USERNAME`
   - `MQTT_PASSWORD`
   - `BLYNK_AUTH_TOKEN`
3. Pastikan konfigurasi pin dan threshold sudah sesuai dengan hardware yang digunakan

## Quick Start

### Kompilasi proyek

```bash
platformio run
```

### Upload dashboard web ke SPIFFS

```bash
platformio run --target uploadfs
```

### Flash ke perangkat

```bash
platformio run --target upload
```

## Unit Test

Jalankan pengujian dengan PlatformIO:

```bash
platformio test -e nodemcu_v2
```

Jika menggunakan board atau environment lain, sesuaikan nama environment sesuai konfigurasi di `platformio.ini`.

## Cara Kerja Sistem

1. NodeMCU membaca nilai kelembaban tanah secara berkala
2. Sistem membandingkan data sensor dengan threshold yang telah ditentukan
3. Pompa air aktif ketika nilai kelembaban di bawah threshold rendah
4. Pompa air mati ketika nilai kelembaban sudah kembali di atas threshold tinggi
5. Data dikirim ke MQTT dan Blynk secara berkala
6. User dapat memantau dan mengatur sistem melalui dashboard web

## Dashboard Web

Buka browser ke IP NodeMCU untuk melihat status real-time, mengubah threshold, serta mengontrol pompa secara manual.

## Dokumentasi

Dokumentasi proyek tersedia di folder berikut:

- `docs/architecture.md`
- `docs/wiring-diagram.md`
- `docs/api-documentation.md`
- `docs/flowchart.md`

## Catatan

Proyek ini dibuat untuk kebutuhan otomatisasi irigasi tanaman dengan pendekatan modular dan mudah dikembangkan untuk kebutuhan produksi maupun penelitian.
