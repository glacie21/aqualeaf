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
  "uptime_ms": 123456
}
```

- `pump_state`: `0` OFF, `1` ON, `2` COOLDOWN, `4` FAULT (timeout safety)
- `operating_mode`: `0` AUTO, `1` MANUAL

## POST /api/pump

Request body:

```json
{ "pump_on": true }
```

Kontrol manual mengaktifkan override: penyiraman otomatis berhenti sampai
konfigurasi disimpan lagi lewat `POST /api/config`. Permintaan ON ditolak
selama pompa dalam COOLDOWN atau FAULT. Response berisi status terbaru
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

Menyimpan threshold ke EEPROM, mengakhiri override manual, dan mengatur mode
sesuai `auto_mode`. Response berisi status terbaru.

## GET /api/health

Returns plain text `OK`. Tidak memerlukan autentikasi.
