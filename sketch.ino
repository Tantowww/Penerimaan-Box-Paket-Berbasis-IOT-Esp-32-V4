#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ================== KONFIGURASI WIFI & TELEGRAM ==================
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define BOT_TOKEN "8607033053:AAH7uiJWaksgINgBARCIZHaGtoAfQMarsHU"
#define OWNER_CHAT_ID "7913136331"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
bool wifiConnected = false;
unsigned long botLastTime = 0;
const unsigned long BOT_SCAN_INTERVAL = 1000; // cek pesan setiap 1 detik

// ================== LCD ==================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================== KEYPAD ==================
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ================== PIN ==================
const int pinLedMerah = 2;
const int pinLedHijau = 4;
const int pinBuzzer   = 15;
const int pinServo    = 5;

Servo servo;

// ================== PASSWORD ==================
const String password_kurir   = "1234";
const String password_pemilik = "112233";

// ================== STATUS ==================
String inputPassword = "";
bool pintuTerbuka = false;
unsigned long waktuBuka = 0;
const unsigned long DURASI_BUKA = 20000; // 20 detik

// ================== COUNTER PAKET ==================
unsigned int paketHariIni = 0;
unsigned long lastReset = 0;
const unsigned long RESET_INTERVAL = 500000; // 1 menit demo

// ============== PROTOTYPE ==============
void tampilkanInput();
void verifikasiPassword();
void bukaPintu();
void tutupPintu();
void kirimNotifTelegram(String pesan);
void handleNewMessages(int numNewMessages);

void setup() {
  Serial.begin(115200);
  pinMode(pinLedMerah, OUTPUT);
  pinMode(pinLedHijau, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);

  digitalWrite(pinLedMerah, HIGH);
  digitalWrite(pinLedHijau, LOW);
  noTone(pinBuzzer);

  servo.attach(pinServo);
  servo.write(0);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0); lcd.print("Smart Box Paket");
  lcd.setCursor(0,1); lcd.print("Connect WiFi...");

  // Coba konek WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi connected");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Penting untuk koneksi aman
    bot.sendMessage(OWNER_CHAT_ID, "🚀 Smart Box Paket online. Siap digunakan.", "");
    lcd.setCursor(0,1); lcd.print("WiFi OK          ");
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi GAGAL. Mode offline.");
    lcd.setCursor(0,1); lcd.print("Offline Mode     ");
  }
  delay(1500);
  lastReset = millis();
  tampilkanInput();
}

void loop() {
  // --- Reset counter bila waktunya ---
  if (millis() - lastReset >= RESET_INTERVAL) {
    paketHariIni = 0;
    lastReset = millis();
    Serial.println("[LOG] Counter harian direset.");
    if (wifiConnected) bot.sendMessage(OWNER_CHAT_ID, "📅 Counter direset.", "");
  }

  // --- Polling pesan Telegram ---
  if (wifiConnected && (millis() - botLastTime >= BOT_SCAN_INTERVAL)) {
    int numNew = bot.getUpdates(bot.last_message_received + 1);
    while (numNew) {
      Serial.println("Pesan baru: " + String(numNew));
      handleNewMessages(numNew);
      numNew = bot.getUpdates(bot.last_message_received + 1);
    }
    botLastTime = millis();
  }

  // --- Perintah dari Serial Monitor (simulasi remote) ---
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "help") {
      Serial.println("Perintah: buka:<password> | status | /log");
    } else if (cmd.startsWith("buka:")) {
      String pass = cmd.substring(5);
      if (pass == password_pemilik && !pintuTerbuka) {
        bukaPintu();
        Serial.println("[OK] Pintu dibuka (remote)");
      } else if (pintuTerbuka) {
        Serial.println("[INFO] Pintu sudah terbuka");
      } else {
        Serial.println("[ERROR] Password salah");
      }
    } else if (cmd == "status") {
      Serial.println("Status: " + String(pintuTerbuka ? "TERBUKA" : "TERKUNCI"));
    } else if (cmd == "/log") {
      Serial.println("[LOG] Paket hari ini: " + String(paketHariIni));
    } else {
      // Anggap simulasi perintah Telegram
      String text = cmd;
      text.toLowerCase();
      if (text == "/start") Serial.println("Bot siap.");
      else if (text == "/status") Serial.println("Status: " + String(pintuTerbuka ? "TERBUKA" : "TERKUNCI"));
      else if (text == "/log") Serial.println("[LOG] Paket hari ini: " + String(paketHariIni));
      else if (text.startsWith("/buka")) {
        int sp = text.indexOf(' ');
        if (sp != -1) {
          String p = text.substring(sp+1);
          if (p == password_pemilik && !pintuTerbuka) { bukaPintu(); Serial.println("✅ Dibuka."); }
          else Serial.println("❌ Gagal.");
        }
      }
    }
  }

  // --- Keypad ---
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key: '"); Serial.print(key); Serial.println("'");

    if (pintuTerbuka) {
      // Saat pintu terbuka, hanya '*' yang diterima untuk menutup paksa
      if (key == '*') {
        Serial.println("Tutup paksa via *");
        tutupPintu();
      }
      // Abaikan semua tombol lain
    } else {
      // Saat pintu terkunci
      if (key == '#') {
        verifikasiPassword();
      } else if (key == '*') {
        inputPassword = "";
        tampilkanInput();
      } else if (key >= '0' && key <= '9') {
        inputPassword += key;
        tampilkanInput();
      }
    }
  }

  // --- Timer tutup otomatis ---
  if (pintuTerbuka && (millis() - waktuBuka >= DURASI_BUKA)) {
    tutupPintu();
  }
}

void tampilkanInput() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Masukkan Password");
  lcd.setCursor(0,1);
  for (int i=0; i<inputPassword.length(); i++) lcd.print("*");
  lcd.print("                ");
}

void verifikasiPassword() {
  if (inputPassword == password_kurir || inputPassword == password_pemilik) {
    bukaPintu();
    inputPassword = "";
  } else {
    digitalWrite(pinLedMerah, HIGH);
    digitalWrite(pinLedHijau, LOW);
    lcd.clear(); lcd.setCursor(0,0); lcd.print("Password Salah!");
    tone(pinBuzzer, 1000, 500);
    delay(1000);
    noTone(pinBuzzer);
    inputPassword = "";
    tampilkanInput();
  }
}

void bukaPintu() {
  if (pintuTerbuka) return;
  pintuTerbuka = true;
  waktuBuka = millis();
  paketHariIni++;

  digitalWrite(pinLedMerah, LOW);
  digitalWrite(pinLedHijau, HIGH);
  servo.write(90);

  // Bunyi 2x pendek
  for (int i=0; i<2; i++) {
    tone(pinBuzzer, 1500, 100);
    delay(150);
    noTone(pinBuzzer);
    delay(100);
  }

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Pintu Terbuka!");
  lcd.setCursor(0,1); lcd.print("Paket diterima");

  kirimNotifTelegram("Ada paket Boss");
}

void tutupPintu() {
  if (!pintuTerbuka) return;
  pintuTerbuka = false;
  servo.write(0);
  digitalWrite(pinLedMerah, HIGH);
  digitalWrite(pinLedHijau, LOW);

  tone(pinBuzzer, 1500, 150);
  delay(200);
  noTone(pinBuzzer);

  lcd.clear();
  tampilkanInput();

  kirimNotifTelegram("Paket Sudah di terima");
}

void kirimNotifTelegram(String pesan) {
  if (wifiConnected) {
    bot.sendMessage(OWNER_CHAT_ID, pesan, "");
  } else {
    Serial.println("[NOTIF_TELEGRAM] " + pesan);
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    text.toLowerCase();

    if (chat_id != OWNER_CHAT_ID) {
      bot.sendMessage(chat_id, "⛔ Akses ditolak.", "");
      continue;
    }

    if (text == "/start") {
      bot.sendMessage(chat_id, "Halo Pemilik! Gunakan:\n/buka [password] – buka pintu\n/status – cek pintu\n/log – lihat jumlah paket", "");
    } else if (text == "/status") {
      bot.sendMessage(chat_id, "Status pintu: " + String(pintuTerbuka ? "TERBUKA" : "TERKUNCI"), "");
    } else if (text == "/log") {
      bot.sendMessage(chat_id, "📦 Paket hari ini: " + String(paketHariIni), "");
    } else if (text.startsWith("/buka")) {
      int spasi = text.indexOf(' ');
      if (spasi != -1) {
        String pass = text.substring(spasi + 1);
        pass.trim();
        if (pass == password_pemilik) {
          if (!pintuTerbuka) {
            bukaPintu();
            bot.sendMessage(chat_id, "✅ Pintu dibuka.", "");
          } else {
            bot.sendMessage(chat_id, "⚠️ Pintu sudah terbuka.", "");
          }
        } else {
          bot.sendMessage(chat_id, "❌ Password salah.", "");
        }
      } else {
        bot.sendMessage(chat_id, "Format: /buka [password]", "");
      }
    } else {
      bot.sendMessage(chat_id, "Perintah tidak dikenal. /start untuk bantuan.", "");
    }
  }
}