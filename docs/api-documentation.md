# API Documentation

Semua endpoint kecuali `/api/health` (termasuk halaman dashboard) memerlukan
HTTP Basic Auth dengan kredensial `WEB_ADMIN_USER` / `WEB_ADMIN_PASS` dari
`include/Secrets.h`. Error dikembalikan sebagai `{"error": "<pesan>"}`.

## GET /api/status

Returns current sensor and system state as JSON.

```json
{
  "moisture_pct": 42.5,
  "moisture_raw": 633,
  "pump_state": 0,
  "pump_cycles": 3,
  "operating_mode": 0,
  "system_state": 2,
  "network_status": 2,
  "mqtt_status": 2,
  "auto_mode": true,
  "manual_override": false,
  "threshold_low": 30,
  "threshold_high": 60,
  "allowed_hours_mask": 16777215,
  "sensor_fault": false,
  "watering_lockout": false,
  "daily_limit_reached": false,
  "pump_on_today_s": 45,
  "override_remaining_s": 0,
  "weak_web_password": false,
  "raw_dry": 820,
  "raw_wet": 380,
  "time_synced": true,
  "local_time": "14:05",
  "schedule_allows_now": true,
  "wifi_ssid": "HomeNet",
  "wifi_ip": "192.168.1.42",
  "wifi_rssi": -61,
  "ap_active": false,
  "hostname": "aqualeaf.local",
  "firmware": "1.0.0",
  "uptime_ms": 123456
}
```

- `pump_state`: `0` OFF, `1` ON, `2` COOLDOWN, `4` FAULT (timeout safety)
- `operating_mode`: `0` AUTO, `1` MANUAL
- `system_state`: `0` BOOT, `2` RUNNING, `5` OTA (update firmware berjalan)
- `network_status`: `0` DISCONNECTED, `1` CONNECTING, `2` CONNECTED, `3` AP_MODE
- `mqtt_status`: `0` DISCONNECTED, `2` CONNECTED
- `allowed_hours_mask`: bit *n* = penyiraman otomatis boleh dimulai pada jam *n*
  (waktu lokal `TIME_ZONE`). `16777215` (`0xFFFFFF`) = sepanjang hari
- `time_synced` / `local_time`: jam perangkat dari NTP. Selama belum tersinkron,
  jadwal tidak diberlakukan (`schedule_allows_now` selalu `true`)
- `sensor_fault`: nilai raw sensor di luar `MOISTURE_RAW_VALID_MIN`–`MOISTURE_RAW_VALID_MAX`;
  penyiraman otomatis dihentikan sampai pembacaan kembali normal
- `watering_lockout`: kelembaban tidak naik setelah `PUMP_MAX_INEFFECTIVE_CYCLES`
  siklus otomatis berturut-turut (tangki kosong / sensor macet). Di-reset lewat
  `POST /api/config` atau kalibrasi ulang
- `daily_limit_reached` / `pump_on_today_s`: total waktu nyala pompa pada jendela
  24 jam berjalan, dibatasi `PUMP_DAILY_LIMIT_MS` (berlaku juga untuk kontrol manual)
- `override_remaining_s`: sisa waktu override manual sebelum mode kembali otomatis
- `weak_web_password`: `WEB_ADMIN_PASS` masih default atau kurang dari 8 karakter
- `raw_dry` / `raw_wet`: titik kalibrasi sensor yang aktif

## GET /api/history

Riwayat kelembaban 24 jam terakhir (disimpan di RAM, hilang saat reboot).

```json
{
  "interval_s": 300,
  "age_s": 42,
  "moisture": [41.2, 40.9, null, 55.3],
  "watered": "0001"
}
```

- `moisture`: urut dari terlama ke terbaru; `null` = sensor fault saat sampel diambil
- `watered`: satu karakter per sampel, `1` = pompa sempat menyala pada interval itu
- `age_s`: umur sampel terbaru. Waktu sampel ke-*i* ≈ sekarang −
  ((n − 1 − i) × `interval_s` + `age_s`) detik

## Endpoint POST

Semua endpoint POST wajib memakai header `Content-Type: application/json`;
selain itu server membalas `415`. Ini mencegah CSRF: form HTML dari situs lain
tidak bisa mengirim JSON tanpa preflight CORS, dan server tidak menjawab
preflight tersebut.

```bash
curl -u admin:PASSWORD -H "Content-Type: application/json" \
     -d '{"pump_on":true}' http://aqualeaf.local/api/pump
```

## POST /api/pump

```json
{ "pump_on": true }
```

Kontrol manual mengaktifkan override: penyiraman otomatis berhenti selama
`MANUAL_OVERRIDE_TIMEOUT_MS` (default 30 menit, dihitung dari perintah manual
terakhir) atau sampai konfigurasi disimpan lagi lewat `POST /api/config`.
Permintaan ON ditolak selama pompa dalam COOLDOWN atau FAULT, atau jika batas
waktu nyala harian sudah tercapai. Response berisi status terbaru
(format sama dengan `/api/status`).

## POST /api/config

```json
{
  "threshold_low": 30,
  "threshold_high": 60,
  "auto_mode": true,
  "allowed_hours_mask": 393408
}
```

Semua field opsional; field yang tidak dikirim mempertahankan nilai aktif.
Threshold di-clamp ke 0–100, ditukar jika `low > high`, dan dijaga berjarak
minimal `MOISTURE_THRESHOLD_MIN_GAP`. `allowed_hours_mask` harus mengaktifkan
minimal satu jam (`400` jika tidak). Contoh di atas: jam 06, 07, 17, 18.

Menyimpan pengaturan ke EEPROM, mengakhiri override manual, me-reset
`watering_lockout`, dan mengatur mode sesuai `auto_mode`. Response berisi
status terbaru.

## POST /api/calibration

Salah satu dari:

```json
{ "point": "dry" }
{ "point": "wet" }
{ "raw_dry": 820, "raw_wet": 380 }
{ "reset": true }
```

- `point`: pakai pembacaan raw terakhir sebagai titik kering (sensor di udara)
  atau basah (sensor di air)
- `raw_dry` / `raw_wet`: set nilai langsung (field yang tidak dikirim tetap)
- `reset`: kembali ke `MOISTURE_RAW_DRY` / `MOISTURE_RAW_WET` dari `Config.h`

Kedua titik harus berada dalam `MOISTURE_RAW_VALID_MIN`–`MOISTURE_RAW_VALID_MAX`
dan `raw_dry` harus lebih besar dari `raw_wet` minimal
`MOISTURE_CALIBRATION_MIN_SPAN`; jika tidak, `400`. Disimpan ke EEPROM.

## POST /api/wifi

```json
{ "ssid": "HomeNet", "password": "rahasia123" }
```

`ssid` 1–32 karakter; `password` kosong (jaringan terbuka) atau 8–63 karakter.
Membalas `202 {"status":"connecting"}` lalu perangkat mencoba tersambung;
kredensial disimpan di flash. Jika request dikirim lewat AP setup, koneksi ke
browser bisa terputus saat perangkat pindah jaringan. Jika gagal tersambung,
AP setup tetap/kembali aktif.

## GET /api/health

Returns plain text `OK`. Tidak memerlukan autentikasi.

## MQTT

Topic dibangun per perangkat: `aqualeaf/<device-id>/<suffix>`.
`<device-id>` adalah MAC address huruf kecil tanpa `:` (mis. `a4cf12b3c4d5`).
Base topic lengkap tercetak di Serial Monitor saat boot (`MQTT base topic: ...`).

| Topic | Isi |
|---|---|
| `aqualeaf/<id>/sensors` | Telemetry JSON (retained) setiap `TASK_MQTT_PUB_INTERVAL_MS`: data sensor, status pompa, threshold, flag safety, `uptime_ms`, `wifi_connected`, `mqtt_connected` |
| `aqualeaf/<id>/status` | `online` saat tersambung; `offline` dikirim broker (Last Will) jika perangkat putus. Retained |

Semua pesan dikirim dengan QoS 0. Topic unik hanya mencegah data antar-perangkat
tercampur. Di broker publik (default `broker.hivemq.com`, tanpa TLS) siapa pun
yang subscribe ke `aqualeaf/#` tetap bisa membacanya; gunakan broker pribadi
dengan `MQTT_USERNAME` / `MQTT_PASSWORD` untuk data yang tidak boleh terbuka.
