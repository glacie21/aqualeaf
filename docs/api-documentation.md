# API Documentation

## GET /api/status

Returns current sensor and system state as JSON.

## POST /api/pump

Request body:

```json
{ "pump_on": true }
```

## POST /api/config

Request body:

```json
{
  "threshold_low": 30,
  "threshold_high": 60,
  "auto_mode": true
}
```

## GET /api/health

Returns plain text `OK`.
