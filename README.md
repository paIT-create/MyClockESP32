# MyClockESP32
MyClockESP32 Final
<img width="958" height="1446" alt="IMG_3756" src="https://github.com/user-attachments/assets/4eb9fb63-e4ff-4f92-8c43-42599cf79de3" />

# 🕒 MyClock ESP32 v1.7.x - NeonAction Edition

Precyzyjny zegar WiFi zbudowany na ESP32, charakteryzujący się neonową estetyką interfejsu i zaawansowaną synchronizacją czasu, wyposażony w czujnik temperatury DS18B20.

## 🚀 Główne Cechy
- **Ultra-precyzyjne taktowanie**: Algorytm hybrydowy eliminujący lag WiFi (jitter).
- **Adaptacyjna Jasność**: Płynna regulacja PWM na podstawie czujnika LDR z pełną kalibracją progów.
- **Tryb Nocny Stealth**: Automatyczne wyciszanie dźwięków i opcjonalne wygaszanie wyświetlacza w zadanym przedziale czasu.
- **Web Dashboard**: Nowoczesny panel sterowania (Dark Mode) z neonowymi akcentami, zoptymalizowany pod urządzenia mobilne.
- **Korekta Temperatury**: Intuicyjny suwak sensoryczny z wizualną informacją zwrotną o statusie (Cold/Neutral/Hot).

## 🔧 Parametry Techniczne
- **Rdzeń**: ESP32 (FreeRTOS)
- **Komunikacja**: WiFi (NTP Sync)
- **Wyświetlacz**: 7-segment LED sterowany przez rejestry przesuwne (ISR)
- **Zasilanie**: 5V USB (pobór prądu zależny od jasności)

## 🛠️ Ustawienia Zaawansowane (Web UI)
- Kalibracja LDR (Dark/Bright)
- Korekta Offsetu temperatury (-9.0°C do +9.0°C)
- Definiowanie godzin ciszy nocnej
- Zarządzanie Master Mute i Hourly Chime

---
*Projekt rozwijany hobbystycznie z naciskiem na jakość UX i precyzję działania.*
