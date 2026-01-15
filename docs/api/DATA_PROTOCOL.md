# API Reference & Data Protocol

## Overview
Sistem BINSAI mengirimkan data ke cloud menggunakan dua protokol:
1. **Blynk IoT** (via Wi-Fi) untuk real-time monitoring
2. **SMS** (via GSM) sebagai fallback saat kondisi kritis

## Blynk Datastreams

### Virtual Pins Mapping

| Virtual Pin | Data Type | Range | Description |
|-------------|-----------|-------|-------------|
| V0          | Integer   | 0-100 | Fill percentage |
| V1          | Integer   | 0/255 | LED Red (Full status) |
| V2          | Integer   | 0/255 | LED Orange (Almost Full) |
| V3          | Integer   | 0/255 | LED Yellow (Half) |
| V4          | Integer   | 0/255 | LED Green (Empty) |
| V5          | Double    | 0-400 | Distance in cm |
| V6          | String    | -     | Capacity status text |
| V10         | Integer   | 0-2000| Gas concentration in ppm |
| V11         | Integer   | 0-3   | Priority level (0=Normal, 3=Critical) |
| V12         | String    | -     | Waste type classification |
| V13         | String    | -     | Recommendation for officers |
| V20         | Double    | -     | Latitude |
| V21         | Double    | -     | Longitude |

### Data Update Frequency
- Data sensor diperbarui setiap 2 detik.
- Log penelitian dikirim setiap 60 detik.

## SMS Protocol

### Format Pesan Kritis
[CRITICAL] Bin #{DEVICE_ID} requires immediate attention
Capacity: {fill_percentage}% | NH3: {gas_ppm}ppm
Location: https://maps.google.com/?q={latitude},{longitude}
Priority: URGENT - Organic decomposition detected

text

### Format Pesan Rutin (Opsional)
[INFO] Bin #{DEVICE_ID} status update
Capacity: {fill_percentage}%
Status: {status_text}
Last updated: {timestamp}

text

## Local API (Serial Monitor)

### Debugging Interface
Sistem menampilkan data melalui Serial Monitor (115200 baud) dengan format:
=== BINSAI Telemetry ===
Time: 2025-12-31 23:59:59
Capacity: 75.5% (Distance: 10.2cm)
Gas: 450 ppm (Organic Level 1)
GPS: -7.764200, 110.389300
Status: Almost Full
Priority: Medium

text

### Serial Commands
Dapat mengirim perintah via Serial Monitor untuk konfigurasi:

| Command | Description | Response |
|---------|-------------|----------|
| `RESET` | Reset semua sensor | `OK` atau `ERROR` |
| `CALIBRATE` | Kalibrasi sensor gas | `Calibrating...` lalu `Done` |
| `STATUS` | Status sistem | JSON string dengan semua data |

## Data Storage Format

### Cloud Logs
Data disimpan di Blynk cloud dalam format:

```json
{
  "timestamp": "2025-12-31T23:59:59Z",
  "device_id": "BINSAI_001",
  "capacity_percentage": 75.5,
  "gas_ppm": 450,
  "latitude": -7.764200,
  "longitude": 110.389300,
  "priority": 2,
  "waste_type": "Organic"
}

## 4. Deployment Guide (`docs/deployment/URBAN_DEPLOYMENT.md`)

```markdown
# Urban Deployment Guide

## Prerequisites

### Hardware
- Unit BINSAI yang telah dirakit (minimal 3 unit untuk pilot project)
- Power bank dengan kapasitas minimal 10.000mAh per unit
- SIM card dengan paket SMS (minimal 100 SMS/bulan)

### Software
- Akun Blynk IoT (gratis untuk hingga 5 device)
- Google Maps API key (untuk link lokasi)

### Lokasi
- Area publik dengan lalu lintas tinggi (jalan utama, pasar, sekolah)
- Sinyal Wi-Fi publik (jika tersedia) atau akses seluler yang baik

## Langkah-langkah Deployment

### Tahap 1: Perencanaan
1. **Survei Lokasi**: Identifikasi titik-titik penumpukan sampah.
2. **Koordinasi dengan Pihak Berwenang**: Dinas Kebersihan setempat.
3. **Penjadwalan**: Tentukan waktu deployment yang tidak mengganggu aktivitas publik.

### Tahap 2: Konfigurasi Perangkat
1. **Flash Firmware** ke setiap unit ESP32.
2. **Kalibrasi Sensor**:
   - Sensor ultrasonik: ukur tinggi bak sampah.
   - Sensor gas: lakukan RZero calibration di udara bersih.
3. **Uji Coba** setiap unit selama 24 jam.

### Tahap 3: Instalasi Fisik
1. Pasang unit BINSAI di dalam bak sampah yang sudah ada.
2. Pastikan sensor menghadap ke bawah (untuk HC-SR04) dan ada ventilasi (untuk MQ-135).
3. Kencangkan dengan bracket yang disediakan.

### Tahap 4: Integrasi dengan Sistem Pengelolaan
1. **Blynk Dashboard**:
   - Buat project baru di Blynk IoT.
   - Tambahkan widget sesuai panduan.
   - Undang petugas sebagai user.
2. **SMS Gateway**:
   - Tambahkan nomor petugas ke daftar penerima SMS.
   - Uji kirim SMS alert.

### Tahap 5: Pelatihan Petugas
1. Demonstrasi cara membaca data di Blynk app.
2. Simulasi penanganan alert kritis.
3. Bagikan manual petugas.

## Monitoring & Pemeliharaan

### Harian
- Pantau dashboard Blynk.
- Periksa SMS alert yang masuk.

### Mingguan
- Bersihkan sensor dari debu dan kotoran.
- Periksa koneksi kabel.

### Bulanan
- Kalibrasi ulang sensor gas.
- Ganti power bank jika diperlukan.

## Troubleshooting Lapangan

| Masalah | Penyebab | Solusi |
|---------|----------|--------|
| Tidak ada data di dashboard | Wi-Fi terputus | Periksa koneksi Wi-Fi atau reset ESP32 |
| SMS tidak terkirim | Pulsa habis atau sinyal buruk | Cek pulsa dan pindahkan unit ke lokasi dengan sinyal lebih baik |
| Pembacaan kapasitas tidak akurat | Sensor ultrasonik kotor | Bersihkan permukaan sensor |
| Sensor gas selalu tinggi | Terpapar sumber gas terus-menerus | Pindahkan unit atau atur ulang threshold |

## Evaluasi Kinerja
Setelah 1 bulan deployment, lakukan evaluasi:
1. Hitung jumlah alert yang dikirim.
2. Bandingkan efisiensi pengangkutan sebelum dan setelah BINSAI.
3. Survei kepuasan petugas.

## Skala ke Kota
Untuk implementasi skala kota (100+ unit):
1. Gunakan Blynk Enterprise atau platform IoT lain yang mendukung banyak device.
2. Pertimbangkan untuk menggunakan LoRaWAN untuk mengurangi biaya komunikasi.
3. Integrasikan dengan sistem manajemen sampah yang sudah ada.
