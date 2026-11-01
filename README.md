#  Smart Box Paket

> **"Paket dateng, pemilik sibuk? Tenang, box-nya yang ngurus."** 

Sistem kotak paket pintar berbasis **ESP32** — buka-tutup pintu via **keypad**, remote control lewat **Telegram Bot**, dilengkapi **LCD**, **LED**, **buzzer**, dan **counter paket harian**. Cocok buat rumah kos, apartemen, atau siapa pun yang sering nggak di rumah pas kurir dateng. 🚪📬

---

##  Fitur

-  **Dual Password** — kurir (`1234`) & pemilik (`112233`)
-  **Keypad 4x4** — `#` verifikasi, `*` reset/tutup paksa
- 📟**LCD I2C 16x2** — status & input (disensor `*`)
-  **LED Indikator** — Merah = terkunci, Hijau = terbuka
-  **Buzzer** — beep tombol & nada sukses/gagal
-  **Servo / Relay Solenoid** — aktuator pintu
-  **Auto-Lock** — nutup sendiri setelah 20 detik
-  **Telegram Bot** — `/buka [password]`, `/status`, `/log`, `/start`
-  **Counter Paket** — auto-reset berkala
-  **WiFi + Offline Mode** — tetep jalan walau WiFi mati

---

##  Pin Mapping

| Komponen | Pin ESP32 |
|---|---|
| LCD VCC | 3V3 |
| LCD GND | GND |
| LCD SDA | GPIO 21 |
| LCD SCL | GPIO 22 |
| Keypad R1–R4 | GPIO 13, 12, 14, 27 |
| Keypad C1–C4 | GPIO 26, 25, 33, 32 |
| LED Merah (+) | GPIO 2 |
| LED Hijau (+) | GPIO 4 |
| Buzzer (+) | GPIO 15 |
| Servo PWM / Relay IN | GPIO 5 |
| GND semua | GND |

>  **Hardware asli:** Relay **Active-LOW** → `LOW` = solenoid narik = pintu **TERBUKA**. Di Wokwi pakai servo (`0°` tutup, `90°` buka).

---

## 🎮 Cara Pakai

**Keypad (Kurir / Pemilik):**
Ketik password → tekan # → pintu kebuka → auto-lock 20 detik
Tekan * saat terbuka = tutup paksa
Tekan * saat terkunci = reset input

text

**Telegram Bot:**

| Perintah | Fungsi |
|---|---|
| `/start` | Bantuan |
| `/status` | Cek status pintu |
| `/log` | Jumlah paket hari ini |
| `/buka 112233` | Buka dari jauh (owner only) |

**Serial Monitor (testing):**
buka:112233 → buka pintu
status → cek status
/log → cek counter
help → bantuan

text

---

##  Konfigurasi

Edit di `sketch.ino`:

```cpp
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define BOT_TOKEN     "8607033053:AAH..."
#define OWNER_CHAT_ID "7913136331"

const String password_kurir   = "1234";
const String password_pemilik = "112233";

const unsigned long DURASI_BUKA    = 20000;  // auto-lock 20s
const unsigned long RESET_INTERVAL = 500000; // reset counter
 Library
Install via Arduino IDE → Library Manager:

LiquidCrystal I2C

Keypad

ESP32Servo

UniversalTelegramBot

ArduinoJson

Di Wokwi otomatis ke-load dari libraries.txt.

🧪 Simulasi Wokwi
Buka wokwi.com → New Project → ESP32

Copy diagram.json → tab diagram.json

Copy sketch.ino → tab sketch.ino

Copy libraries.txt → tab libraries.txt

Tekan ▶️ Start Simulation

Buka Serial Monitor buat log

 Wokwi pakai Wokwi-GUEST (no password), Telegram tetap konek via gateway internet.

Struktur File
text
📦 Smart-Box-Paket/
├── diagram.json         # Wiring Wokwi
├── sketch.ino           # Kode utama (Servo, Wokwi)
├── final ino.txt        # Kode hardware asli (Relay)
├── Alur.html            # Diagram alur wiring (SVG)
├── libraries.txt        # Library Wokwi
├── wokwi-project.txt    # Link project Wokwi
└── README.md            # File ini 
 Troubleshooting
Masalah	Solusi
LCD blank / kotak	Cek alamat I2C (0x27 / 0x3F)
WiFi gagal	Pastikan 2.4GHz, cek SSID/password
Bot nggak respon	Cek BOT_TOKEN, OWNER_CHAT_ID, setCACert
Relay nggak nyala	Ingat Active-LOW → LOW = aktif
Keypad nggak kebaca	Cek urutan rowPins & colPins
Servo gerak aneh	Kasih power eksternal 5V


 Reed switch deteksi pintu

Log ke SD card / Google Sheets

 RFID buat kurir

 Web dashboard monitoring
