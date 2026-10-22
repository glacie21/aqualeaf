# System Flowchart

1. Booting: inisialisasi hardware (relay idle), muat pengaturan dari EEPROM
   (validasi versi + CRC, migrasi format lama), mulai WiFi, NTP, MQTT, dashboard.
2. Koneksi WiFi: pakai kredensial tersimpan; jika gagal/kosong buka AP setup
   dengan captive portal. Setelah tersambung: OTA + mDNS aktif.
3. Baca sensor tanah setiap 2 detik; nilai raw di luar rentang wajar = sensor fault
   (penyiraman otomatis berhenti).
4. Jika kelembaban ≤ threshold low, jam termasuk jadwal, dan proteksi dry-run
   mengizinkan: pompa ON.
5. Pompa OFF saat kelembaban ≥ threshold high, saat batas waktu per siklus
   tercapai, atau saat batas harian habis.
6. Override manual berakhir otomatis setelah 30 menit.
7. Catat riwayat kelembaban setiap 5 menit (24 jam di RAM).
8. Kirim data ke MQTT dan Blynk secara berkala.
9. Web dashboard menyediakan status realtime, grafik, pengaturan, dan REST API.
