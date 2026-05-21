# Wiring Diagram

Komponen utama:

- ESP8266 NodeMCU
- Soil moisture sensor ke pin `A0`
- Relay module ke pin `D1`
- Pompa air terhubung ke relay output
- LED status (opsional) ke pin `D2`

Peringatan:
- Relay module biasanya menggunakan sinyal aktif-LOW.
- Pastikan pompa menggunakan sumber daya terpisah jika diperlukan.
