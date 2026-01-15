# Hardware Assembly Guide

## Daftar Komponen

| No | Komponen | Jumlah | Spesifikasi | Catatan |
|----|----------|--------|-------------|---------|
| 1  | ESP32 DevKit v1 | 1 | Dual-core, Wi-Fi, Bluetooth | |
| 2  | Sensor Ultrasonik HC-SR04 | 1 | 5V, 40kHz | |
| 3  | Sensor Gas MQ-135 | 1 | 5V, analog output | |
| 4  | Modul GPS NEO-6M | 1 | 3.3V, UART | |
| 5  | Modul GSM SIM800L | 1 | 5V, dengan board | |
| 6  | LCD I2C 16x2 | 1 | 5V, address 0x27 | |
| 7  | Power Bank 10.000mAh | 1 | Output 5V 3A | |
| 8  | Casing tahan air | 1 | Sesuai ukuran komponen | |
| 9  | Kabel jumper | Secukupnya | Male-to-female, female-to-female | |
| 10 | Papan PCB universal | 1 | Untuk penyolderan final | |

## Diagram Rangkaian

### Koneksi ESP32 dengan Sensor

| Pin ESP32 | Komponen | Pin Komponen |
|-----------|----------|--------------|
| GPIO5     | HC-SR04  | Trig         |
| GPIO18    | HC-SR04  | Echo         |
| GPIO34    | MQ-135   | AOUT         |
| GPIO21    | LCD I2C  | SDA          |
| GPIO22    | LCD I2C  | SCL          |
| GPIO13    | NEO-6M   | TX           |
| GPIO15    | NEO-6M   | RX           |
| GPIO16    | SIM800L  | TX           |
| GPIO17    | SIM800L  | RX           |
| GPIO27    | SIM800L  | PWRKEY       |
| 3.3V      | MQ-135, NEO-6M | VCC       |
| 5V        | HC-SR04, LCD I2C, SIM800L | VCC   |
| GND       | Semua komponen | GND      |

### Catatan:
- Modul SIM800L memerlukan catu daya 5V dengan arus minimal 2A.
- Sensor MQ-135 memerlukan pemanasan awal (pre-heat) selama 120 detik sebelum digunakan.

## Langkah Perakitan

### Tahap 1: Persiapan Komponen
1. Siapkan semua komponen sesuai daftar.
2. Uji setiap komponen secara individual untuk memastikan berfungsi.

### Tahap 2: Penyolderan pada PCB
1. Letakkan ESP32 di tengah PCB.
2. Hubungkan setiap komponen ke ESP32 sesuai diagram di atas menggunakan kabel jumper yang disolder.
3. Pastikan koneksi kuat dan tidak ada yang short.

### Tahap 3: Integrasi Casing
1. Tempatkan PCB yang telah dirakit ke dalam casing.
2. Pastikan sensor HC-SR04 dan MQ-135 terpasang di posisi yang tepat (menghadap ke dalam bak sampah).
3. Modul GPS dan antena GSM sebaiknya diletakkan di bagian atas casing untuk mendapatkan sinyal yang baik.

### Tahap 4: Pengujian Awal
1. Hubungkan power bank ke ESP32.
2. Nyalakan sistem dan pantau melalui serial monitor (baudrate 115200).
3. Pastikan semua sensor memberikan pembacaan yang wajar.

## Troubleshooting

| Masalah | Kemungkinan Penyebab | Solusi |
|---------|----------------------|--------|
| ESP32 tidak menyala | Koneksi power longgar | Periksa koneksi VIN dan GND |
| Sensor HC-SR04 tidak bekerja | Koneksi Trig/Echo terbalik | Periksa kembali koneksi |
| LCD tidak menampilkan apa pun | Address I2C salah | Coba address 0x27 atau 0x3F |
| Modul GPS tidak mendapatkan sinyal | Posisi antena buruk | Pindahkan ke area terbuka |
| Modul GSM tidak terkoneksi | APN belum diatur | Atur APN di kode program |

## Pemeliharaan
- Bersihkan sensor ultrasonik secara berkala dari debu.
- Kalibrasi sensor MQ-135 setiap 6 bulan.
- Periksa koneksi kabel secara rutin.
