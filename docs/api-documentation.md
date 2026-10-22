# API Documentation

Semua endpoint kecuali `/api/health` (termasuk halaman dashboard) memerlukan
HTTP Basic Auth dengan kredensial `WEB_ADMIN_USER` / `WEB_ADMIN_PASS` dari
`include/Secrets.h`.

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
  "sensor_fault": false,
  "watering_lockout": false,
  "daily_limit_reached": false,
  "pump_on_today_s": 45,
  "override_remaining_s": 0,
  "uptime_ms": 123456
}
```

- `pump_state`: `0` OFF, `1` ON, `2` COOLDOWN, `4` FAULT (timeout safety)
- `operating_mode`: `0` AUTO, `1` MANUAL
- `sensor_fault`: nilai raw sensor di luar `MOISTURE_RAW_VALID_MIN`–`MOISTURE_RAW_VALID_MAX`;
  penyiraman otomatis dihentikan sampai pembacaan kembali normal
- `watering_lockout`: kelembaban tidak naik setelah `PUMP_MAX_INEFFECTIVE_CYCLES`
  siklus otomatis berturut-turut (tangki kosong / sensor macet). Di-reset lewat
  `POST /api/config`
- `daily_limit_reached` / `pump_on_today_s`: total waktu nyala pompa pada jendela
  24 jam berjalan, dibatasi `PUMP_DAILY_LIMIT_MS` (berlaku juga untuk kontrol manual)
- `override_remaining_s`: sisa waktu override manual sebelum mode kembali otomatis

## POST /api/pump

Request body:

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

Request body:

```json
{
  "threshold_low": 30,
  "threshold_high": 60,
  "auto_mode": true
}
```

Semua field opsional; field yang tidak dikirim mempertahankan nilai aktif.
Threshold di-clamp ke 0–100, ditukar jika `low > high`, dan dijaga berjarak
minimal `MOISTURE_THRESHOLD_MIN_GAP`.

Menyimpan threshold ke EEPROM, mengakhiri override manual, me-reset
`watering_lockout`, dan mengatur mode sesuai `auto_mode`. Response berisi
status terbaru.

## GET /api/health

Returns plain text `OK`. Tidak memerlukan autentikasi.
