# AquaLeaf Architecture

AquaLeaf dibangun dengan arsitektur modular yang memisahkan:

- `logic/` untuk logika murni tanpa ketergantungan Arduino: state machine pompa,
  aturan threshold & jadwal, proteksi dry-run, konversi sensor, format EEPROM,
  dan buffer riwayat. Waktu dan data masuk sebagai parameter sehingga modul ini
  bisa dites di PC (`pio test -e native`).
- `sensors/` untuk pembacaan sensor tanah
- `actuators/` untuk kontrol relay dan pompa (`PumpController` membungkus
  `PumpStateMachine` dan menggerakkan relay)
- `network/` untuk WiFi (termasuk AP setup + captive DNS), MQTT, OTA dan mDNS
- `dashboard/` untuk web UI dan REST API
- `storage/` untuk persistence pengaturan (EEPROM, format dari `SettingsCodec`)
- `utils/` untuk logger dan helper umum
- `core/` untuk orkestrasi logika utama (`SystemManager`)

Aturan praktis: jika sebuah keputusan bisa diambil tanpa menyentuh hardware,
letakkan di `logic/` dan beri unit test native; modul lain cukup menjadi
penghubung ke hardware dan jaringan.
