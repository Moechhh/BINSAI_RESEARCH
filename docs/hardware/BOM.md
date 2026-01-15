# Bill of Materials (BOM) - Proyek BINSAI
**Sistem Monitoring Bak Sampah Pintar Berbasis IoT**

| No | Komponen Utama | Spesifikasi Teknis | Qty | Harga Satuan (IDR) | Total (IDR) |
|:--:|:---|:---|:--:|:---:|:---:|
| **1** | **Mikrokontroler** | ESP32 DevKit V1 (30 Pin, WiFi + Bluetooth) | 1 | Rp 55.000 | Rp 55.000 |
| **2** | **Sensor Jarak** | Ultrasonic HC-SR04 (2cm - 400cm range) | 1 | Rp 12.000 | Rp 12.000 |
| **3** | **Sensor Gas** | MQ-135 Air Quality Control (NH3, Benzene, Alcohol) | 1 | Rp 18.000 | Rp 18.000 |
| **4** | **Modul Komunikasi** | SIM800L GPRS/GSM Module (Mini V2.0) | 1 | Rp 85.000 | Rp 85.000 |
| **5** | **Modul Lokasi** | GPS NEO-6M Module with Ceramic Antenna | 1 | Rp 65.000 | Rp 65.000 |
| **6** | **Catu Daya Utama** | Power Bank Olike 10.000mAh (22.5W Fast Charge) | 1 | Rp 199.000 | Rp 199.000 |
| **7** | **Kabel Power I** | USB to DC 5V (Barrel Jack/Header) for ESP32 | 1 | Rp 15.000 | Rp 15.000 |
| **8** | **Kabel Power II** | USB to DC 5V (Barrel Jack/Header) for SIM800L | 1 | Rp 15.000 | Rp 15.000 |
| **9** | **Enclosure** | Custom 3D Printed Box / Waterproof Box X6 | 1 | Rp 35.000 | Rp 35.000 |
| **10**| **Konektivitas** | SIM Card (Kuota Data + SMS) | 1 | Rp 15.000 | Rp 15.000 |
| **Total Estimasi Anggaran** | | | | **Rp 514.000** |

---

## Justifikasi Teknis & Analisis Daya

### 1. Kapasitas dan Pengisian (Power Bank Olike 22.5W)
Pemilihan power bank 22.5W dengan kapasitas 10.000 mAh memberikan keunggulan pada durasi operasional (uptime) perangkat di lapangan. Penggunaan power bank komersial menyederhanakan proses *recharging* bagi pengguna awam (petugas kebersihan).

### 2. Distribusi Arus (Dual USB to DC)
Sistem menggunakan dua jalur distribusi daya independen dari port USB power bank. Hal ini bertujuan untuk meminimalisir gangguan induksi arus yang sering terjadi pada modul GSM SIM800L saat melakukan transmisi data (burst current).

---
*Catatan: Harga berdasarkan rata-rata marketplace nasional per Januari 2026.*
