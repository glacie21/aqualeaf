# AquaLeaf

AquaLeaf adalah platform irigasi tanaman otomatis berbasis IoT yang dirancang untuk menjaga kelembaban tanah secara real-time menggunakan ESP8266 NodeMCU. Sistem ini menggabungkan pembacaan sensor kelembaban, kontrol relay pompa air, komunikasi MQTT, integrasi Blynk, serta dashboard web untuk monitoring dan pengendalian.

## Fitur Utama

- Penyiraman otomatis berdasarkan threshold kelembaban (dengan hysteresis low/high)
- Jadwal jam penyiraman (mis. hanya pagi dan sore) — waktu diambil dari NTP
- Safety pompa: batas waktu nyala per siklus, cooldown, batas total per 24 jam
- Proteksi dry-run: penyiraman otomatis dikunci jika kelembaban tidak naik
  setelah beberapa siklus (tangki kosong / selang lepas / sensor macet)
- Deteksi sensor rusak (nilai raw di luar rentang wajar)
- Dashboard web: status, grafik kelembaban 24 jam, kontrol manual, threshold,
  jam penyiraman, kalibrasi sensor, dan pengaturan WiFi
- Setup WiFi tanpa reflash lewat access point + captive portal
- Update firmware over-the-air (OTA) dan akses via `http://aqualeaf.local` (mDNS)
- Telemetry MQTT per perangkat (dengan status online/offline via Last Will) dan Blynk
- Pengaturan tersimpan di EEPROM dengan versi + CRC
- Unit test logika inti yang berjalan di PC dan GitHub Actions

## Struktur Folder

```text
.
├── .github/workflows/ci.yml    # Build + unit test di GitHub Actions
├── data/                       # Asset dashboard web untuk LittleFS
├── docs/                       # Dokumentasi proyek
├── include/                    # Config.h, Constants.h, Pins.h, Secrets.example.h
├── scripts/pre_build.py        # Cek Secrets.h sebelum build
├── src/
│   ├── actuators/              # Relay dan PumpController
│   ├── core/                   # SystemManager (orkestrasi) dan Scheduler
│   ├── dashboard/              # Web server + REST API
│   ├── logic/                  # Logika murni tanpa Arduino (dites di PC)
│   ├── models/                 # Struct data
│   ├── network/                # WiFi, MQTT, OTA/mDNS
│   ├── sensors/                # Sensor kelembaban tanah
│   ├── storage/                # Penyimpanan pengaturan di EEPROM
│   ├── utils/                  # Logger dan helper
│   └── main.cpp
├── test/
│   ├── test_native_*/          # Unit test host (pio test -e native)
│   └── test_pump, test_sensor, ... # Test di board (pio test -e nodemcu_v2)
└── platformio.ini
```

## Persyaratan

- PlatformIO
- ESP8266 NodeMCU (v1/v2/v3) / board kompatibel
- Sensor kelembaban tanah (kapasitif disarankan) dan relay modul pompa air
- Broker MQTT (opsional) dan akun Blynk (opsional)

## Persiapan Awal

1. Salin template kredensial:
   ```powershell
   Copy-Item include/Secrets.example.h include/Secrets.h
   ```
2. Isi `include/Secrets.h`:
   - `WEB_ADMIN_USER`, `WEB_ADMIN_PASS` — login dashboard (**ganti nilai default**)
   - `OTA_PASSWORD` — password update firmware via WiFi (**ganti nilai default**)
   - `WIFI_SSID`, `WIFI_PASSWORD` — opsional; hanya dipakai saat boot pertama.
     Bisa dibiarkan dan diatur dari dashboard (lihat *Setup WiFi*)
   - `MQTT_USERNAME`, `MQTT_PASSWORD` — kosongkan jika broker tanpa auth
   - `BLYNK_*` — biarkan token placeholder untuk menonaktifkan Blynk
3. Sesuaikan `include/Config.h` (threshold, zona waktu `TIME_ZONE`, `DEVICE_HOSTNAME`,
   batas pompa) dan `include/Pins.h` dengan hardware Anda.

## Quick Start

```bash
pio run -e nodemcu_v2 -t upload      # flash firmware via USB
pio run -e nodemcu_v2 -t uploadfs    # upload dashboard web ke LittleFS
pio device monitor                   # lihat log serial
```

### Update firmware via WiFi (OTA)

Setelah flash pertama via USB, firmware berikutnya bisa dikirim lewat WiFi:

```powershell
$env:AQUALEAF_OTA_PASSWORD = "<OTA_PASSWORD dari Secrets.h>"
pio run -e nodemcu_v2_ota -t upload
pio run -e nodemcu_v2_ota -t uploadfs
```

Jika `aqualeaf.local` tidak ter-resolve, ganti `upload_port` di `platformio.ini`
dengan IP perangkat. Selama update berjalan pompa otomatis dimatikan.

## Setup WiFi

Jika perangkat belum punya jaringan tersimpan (atau tidak bisa tersambung selama
20 detik), ia membuka access point **AquaLeaf-Setup** (password `WIFI_AP_PASSWORD`
di `Config.h`). Sambungkan ponsel ke AP tersebut; halaman login akan muncul
otomatis (atau buka `http://192.168.4.1`). Isi SSID dan password di panel
**Network**, lalu tekan **Connect**. Kredensial disimpan di flash dan AP
dimatikan begitu perangkat tersambung.

Jika ada beberapa perangkat dalam satu jaringan, beri masing-masing
`DEVICE_HOSTNAME` yang berbeda.

## Dashboard Web

Buka `http://aqualeaf.local` (atau IP perangkat) dan login dengan
`WEB_ADMIN_USER` / `WEB_ADMIN_PASS`.

- **Manual pump** — menyalakan/mematikan pompa dan menangguhkan mode otomatis
  selama 30 menit (atau sampai konfigurasi disimpan).
- **Threshold, Automatic Mode, Watering Hours** — tekan **Save Configuration**.
  Menyimpan juga me-reset kunci dry-run.
- **Sensor calibration** — pegang sensor di udara lalu tekan **Set dry**, celupkan
  ke air lalu tekan **Set wet**. Nilai tersimpan di EEPROM.
- **Grafik 24 jam** — kelembaban tiap 5 menit beserta tanda kapan pompa menyala;
  riwayat disimpan di RAM sehingga hilang saat reboot.

## Unit Test

```bash
pio test -e native        # logika inti di PC (butuh gcc/g++; juga jalan di CI)
pio test -e nodemcu_v2    # test di board (board terhubung via USB)
```

Logika yang tidak bergantung pada hardware — state machine pompa, aturan
threshold & jadwal, proteksi dry-run, konversi sensor, format EEPROM, buffer
riwayat — ada di `src/logic/` dan dites di `env:native`. Di Windows, `gcc` bisa
dipasang lewat MSYS2, atau jalankan test di WSL/Docker. Setiap push juga
menjalankan test dan build di GitHub Actions.

## Cara Kerja Sistem

1. Sensor dibaca setiap 2 detik; nilai raw di luar rentang wajar ditandai sebagai fault
2. Jika kelembaban ≤ threshold low **dan** jam sekarang termasuk jam penyiraman,
   pompa menyala (maks. 30 detik per siklus, lalu cooldown 5 menit)
3. Pompa mati saat kelembaban ≥ threshold high
4. Jika beberapa siklus berturut-turut tidak menaikkan kelembaban, penyiraman otomatis dikunci
5. Total waktu nyala pompa dibatasi per 24 jam (termasuk kontrol manual)
6. Data dikirim ke MQTT dan Blynk secara berkala

## Dokumentasi

- `docs/architecture.md`
- `docs/wiring-diagram.md`
- `docs/api-documentation.md`
- `docs/flowchart.md`
