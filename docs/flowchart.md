# System Flowchart

1. Booting: inisialisasi hardware, WiFi, MQTT, dashboard.
2. Koneksi WiFi: auto reconnect dan fallback AP jika gagal.
3. Baca sensor tanah setiap periode.
4. Jika kelembaban di bawah threshold low, pompa ON.
5. Jika kelembaban di atas threshold high, pompa OFF.
6. Kirim data ke MQTT dan Blynk secara berkala.
7. Web dashboard menyediakan status realtime dan REST API.
