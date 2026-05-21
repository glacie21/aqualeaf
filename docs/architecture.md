# AquaLeaf Architecture

AquaLeaf dibangun dengan arsitektur modular yang memisahkan:

- `sensors/` untuk pembacaan sensor tanah
- `actuators/` untuk kontrol relay dan pompa
- `network/` untuk WiFi dan MQTT
- `dashboard/` untuk web UI dan REST API
- `storage/` untuk persistence konfigurasi
- `utils/` untuk logger dan helper umum
- `core/` untuk orkestrasi logika utama

Desain ini mengikuti prinsip SOLID dengan tanggung jawab tunggal, mudah diuji, dan mudah dikembangkan.
