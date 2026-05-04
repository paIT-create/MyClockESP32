# MyClockESP32
MyClockESP32 Final
<img width="958" height="1446" alt="IMG_3756" src="https://github.com/user-attachments/assets/4eb9fb63-e4ff-4f92-8c43-42599cf79de3" />

## 🕒 MyClockESP32 v1.5 – Ultra-Stable Edition
Profesjonalny zegar oparty na układzie ESP32, synchronizowany czasem NTP, wyposażony w czujnik temperatury DS18B20, inteligentną regulację jasności oraz rozbudowany panel konfiguracyjny "Neon-Glow".
## 🚀 Kluczowe cechy

* Synchronizacja NTP: Precyzyjny czas pobierany z serwerów GUM (tempus1.gum.gov.pl) z automatyczną obsługą stref czasowych i czasu letniego (DST).
* Dual-Core Processing: Rozdzielenie zadań na dwa rdzenie (WiFi/Web na Core 0, Logika/Wyświetlacz na Core 1) zapewnia płynność pracy bez migotania LED.
* Panel Konfiguracyjny Web: Nowoczesny interfejs w stylu "Neon-Glow" (zoptymalizowany pod kątem pamięci PROGMEM) dostępny przez przeglądarkę.
* Inteligentna Jasność (LDR): Automatyczna regulacja jasności na podstawie czujnika światła z funkcją kalibracji progów "Live View" w panelu WWW.
* Zaawansowany Budzik: Harmonogram tygodniowy, wybór melodii oraz regulacja głośności buzzera pasywnego (PWM).
* Niezawodność: Wbudowany system Watchdog, mechanizm automatycznego wznawiania połączenia WiFi oraz "tryb nocny" dla dźwięków systemowych (22:00–06:00).

## 🛠️ Technologie i Optymalizacje

* FreeRTOS: Wielowątkowość i bezpieczne zarządzanie zasobami.
* Captive Portal & DNS Server: Łatwa konfiguracja WiFi w nowym otoczeniu (automatyczne przekierowanie na stronę konfiguracji).
* Chunked Web Transfer: Stabilne przesyłanie interfejsu WWW w małych pakietach (1KB), co zapobiega przepełnieniu bufora TCP.
* Kalibracja Czujników: Możliwość ustawienia offsetu temperatury (DS18B20) oraz progów LDR bezpośrednio w panelu konfiguracyjnym.

## 📂 Struktura Projektu

* MyClockESP32.ino – Główny kod programu (logika systemowa).
* web_ui.h – Interfejs użytkownika (HTML/CSS/JS) przechowywany w pamięci Flash.
* README.md – Dokumentacja projektu.

## 🔧 Szybka konfiguracja

   1. Podłącz zegar do zasilania.
   2. Połącz się z siecią WiFi o nazwie ESP32-Clock-XXXX.
   3. Jeśli okno konfiguracji nie pojawi się automatycznie, wpisz w przeglądarce 192.168.4.1.
   4. Skonfiguruj swoją sieć domową i ciesz się precyzyjnym czasem.
