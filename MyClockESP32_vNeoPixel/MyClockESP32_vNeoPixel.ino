/*
  ESP32 Clock + Thermometer (DS18B20)
  Ultra-Stable & Versatile Edition v1.9 (2026-06)
  LED 4x7seg (CC/CA) via 74HC595 + (ULN2803/PNP) or NEOPIXEL (144x SK6812)
  ✔ Automatyczna separacja zasobów LED 7-Seg vs NeoPixel (SK6812)
  ✔ Pełna unifikacja logiki LDR, Alarmów i statusu WebUI

  - Display refresh in hardware timer ISR (no blanking)
  - DS18B20 handled asynchronously
  - Time with DST via configTzTime + getLocalTime
  - WiFi configuration via AutoConnect (retainPortal)
  - OTA via AutoConnectOTA
  - Status endpoint: /status
  - Config UI: /config
  - mDNS: esp32-clock-XXXX.local
  - Brightness: OE pin PWM + optional auto brightness from LDR (ADC)
  - Web UI - LDR Calibration and Temperature Offset
  - Web UI - Alarm handler (weekly schedule)
  
  ✔ ESP32 core 2.0.17  
  ✔ AutoConnect 1.4.2
    *błąd aktualizacji OTA z iOS/Android 
    (- błąd w  pliku AutoConnectElementBasisImpl.h: accept="application/octet-steam" zamiast "octet-stream" 
    powoduje brak możliwości wyboru pliku z firmware)
  ✔ PageBuilder 1.5.6
*/
#include <Arduino.h>

// WiFi / Portal / OTA
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <AutoConnect.h>
#include <AutoConnectOTA.h>
#include <ESPmDNS.h>
#include "web_ui.h"

// Time
#include <time.h>
#include "esp_sntp.h"  // Niezbędne do zmiany interwału synchronizacji i callbacka

// DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

// Preferences & WDT
#include <Preferences.h>
#include <esp_task_wdt.h>

// -----------------------------------------------------------------------------
//                 --- KONFIGURACJA WERSJI HARDWARE ---
// -----------------------------------------------------------------------------
#define FW_VERSION "Beta*[NEOPIXEL/CC/CA]202606.1.9.0-Versatile NeonAction"

// =============================================================================
// SELEKTOR ARCHITEKTURY - WYBÓR WYŚWIETLACZA
// =============================================================================
#define DISPLAY_TYPE_NEOPIXEL  // Odkomentuj dla SK6812, zakomentuj dla klasycznych 7-seg LED

#ifndef DISPLAY_TYPE_NEOPIXEL
#define DISPLAY_COMMON_CATHODE true  // Zmień na false dla CA (PNP)
// Logika dla segmentów (74HC595), w CA segment świeci przy stanie niskim (odwrócenie fontu)
#define SEG_MASK(s) (DISPLAY_COMMON_CATHODE ? (s) : ~(s))
#endif
// Zmień na false dla wersji bez głośnika (false wyłącza sekcję Budzika i dźwięków w WebUI)
#define HAS_BUZZER true

// -----------------------------------------------------------------------------
// Pinout & Specyfika Sprzętowa wyświetlaczy
// -----------------------------------------------------------------------------
static const int PIN_ONEWIRE = 15;  // DS18B20 data
static const int PIN_LDR_ADC = 34;  // LDR analog input (ADC1)
static const int PIN_LED = 2;       // On-board LED
static const int PIN_BUZZER = 4;    // BUZZER
static const int BUZZER_CH = 2;     // Kanał PWM dla buzzera

#ifdef DISPLAY_TYPE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
// 74HCT125 jako konwerter poziomów z funkcją blokady bramki (OE aktywna LOW)
static const int PIN_NEO_DATA = 23;
static const int PIN_NEO_OE = 22;
static const int NUM_LEDS = 144;
Adafruit_NeoPixel strip(NUM_LEDS, PIN_NEO_DATA, NEO_GRB + NEO_KHZ800);
// --- PALETA BARW DLA ARCHITEKTURY NEOPIXEL ---
// Każdy element ma teraz swoje własne niezależne bajty R, G, B
// 1. Godziny i Minuty (Domyślnie: Twój luksusowy złoty)
//uint8_t g_colTimeR = 255, g_colTimeG = 100, g_colTimeB = 0;
uint8_t g_colTimeR = 255, g_colTimeG = 0, g_colTimeB = 0;
// 2. Sekundy (Domyślnie: Ciepły pomarańcz, lekko zbliżony do złotego)
//uint8_t g_colSecR = 255, g_colSecG = 40, g_colSecB = 0;
uint8_t g_colSecR = 46, g_colSecG = 49, g_colSecB = 56;
// 3. Dwukropki (Domyślnie: Głęboka, czysta czerwień)
//uint8_t g_colColonR = 255, g_colColonG = 0, g_colColonB = 0;
uint8_t g_colColonR = 40, g_colColonG = 215, g_colColonB = 70;
// 4. Temperatura (Domyślnie: Mroźny, neonowy błękit/cyan)
uint8_t g_colTempR = 0, g_colTempG = 180, g_colTempB = 255;
// 5. Ambient Light (Domyślnie: Klasyczny, uspokajający ciemnoniebieski)
//uint8_t g_colAmbientR = 0, g_colAmbientG = 0, g_colAmbientB = 50;
uint8_t g_colAmbientR = 2, g_colAmbientG = 2, g_colAmbientB = 110;
// Flaga efektów specjalnych (0 - stałe kolory, 1 - tęcza na sekundniku, 2 - gradient)
uint8_t g_ledEffectMode = 0;
bool g_tempColorAuto = true;  // Domyślnie włączona automatyczna barwa temperatury
// Tablica przechowująca aktualny, płynny stan R, G, B dla 42 diod sekundnika (pozycje 4 i 5)
// 42 diody x 3 składowe (R,G,B) = 126 bajtów
uint8_t g_fadeBuffer[42][3];
#else
// Zasoby przypisane WYŁĄCZNIE do klasycznego wyświetlacza LED 7-seg
static const int PIN_595_CLK = 12;
static const int PIN_595_LATCH = 13;
static const int PIN_595_DATA = 14;
static const int PIN_595_OE = 27;
// Digit select pins (to ULN2803 inputs) for 4 digits (common cathode)
static const int PIN_DIGIT_0 = 32;
static const int PIN_DIGIT_1 = 33;
static const int PIN_DIGIT_2 = 25;
static const int PIN_DIGIT_3 = 26;
// Automatyczne ustawienie logiki cyfr na podstawie typu wyświetlacza
// CC (NPN/Direct) wysokiego (HIGH), CA (PNP) potrzebuje stanu niskiego (LOW)
static const bool DIGIT_ENABLE_HIGH = DISPLAY_COMMON_CATHODE;
#endif
// -----------------------------------------------------------------------------
// Czcionka 7-segmentowa (Wspólna baza znaków dla obu architektur)
// -----------------------------------------------------------------------------
static const uint8_t SEG_A = 1 << 0;
static const uint8_t SEG_B = 1 << 1;
static const uint8_t SEG_C = 1 << 2;
static const uint8_t SEG_D = 1 << 3;
static const uint8_t SEG_E = 1 << 4;
static const uint8_t SEG_F = 1 << 5;
static const uint8_t SEG_G = 1 << 6;
static const uint8_t SEG_DP = 1 << 7;
// Bit layout: 0bDPGFEDCBA
static const uint8_t FONT_HEX[16] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          // 0
  SEG_B | SEG_C,                                          // 1
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                  // 2
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                  // 3
  SEG_B | SEG_C | SEG_F | SEG_G,                          // 4
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                  // 5
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,          // 6
  SEG_A | SEG_B | SEG_C,                                  // 7
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,  // 8
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,          // 9
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,          // A
  SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,                  // b
  SEG_A | SEG_D | SEG_E | SEG_F,                          // C
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,                  // d
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                  // E
  SEG_A | SEG_E | SEG_F | SEG_G                           // F
};

static const uint8_t FONT_MINUS = SEG_G;
static const uint8_t FONT_BLANK = 0;
static const uint8_t FONT_DEGREE = SEG_A | SEG_B | SEG_F | SEG_G;
static const uint8_t FONT_C = SEG_A | SEG_D | SEG_E | SEG_F;
static const uint8_t FONT_T = SEG_D | SEG_E | SEG_F | SEG_G;

// -----------------------------------------------------------------------------
// Globalny stan urządzenia
// -----------------------------------------------------------------------------
#ifndef DISPLAY_TYPE_NEOPIXEL
volatile uint8_t g_displaySeg[4] = { 0, 0, 0, 0 };
uint8_t g_displayNext[4] = { 0, 0, 0, 0 };
#endif

char g_lastSyncTimeStr[32] = "Brak synchronizacji";
volatile int g_hour = 0;
volatile int g_minute = 0;
volatile int g_second = 0;
volatile float g_tempC = NAN;
float g_tempOffset = 0.0f;

volatile bool g_showTemp = false;
volatile bool g_showBootId = true;
volatile bool g_timeValid = false;
volatile bool g_tempValid = false;
bool g_tempErrorBeepDone = false;  // Flaga zapobiegająca zapętleniu dźwięku awarii czujnika temperatury

// WiFi watchdog & OTA
volatile bool g_forceWifiDot = false;
bool wifiWasConnected = false;
volatile bool g_otaActive = false;

// Brightness & LDR
Preferences prefs;
volatile bool g_autoBrightness = true;
volatile uint8_t g_brightness = 128;
int g_rawDark = 3900;
int g_rawBright = 900;
bool g_nightLedOff = false;

// Serwer Web, Portal i DNS
WebServer server(80);
AutoConnect portal(server);
AutoConnectConfig portalConfig;
AutoConnectOTA ota;
DNSServer dnsServer;
const byte DNS_PORT = 53;

String g_hostName;
String g_deviceId;
char id[5] = { 0 };  // 4 hex + '\0'

// Obsługa czujnika Dallas
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature sensors(&oneWire);

// Sekcja audio i preferencji nocnych
int g_buzzerVol = 50;                     // Domyślnie 50%
int g_alarmH = 7, g_alarmM = 0;           // Domyślnie 7:00
bool g_hourlyChime = true;                // Domyślnie włączony
bool g_alarmActive = false;               // Domyślnie wyłączony
bool g_masterMute = false;                // Całkowite wyciszenie - wyłączone
int g_alarmMelody = 0;                    // Wybór melodii (0 - klasyk, 1 - radosna, 2 - syrena
uint8_t g_alarmDays = 127;                // Bity: 0-Niedz, 1-Pon... 6-Sob. 127 = wszystkie dni.
volatile bool g_isAlarming = false;       // Flaga, czy budzik aktualnie gra
int g_hNightStart = 22, g_hNightEnd = 6;  // Tryb nocny w godzinach: domyślnie 22:00 - 6:00

// =============================================================================
// LOGIKA NOCNA (Wspólna dla obu architektur)
// =============================================================================
bool isItNightRightNow() {
  if (!g_timeValid) return false;
  if (g_hNightStart == g_hNightEnd) return false;  // Funkcja wyłączona
  if (g_hNightStart > g_hNightEnd) {
    return (g_hour >= g_hNightStart || g_hour < g_hNightEnd);
  } else {
    return (g_hour >= g_hNightStart && g_hour < g_hNightEnd);
  }
}

// =============================================================================
// FORMATOWANIE ZNAKÓW I CZCIONKI (Wspólne helpery)
// =============================================================================
static inline uint8_t segForDigit(int d) {
  if (d < 0 || d > 9) return FONT_BLANK;
  return FONT_HEX[d];
}

uint8_t segFromChar(char c) {
  if (c >= '0' && c <= '9') return FONT_HEX[c - '0'];
  if (c >= 'A' && c <= 'F') return FONT_HEX[c - 'A' + 10];
  if (c >= 'a' && c <= 'f') return FONT_HEX[c - 'a' + 10];
  if (c == '-') return FONT_MINUS;
  if (c == 'C') return FONT_C;
  if (c == ' ') return FONT_BLANK;
  return FONT_BLANK;
}

int getDS18B20Resolution() {
  DeviceAddress addr;
  if (!sensors.getAddress(addr, 0)) return -1;
  return sensors.getResolution(addr);
}

// =============================================================================
//                --- ARCHITEKTURA NEOPIXEL (SK6812) ---
// =============================================================================
#ifdef DISPLAY_TYPE_NEOPIXEL

void clearNeoDisplay() {
  strip.clear();
  strip.show();
}

void setDigitNeo(uint8_t digitPos, uint8_t fontPattern, uint32_t color) {
  int baseLed = digitPos * 21;      // 7 segmentów x 3 diody = 21 na cyfrę
  if (digitPos >= 2) baseLed += 2;  // Mijamy pierwszy dwukropek
  if (digitPos >= 4) baseLed += 2;  // Mijamy drugi dwukropek

  for (uint8_t segIdx = 0; segIdx < 7; segIdx++) {
    bool isSegmentOn = (fontPattern & (1 << segIdx)) != 0;
    uint32_t segColor = isSegmentOn ? color : 0;

    int startLedIndex = baseLed + (segIdx * 3);
    for (int i = 0; i < 3; i++) {
      strip.setPixelColor(startLedIndex + i, segColor);
    }
  }
}

void setColonNeo(uint8_t colIdx, bool state, uint32_t color) {
  int startLed = (colIdx == 0) ? 42 : 86;  // indeksy dwukropków - 0: 42, 1: 86
  uint32_t colColor = state ? color : 0;
  strip.setPixelColor(startLed, colColor);
  strip.setPixelColor(startLed + 1, colColor);
}

void setAmbientNeo(uint32_t color) {
  for (int i = 130; i < 144; i++) {
    strip.setPixelColor(i, color);
  }
}

void showBootId6() {
  g_showBootId = true;
  strip.clear();
  uint32_t idColor = strip.Color(0, 200, 255);  // Cyan dla Boot ID

  // Wyświetlamy 4 znaki ID na czterech ostatnich pozycjach paska
  setDigitNeo(2, segFromChar(id[0]), idColor);
  setDigitNeo(3, segFromChar(id[1]), idColor);
  setDigitNeo(4, segFromChar(id[2]), idColor);
  setDigitNeo(5, segFromChar(id[3]), idColor);
  strip.show();

  delay(5000);
  g_showBootId = false;
}
#endif
// =============================================================================
//                --- ARCHITEKTURA KLASYCZNA LED 7-SEG ---
// =============================================================================
#ifndef DISPLAY_TYPE_NEOPIXEL

static inline void shiftOutByte(uint8_t val) {
  for (int i = 7; i >= 0; --i) {
    digitalWrite(PIN_595_CLK, LOW);
    digitalWrite(PIN_595_DATA, (val >> i) & 0x01);
    digitalWrite(PIN_595_CLK, HIGH);
  }
}

static inline void write595(uint8_t segments) {
  digitalWrite(PIN_595_OE, HIGH);  // Wyłącz świecenie na czas przesyłu danych
  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, segments);
  digitalWrite(PIN_595_LATCH, HIGH);
}
// -----------------------------------------------------------------------------
// Display low-level
// -----------------------------------------------------------------------------
static const int DIGIT_PINS[4] = { PIN_DIGIT_0, PIN_DIGIT_1, PIN_DIGIT_2, PIN_DIGIT_3 };

static inline void allDigitsOff() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(DIGIT_PINS[i], DIGIT_ENABLE_HIGH ? LOW : HIGH);
  }
}

static inline void digitOn(uint8_t idx) {
  digitalWrite(DIGIT_PINS[idx], DIGIT_ENABLE_HIGH ? HIGH : LOW);
}
// -----------------------------------------------------------------------------
// Display timer ISR (ultra-stable, no drift)
// -----------------------------------------------------------------------------
hw_timer_t *displayTimer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint8_t currentDigit = 0;
static const uint16_t FRAME_US = 4000;

void IRAM_ATTR onDisplayTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  allDigitsOff();

  if (g_otaActive) {
    write595(SEG_MASK(FONT_HEX[10]));  // Litera 'A'
    digitOn(0);
  } else {
    write595(SEG_MASK(g_displaySeg[currentDigit]));
    digitOn(currentDigit);
    currentDigit = (currentDigit + 1) & 0x03;  // Przełącz na następną cyfrę
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void commitDisplayBuffer() {
  portENTER_CRITICAL(&timerMux);
  g_displaySeg[0] = g_displayNext[0];
  g_displaySeg[1] = g_displayNext[1];
  g_displaySeg[2] = g_displayNext[2];
  g_displaySeg[3] = g_displayNext[3];
  portEXIT_CRITICAL(&timerMux);
}

void showBootId4() {
  g_showBootId = true;
  // Wpisujemy ID bezpośrednio do bufora, który czyta przerwanie (ISR)
  portENTER_CRITICAL(&timerMux);
  g_displaySeg[0] = segFromChar(id[0]);
  g_displaySeg[1] = segFromChar(id[1]);
  g_displaySeg[2] = segFromChar(id[2]);
  g_displaySeg[3] = segFromChar(id[3]);
  portEXIT_CRITICAL(&timerMux);

  delay(5000);
  g_showBootId = false;
}

void setDisplayTime(int hh, int mm, bool colonOn) {
  uint8_t s0 = (hh >= 10) ? segForDigit(hh / 10) : FONT_BLANK;
  uint8_t s1 = segForDigit(hh % 10);
  uint8_t s2 = segForDigit(mm / 10);
  uint8_t s3 = segForDigit(mm % 10);

  if (colonOn) s1 |= SEG_DP;
  else s1 &= ~SEG_DP;

  g_displayNext[0] = s0;
  g_displayNext[1] = s1;
  g_displayNext[2] = s2;
  g_displayNext[3] = s3;
  commitDisplayBuffer();
}

void setDisplayTemp(float tC) {
  if (isnan(tC) || tC < 0.0f || tC > 99.0f) {
    g_displayNext[0] = FONT_BLANK;
    g_displayNext[1] = FONT_BLANK;
    g_displayNext[2] = FONT_BLANK;
    g_displayNext[3] = FONT_BLANK;
    commitDisplayBuffer();
    return;
  }
  int temp = (int)roundf(tC);
  g_displayNext[0] = (temp >= 10) ? segForDigit(temp / 10) : FONT_BLANK;
  g_displayNext[1] = segForDigit(temp % 10);
  g_displayNext[2] = FONT_DEGREE;
  g_displayNext[3] = FONT_C;
  commitDisplayBuffer();
}
#endif
// =============================================================================
//               --- STEROWANIE SYGNALIZACJĄ AUDIO (BUZZER) ---
// =============================================================================
void initBuzzer() {
#if HAS_BUZZER
  // Wersja dla pakietu ESP32 Core 2.x
  // Kanał 2, Częstotliwość 2000Hz, Rozdzielczość 8 bitów
  ledcSetup(BUZZER_CH, 2000, 8);
  ledcAttachPin(PIN_BUZZER, BUZZER_CH);
  ledcWrite(BUZZER_CH, 0);  // Wyciszenie na starcie
#endif
}

void beep(int freq, int duration, bool isAlarm) {
#if HAS_BUZZER
  if (g_masterMute) return;  // Całkowite wyciszenie z poziomu WebUI

  // AUTOMATYCZNE WYCISZENIE NOCNE (Dla Chime/Gongów godzinnych)
  // Alarmy (isAlarm == true) są nadrzędne i ignorują ciszę nocną!
  if (!isAlarm && isItNightRightNow()) {
    return;  // Trwa noc, rezygnujemy z dźwięku
  }

  // Mapowanie głośności: 0-100% z WebUI na bezpieczny zakres pracy buzzera (0-128)
  // Dla piezo-buzzerów wypełnienie 50% (128 w skali 255) to maksymalna sprawność akustyczna
  int duty = map(g_buzzerVol, 0, 100, 0, 128);

  // Generowanie tonu
  ledcWriteTone(BUZZER_CH, freq);
  ledcWrite(BUZZER_CH, duty);  // Ustawiamy głośność
  vTaskDelay(pdMS_TO_TICKS(duration));
  // Twarde odcięcie sygnału, aby zapobiec irytującemu brzęczeniu rezonansowemu
  ledcWrite(BUZZER_CH, 0);
#endif
}
// =============================================================================
//            --- AUTOMATYKA I KONTROLA JASNOŚCI (LDR / PWM) ---
// =============================================================================
#ifndef DISPLAY_TYPE_NEOPIXEL
static const int PWM_CH = 0;
static const int PWM_FREQ = 20000;  // 20 kHz
static const int PWM_RES = 8;       // 0..255
// Sterownik sprzętowy PWM dla klasycznej linii OE rejestrów 595 (Aktywny LOW)
void applyBrightness(uint8_t logical) {
  // Odwrócenie logiki dla pinu OE (logical 255 -> duty 0 -> pełne świecenie)
  uint8_t oeDuty = 255 - logical;
  ledcWrite(PWM_CH, oeDuty);
}

void initBrightnessHardware() {
  pinMode(PIN_595_OE, OUTPUT);
  digitalWrite(PIN_595_OE, HIGH);  // Wyciszenie świecenia na starcie

  // Wersja dla pakietu ESP32 Core 2.x
  // Kanał PWM_CH (0), Częstotliwość 20kHz, Rozdzielczość 8 bitów
  ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_595_OE, PWM_CH);
  ledcWrite(PWM_CH, 255);  // Pełne wygaszenie na starcie (255 = off)
  applyBrightness(g_brightness);
}
#endif
// =============================================================================
//      --- INICJALIZACJA SPRZĘTOWA DLA ARCHITEKTURY LED (7-SEG) ---
// =============================================================================
#ifndef DISPLAY_TYPE_NEOPIXEL

void initDisplayHardware() {
  pinMode(PIN_595_DATA, OUTPUT);
  pinMode(PIN_595_CLK, OUTPUT);
  pinMode(PIN_595_LATCH, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(DIGIT_PINS[i], OUTPUT);
  }

  allDigitsOff();
  write595(0);
}

#endif

uint8_t computeAutoBrightnessFromLDR() {
  // LDR is at the bottom (to GND), 10k at the top (to +3.3V)
  // → dark = high ADC value, bright = low ADC value
  // Odczyt z fotorezystora: ciemno = wysoka wartość ADC (3900), jasno = niska (900)
  int raw = analogRead(PIN_LDR_ADC);

  // Filtracja EMA (Exponential Moving Average) - eliminuje nagłe skoki (np. cień ręki)
  static float ldrEma = 2000.0f;
  ldrEma = 0.9f * ldrEma + 0.1f * (float)raw;

  // ZABEZPIECZENIE: Ochrona przed krytycznym dzieleniem przez zero
  if (abs(g_rawDark - g_rawBright) < 10) return 128;

  // Normalizacja odczytu do zakresu 0.0 - 1.0
  float normalized = (float)(g_rawDark - ldrEma) / (float)(g_rawDark - g_rawBright);
  if (normalized < 0.0f) normalized = 0.0f;
  if (normalized > 1.0f) normalized = 1.0f;

  // Ustalenie progu minimalnego i maksymalnego jasności roboczej
  int currentBMin = 5;
  int currentBMax = 250;

  // Jeśli wygaszanie nocne jest aktywne w WebUI i trwa noc - pozwalamy obliczeniom zejść do 0
  if (g_nightLedOff && isItNightRightNow() && !g_isAlarming) {
    currentBMin = 0;
  }

  // Finalne wyliczenie wartości jasności (0-255)
  int outBrightness = (int)(currentBMin + normalized * (currentBMax - currentBMin));

  // uncomment for measurements
  //Serial.printf("LDR raw=%d  ema=%.1f  norm=%.2f  brightness=%d\n", raw, ldrEma, normalized, outBrightness);

  return (uint8_t)constrain(outBrightness, 0, 255);
}
// =============================================================================
//               --- ZADANIA SYSTEMOWE FREERTOS (CORE LOGIC) ---
// =============================================================================

void BrightnessTask(void *pv) {
  // Wartości startowe wymuszające pierwsze rysowanie ekranu po starcie
  uint8_t lastAppliedBrightness = 255;
  uint8_t lastDisplayedSecond = 99;

#ifdef DISPLAY_TYPE_NEOPIXEL
  static bool neoHardwareInitialized = false;
  static uint8_t fadeStepsLeft = 0;  // Licznik obrotów dla efektu Fade
#endif

  for (;;) {
    uint8_t target = g_brightness;

    // 1. Odczyt automatyki jasności z czujnika LDR
    if (g_autoBrightness) {
      target = computeAutoBrightnessFromLDR();
      g_brightness = target;
    }

    // 2. Bezpiecznik trybu nocnego (Stealth) - wygaszenie tylko gdy jest ciemno (target == 0)
    if (g_nightLedOff && g_timeValid && isItNightRightNow() && !g_isAlarming && (target == 0)) {
      target = 0;
    }

    // 3. SEKCJA WYŻWALAJĄCA WYŚWIETLACZE (Rozdział architektur)
#ifdef DISPLAY_TYPE_NEOPIXEL
    // --- ARCHITEKTURA NEOPIXEL (SK6812) ---
    if (target > 0 && g_timeValid) {

      // Inicjalizacja opóźniona (Late Binding) - wykonywana TYLKO RAZ po złapaniu WiFi
      if (!neoHardwareInitialized) {
        pinMode(PIN_NEO_OE, OUTPUT);
        digitalWrite(PIN_NEO_OE, HIGH);  // Blokada sprzętowa bramki

        strip.begin();  // Rezerwacja kanału RMT bezpieczna po handshake WiFi
        strip.clear();
        strip.setBrightness(target);

        digitalWrite(PIN_NEO_OE, LOW);  // Otwarcie linii danych
        strip.show();
        neoHardwareInitialized = true;
        Serial.println("🌐 SYSTEM: Karta sieciowa stabilna. Sprzęt NeoPixel został bezpiecznie zainicjowany!");
        vTaskDelay(pdMS_TO_TICKS(50));
        showBootId6();
      }
      // Warunek inteligentnego odświeżania - strzał do paska TYLKO przy realnej zmianie
      // if (g_second != lastDisplayedSecond || target != lastAppliedBrightness) {
      //   strip.setBrightness(target);

      //   refreshNeoDisplay();  // Generowanie matrycy w RAM
      //   strip.show();         // Jedyny, bezpieczny i kontrolowany impuls danych na sekundę

      //   lastDisplayedSecond = g_second;
      //   lastAppliedBrightness = target;
      // }

      // TWARDE, BEZWARUNKOWE ODŚWIEŻANIE CO 200MS dla płynności Fade i Tęczy
      // strip.setBrightness(target);
      // refreshNeoDisplay();  // Wylicza barwy w RAM
      // strip.show();         // Wysyła dane 5 razy na sekundę. Pełna płynność!

      // lastDisplayedSecond = g_second;
      // lastAppliedBrightness = target;

      // Wykrywamy nową sekundę i uzbrajamy licznik kroków Fade
      if (g_second != lastDisplayedSecond) {
        // Wykrywamy nową sekundę i uzbrajamy licznik na 7 kroków (7 * 50ms = 350ms idealnego płynnego przejścia)
        fadeStepsLeft = 7;
      }

      // WARUNEK INTELIGENTNEGO ODŚWIEŻANIA v2.2:
      // Wysyłamy dane TYLKO gdy:
      // 1. Zmieniła się sekunda lub jasność LDR
      // 2. Trwa aktywna faza wygaszania/zapalania cyfr (fadeStepsLeft > 0)
      // 3. Włączona jest pełna tęcza przestrzenna (g_ledEffectMode == 2), która wymaga stałego ruchu
      bool needShow = (g_second != lastDisplayedSecond) || (target != lastAppliedBrightness) || (fadeStepsLeft > 0) || (g_ledEffectMode == 2);

      if (needShow && !g_otaActive) {
        strip.setBrightness(target);
        refreshNeoDisplay();
        strip.show();

        if (fadeStepsLeft > 0) fadeStepsLeft--;  // Zmniejszamy licznik kroków

        lastDisplayedSecond = g_second;
        lastAppliedBrightness = target;
      }

    } else {
      // Tryb nocny wygaszenia lub faza bootowania - trzymamy mrok na sklejce
      if (neoHardwareInitialized && lastAppliedBrightness != 0 && !g_otaActive) {
        strip.clear();
        strip.show();
        lastAppliedBrightness = 0;
      }
    }

#else
    // --- ARCHITEKTURA KLASYCZNA LED 7-SEG ---
    if (abs((int)target - (int)lastAppliedBrightness) >= 2 || (target == 0 && lastAppliedBrightness != 0) || (target != 0 && lastAppliedBrightness == 0)) {
      applyBrightness(target);
      lastAppliedBrightness = target;
    }
#endif

    // Próbkowanie automatyki co 200ms zapewnia idealną, płynną reakcję na suwaki
    // Zmiana taktowania na 50ms (Dla idealnej, kinowej płynności fali Fade!)
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
// -----------------------------------------------------------------------------
// TimeTask
// -----------------------------------------------------------------------------
void TimeTask(void *pv) {
  struct tm ti;
  struct timeval tv;
  long lastLoggedSec = -1;

  for (;;) {
    gettimeofday(&tv, NULL);

    // Wykonuje się dokładnie raz na sekundę, gdy system operacyjny przeskakuje czas
    if (tv.tv_sec != lastLoggedSec) {
      lastLoggedSec = tv.tv_sec;

      if (getLocalTime(&ti, 0)) {
        g_hour = ti.tm_hour;
        g_minute = ti.tm_min;
        g_second = ti.tm_sec;
        g_timeValid = true;

        // Podwójny sygnał dźwiękowy o pełnej godzinie (Casio style)
        if (g_minute == 0 && g_second == 0 && g_hourlyChime) {
          beep(4000, 50, false);
          vTaskDelay(pdMS_TO_TICKS(50));
          beep(4000, 50, false);
        }
      }
    }

    // OBLICZANIE INTERWAŁU HYBRYDOWEGO (Precyzja v1.8)
    int ms_to_next = 1000 - (tv.tv_usec / 1000);

    // Antykryzysowy bezpiecznik resetujący licznik przy potężnych lagach stosu IP
    if (ms_to_next < 0 || ms_to_next > 1000) {
      ms_to_next = 100;
    }

    // Dwubiegowy system snu zadania: oszczędzanie CPU na początku, wysoka czujność pod koniec sekundy
    if (ms_to_next > 150) {
      vTaskDelay(pdMS_TO_TICKS(100));  // Głęboki sen
    } else {
      vTaskDelay(pdMS_TO_TICKS(10));  // Gęste próbkowanie dla idealnego taktu dwukropka
    }
  }
}
// -----------------------------------------------------------------------------
// TempTask
// -----------------------------------------------------------------------------
void TempTask(void *pv) {
  for (;;) {
    // Rozpoczynamy konwersję temperatury w tle (asynchronicznie)
    sensors.requestTemperatures();
    vTaskDelay(pdMS_TO_TICKS(800));  // Bezpieczny czas na ustabilizowanie odczytu układu Dallas

    float t = NAN;
    int retryCount = 0;
    const int maxRetries = 5;

    // Pętla pancernego ponawiania odczytu w przypadku zakłóceń linii 1-Wire
    while (retryCount < maxRetries) {
      t = sensors.getTempCByIndex(0);
      if (t > -80.0f && t < 150.0f) {
        break;  // Odczyt poprawny, przerywamy pętlę awaryjną
      }
      retryCount++;
      vTaskDelay(pdMS_TO_TICKS(150));
    }

    if (!isnan(t) && t > -80.0f) {
      g_tempC = t + g_tempOffset;  // Aplikujemy sensoryczny offset z WebUI
      g_tempValid = true;
      // --- CZUJNIK SPRAWNY -> KASUJEMY FLAGĘ BŁĘDU ---
      if (g_tempErrorBeepDone) {
        g_tempErrorBeepDone = false;
        Serial.println("🔄 SYSTEM: Czujnik DS18B20 odzyskał sprawność. Sygnalizacja audio ponownie uzbrojona.");
      }
    } else {
      Serial.printf("❌ BŁĄD SYSTEMU: Czujnik DS18B20 nie odpowiedział po %d próbach!\n", maxRetries);
      g_tempValid = false;
      g_tempC = NAN;
      // --- INTELIGENTNY BEZPIECZNIK AKUSTYCZNY ---
      if (!g_tempErrorBeepDone) {
        beep(500, 300, false);       // Niski ton ostrzegawczy o awarii sprzętu – TYLKO JEDEN RAZ!
        g_tempErrorBeepDone = true;  // Zatrzaskujemy flagę, blokując kolejne sygnały dźwiękowe
        Serial.println("🔔 OSTRZEŻENIE: Wyemitowano jednorazowy alarm awarii sensora.");
      }
    }

    // Pomiar temperatury wykonuje się co 15 sekund (w pełni odizolowany na rdzeniu 0)
    vTaskDelay(pdMS_TO_TICKS(15000));
  }
}
// -----------------------------------------------------------------------------
// AlarmTask
// -----------------------------------------------------------------------------
void AlarmTask(void *pv) {
  for (;;) {
    struct tm ti;
    if (g_alarmActive && getLocalTime(&ti, 0)) {
      // Weryfikacja harmonogramu tygodniowego za pomocą maski bitowej dni (g_alarmDays)
      bool dayMatch = (g_alarmDays & (1 << ti.tm_wday));

      if (dayMatch && g_hour == g_alarmH && g_minute == g_alarmM && g_second == 0) {
        g_isAlarming = true;
        Serial.printf("⏰ ALARM! Melodia: %d\n", g_alarmMelody);

        // Cykl odtwarzania sygnału przez maksymalnie 60 sekund lub do wyłączenia (Mute)
        for (int i = 0; i < 60 && g_isAlarming; i++) {
          if (g_masterMute) {
            g_isAlarming = false;
            break;
          }

          switch (g_alarmMelody) {
            case 1:  // Melodia radosna (narastająca)
              beep(1500, 100, true);
              beep(2000, 100, true);
              beep(2500, 100, true);
              vTaskDelay(pdMS_TO_TICKS(700));
              break;
            case 2:  // Syrena (naprzemienna)
              beep(3000, 400, true);
              beep(1500, 400, true);
              vTaskDelay(pdMS_TO_TICKS(200));
              break;
            default:  // Melodia klasyczna (pi-pi-pi)
              for (int j = 0; j < 3; j++) {
                beep(2500, 100, true);
                vTaskDelay(pdMS_TO_TICKS(100));
              }
              vTaskDelay(pdMS_TO_TICKS(700));
              break;
          }
        }
        g_isAlarming = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));  // Sprawdzanie warunków alarmu co 1 sekundę
  }
}
// =============================================================================
//      --- LOGIKA GŁÓWNA DLA ARCHITEKTURY LED (Wersja 4-Cyfrowa) ---
// =============================================================================
#ifndef DISPLAY_TYPE_NEOPIXEL

void LogicTask(void *pv) {
  Serial.println("LogicTask: Uruchomiony (Tryb: LED 7-Seg)");
  static int lastSec = -1;
  static bool colon = false;

  for (;;) {
    // BEZPIECZNIK: Jeśli mamy czas, a flaga showID nadal wisi - wymuś start zegara
    if (g_timeValid && g_showBootId) {
      g_showBootId = false;
    }
    // Bezpieczne kopiowanie czasu w sekcji krytycznej
    int h, m, s;
    portENTER_CRITICAL(&timerMux);
    h = g_hour;
    m = g_minute;
    s = g_second;
    portEXIT_CRITICAL(&timerMux);

    if (g_showBootId) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    uint8_t localBuf[4] = { FONT_BLANK, FONT_BLANK, FONT_BLANK, FONT_BLANK };

    if (!g_timeValid) {
      for (int i = 0; i < 4; i++) localBuf[i] = FONT_MINUS;
    } else {
      uint32_t now = millis();
      // Cykl: 15 sekund czas, 5 sekund temperatura
      bool showTempNow = (now % 20000 < 5000);
      // NOWA MATEMATYKA DLA LED:
      // prawda dla 10-14, 30-34 i 50-54 sekundy
      //bool showTempNow = ((s % 20) >= 10 && (s % 20) < 15);

      if (g_second != lastSec) {
        lastSec = g_second;
        colon = (g_second % 2) == 0;
      }

      if (showTempNow && g_tempValid && !isnan(g_tempC)) {
        int temp = (int)roundf(g_tempC);
        // Dodatkowe zabezpieczenie zakresu dla 4 cyfr
        if (temp > -9 && temp < 100) {
          localBuf[0] = (temp >= 10) ? segForDigit(temp / 10) : FONT_BLANK;
          localBuf[1] = segForDigit(temp % 10);
          localBuf[2] = FONT_DEGREE;
          localBuf[3] = FONT_C;
        } else {
          // Jeśli temp poza zakresem, pokaż kreski
          for (int i = 0; i < 4; i++) localBuf[i] = FONT_MINUS;
        }
      } else {
        localBuf[0] = (h >= 10) ? segForDigit(h / 10) : FONT_BLANK;
        localBuf[1] = segForDigit(h % 10);
        if (colon) localBuf[1] |= SEG_DP;
        localBuf[2] = segForDigit(m / 10);
        localBuf[3] = segForDigit(m % 10);
      }
      if (g_forceWifiDot) localBuf[3] |= SEG_DP;
    }
    // KOPIOWANIE
    portENTER_CRITICAL(&timerMux);
    g_displaySeg[0] = localBuf[0];
    g_displaySeg[1] = localBuf[1];
    g_displaySeg[2] = localBuf[2];
    g_displaySeg[3] = localBuf[3];
    portEXIT_CRITICAL(&timerMux);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
#endif

// =============================================================================
//      --- ROZBUDOWANY SILNIK NEOPIXEL (Obsługa Sekwencji Temp/Czas) ---
// =============================================================================
// #ifdef DISPLAY_TYPE_NEOPIXEL
// void refreshNeoDisplay() {
//   strip.clear();
//   uint32_t activeColor = strip.Color(255, 100, 0);  // kolor główny
//   uint32_t activeColorTemp = strip.Color(255, 50, 0);

//   // uint32_t now = millis();
//   // bool showTempNow = (now % 20000 < 5000);  // Cykl: 15s czas, 5s temperatura
//   // NOWA MATEMATYKA SEKUNDOWA:
//   // (g_second % 20) daje powtarzalny wynik od 0 do 19.
//   // Warunek (< 5) sprawiłby, że temp byłaby w sekundach 00-04, 20-24, 40-44.
//   // Przesunięcie (>= 10 && < 15) uderza idealnie w środki przedziałów: 10-14, 30-34, 50-54!
//   bool showTempNow = ((g_second % 20) >= 10 && (g_second % 20) < 15);

//   if (showTempNow && g_tempValid && !isnan(g_tempC)) {
//     // =========================================================================
//     //   TRYB TEMPERATURY PREMIUM: [ ][ ] [2][8] [°][C]
//     // =========================================================================
//     int temp = (int)roundf(g_tempC);

//     if (temp > -9 && temp < 100) {
//       uint8_t t0 = (temp >= 10) ? segForDigit(temp / 10) : FONT_BLANK;
//       uint8_t t1 = segForDigit(temp % 10);

//       // Pozycje 0 i 1: Całkowicie wygaszone dla skupienia uwagi na wyniku
//       setDigitNeo(0, FONT_BLANK, activeColor);
//       setDigitNeo(1, FONT_BLANK, activeColor);

//       // Pozycje 2-5: Pełna temperatura (np. 2, 8, °, C)
//       setDigitNeo(2, t0, activeColorTemp);
//       setDigitNeo(3, t1, activeColorTemp);
//       setDigitNeo(4, FONT_DEGREE, activeColorTemp);  // Twój oryginalny symbol stopnia
//       setDigitNeo(5, FONT_C, activeColorTemp);       // Twoja oryginalna litera C

//       // W trybie temperatury gasimy oba dwukropki, aby nie udawały kropek dziesiętnych
//       setColonNeo(0, false, activeColor);
//       setColonNeo(1, false, activeColor);
//     }
//   } else {
//     // =========================================================================
//     //   TRYB STANDARDOWY: [H][H] : [M][M] : [S][S]
//     // =========================================================================
//     uint8_t h0 = g_hour / 10;
//     uint8_t h1 = g_hour % 10;
//     uint8_t m0 = g_minute / 10;
//     uint8_t m1 = g_minute % 10;
//     uint8_t s0 = g_second / 10;
//     uint8_t s1 = g_second % 10;

//     // Wygaszanie nieznaczącego zera w godzinach 00-09
//     uint8_t patternH0 = (h0 == 0) ? FONT_BLANK : FONT_HEX[h0];

//     // Rysowanie pełnego czasu na 6 pozycjach
//     setDigitNeo(0, patternH0, activeColor);
//     setDigitNeo(1, FONT_HEX[h1], activeColor);
//     setDigitNeo(2, FONT_HEX[m0], activeColor);
//     setDigitNeo(3, FONT_HEX[m1], activeColor);
//     setDigitNeo(4, FONT_HEX[s0], activeColor);
//     setDigitNeo(5, FONT_HEX[s1], activeColor);

//     // Miganie dwukropków zgrane co sekundę
//     bool colonState = (g_second % 2 == 0);
//     setColonNeo(0, colonState, activeColor);
//     setColonNeo(1, colonState, activeColor);
//   }

//   // Zawsze włączony niebieski Ambient z tyłu sklejki
//   setAmbientNeo(strip.Color(0, 0, 50));
// }
// #endif
// #ifdef DISPLAY_TYPE_NEOPIXEL
// void refreshNeoDisplay() {
//   strip.clear();

//   // 1. Przygotowujemy systemowe kolory 32-bitowe z naszych niezależnych zmiennych
//   uint32_t colorHM = strip.Color(g_colTimeR, g_colTimeG, g_colTimeB);
//   uint32_t colorSeconds = strip.Color(g_colSecR, g_colSecG, g_colSecB);
//   uint32_t colorColon = strip.Color(g_colColonR, g_colColonG, g_colColonB);
//   uint32_t colorTemp = strip.Color(g_colTempR, g_colTempG, g_colTempB);
//   uint32_t colorAmbient = strip.Color(g_colAmbientR, g_colAmbientG, g_colAmbientB);

//   // 2. Obsługa efektu TĘCZY dla sekundnika (Twój luźny pomysł - pierwszy krok!)
//   // Wykorzystujemy funkcję koła kolorów (Hue), gdzie 0-65535 to pełna paleta barw.
//   // Mnożąc g_second * 1092 rozkładamy 60 sekund idealnie na całe koło tęczy (65535 / 60 = 1092)
//   if (g_ledEffectMode == 1) {
//     colorSeconds = strip.gamma32(strip.ColorHSV(g_second * 1092, 255, 255));
//   }

//   // Cykl: 15s czas, 5s temperatura
//   bool showTempNow = ((g_second % 20) >= 10 && (g_second % 20) < 15);

//   if (showTempNow && g_tempValid && !isnan(g_tempC)) {
//     // =========================================================================
//     //   TRYB TEMPERATURY PREMIUM: Godziny wygaszone, termometr lśni własną barwą!
//     // =========================================================================
//     int temp = (int)roundf(g_tempC);

//     if (temp > -9 && temp < 100) {
//       uint8_t t0 = (temp >= 10) ? segForDigit(temp / 10) : FONT_BLANK;
//       uint8_t t1 = segForDigit(temp % 10);

//       setDigitNeo(0, FONT_BLANK, colorHM);
//       setDigitNeo(1, FONT_BLANK, colorHM);

//       // Temperatura i jej jednostki świecą dedykowanym kolorem (np. neonowy błękit)
//       setDigitNeo(2, t0, colorTemp);
//       setDigitNeo(3, t1, colorTemp);
//       setDigitNeo(4, FONT_DEGREE, colorTemp);
//       setDigitNeo(5, FONT_C, colorTemp);

//       // W trybie temperatury gasimy dwukropki
//       setColonNeo(0, false, colorColon);
//       setColonNeo(1, false, colorColon);
//     }
//   } else {
//     // =========================================================================
//     //   TRYB STANDARDOWY MULTI-COLOR: Rozdzielone barwy czasu, sekund i kropek
//     // =========================================================================
//     uint8_t h0 = g_hour / 10;
//     uint8_t h1 = g_hour % 10;
//     uint8_t m0 = g_minute / 10;
//     uint8_t m1 = g_minute % 10;
//     uint8_t s0 = g_second / 10;
//     uint8_t s1 = g_second % 10;

//     uint8_t patternH0 = (h0 == 0) ? FONT_BLANK : FONT_HEX[h0];

//     // Godziny i Minuty dostają kolor nr 1 (colorHM)
//     setDigitNeo(0, patternH0, colorHM);
//     setDigitNeo(1, FONT_HEX[h1], colorHM);
//     setDigitNeo(2, FONT_HEX[m0], colorHM);
//     setDigitNeo(3, FONT_HEX[m1], colorHM);

//     // Sekundy dostają swój własny, osobny kolor (colorSeconds - stały lub tęczowy!)
//     setDigitNeo(4, FONT_HEX[s0], colorSeconds);
//     setDigitNeo(5, FONT_HEX[s1], colorSeconds);

//     // Dwukropki migają własną, zdefiniowaną barwą (colorColon)
//     bool colonState = (g_second % 2 == 0);
//     setColonNeo(0, colonState, colorColon);
//     setColonNeo(1, colonState, colorColon);
//   }

//   // Łuna z tyłu sklejki lśni osobnym kolorem Ambient (colorAmbient)
//   setAmbientNeo(colorAmbient);
// }
// #endif
#ifdef DISPLAY_TYPE_NEOPIXEL
// Tablice pamiętające AKTUALNĄ jasność (0-255) dla każdego z 14 segmentów na sekundniku (2 cyfry x 7 segmentów)
static uint8_t g_segBright[14] = { 0 };
// Funkcja realizująca płynne przejście (Fade) dla poszczególnych diod
void setDigitNeoFade(uint8_t digitPos, uint8_t fontPattern, uint32_t targetColor) {
  // Przeliczamy pozycję na indeks bazowy w tablicy segmentów (pozycja 4: 0-6, pozycja 5: 7-13)
  uint8_t baseSegIdx = (digitPos == 4) ? 0 : 7;

  int baseLed = digitPos * 21;
  if (digitPos >= 2) baseLed += 2;
  if (digitPos >= 4) baseLed += 2;

  // Rozbijamy docelowy kolor na czyste składowe
  uint8_t reqR = (targetColor >> 16) & 0xFF;
  uint8_t reqG = (targetColor >> 8) & 0xFF;
  uint8_t reqB = targetColor & 0xFF;

  for (uint8_t segIdx = 0; segIdx < 7; segIdx++) {
    uint8_t currentSeg = baseSegIdx + segIdx;

    // Sprawdzamy czy dany segment powinien świecić (255) czy być zgaszony (0)
    uint8_t targetBright = (fontPattern & (1 << segIdx)) ? 255 : 0;

    // PŁYNNE DOSKOKI JASNOŚCI: Zmieniamy jasność o 45 jednostek
    // Zapewnia to idealne przejście w około 300 milisekund!
    int step = 45;
    if (g_segBright[currentSeg] < targetBright) {
      g_segBright[currentSeg] = min(g_segBright[currentSeg] + step, (int)targetBright);
    } else if (g_segBright[currentSeg] > targetBright) {
      g_segBright[currentSeg] = max(g_segBright[currentSeg] - step, (int)targetBright);
    }

    // Skalujemy kolor bazowy przez aktualną, wyliczoną płynnie jasność segmentu!
    // Dzięki temu kolory tęczy w trybie 1 nie ulegają zniekształceniu!
    uint8_t finalR = (reqR * g_segBright[currentSeg]) / 255;
    uint8_t finalG = (reqG * g_segBright[currentSeg]) / 255;
    uint8_t finalB = (reqB * g_segBright[currentSeg]) / 255;

    // Wpisujemy gotowy, czysty kolor do paska
    int startLedIndex = baseLed + (segIdx * 3);
    for (int i = 0; i < 3; i++) {
      strip.setPixelColor(startLedIndex + i, strip.Color(finalR, finalG, finalB));
    }
  }
}

// Pomocnicza funkcja dla trybu PEŁNEJ TĘCZY PRZESTRZENNEJ
// Koloruje każdą diodę w segmencie z osobnym przesunięciem na kole barw
void setDigitNeoRainbow(uint8_t digitPos, uint8_t fontPattern, uint32_t timeOffset) {
  int baseLed = digitPos * 21;
  if (digitPos >= 2) baseLed += 2;
  if (digitPos >= 4) baseLed += 2;

  for (uint8_t segIdx = 0; segIdx < 7; segIdx++) {
    bool isSegmentOn = (fontPattern & (1 << segIdx)) != 0;

    int startLedIndex = baseLed + (segIdx * 3);
    for (int i = 0; i < 3; i++) {
      int currentLed = startLedIndex + i;

      if (isSegmentOn) {
        // Każdy kolejny piksel w pasku (currentLed) dostaje przesunięcie o 400 jednostek,
        // a timeOffset (oparty na millis) przesuwa tęczę w czasie, generując ruch fali!
        uint16_t hue = timeOffset + (currentLed * 400);
        strip.setPixelColor(currentLed, strip.gamma32(strip.ColorHSV(hue, 255, 255)));
      } else {
        strip.setPixelColor(currentLed, 0);
      }
    }
  }
}

void refreshNeoDisplay() {
  strip.clear();

  // 1. DYNAMICZNY KALKULATOR KOLORU TEMPERATURY (Termo-indykacja)
  uint32_t colorTemp = strip.Color(g_colTempR, g_colTempG, g_colTempB);  // Domyślny z WebUI
  if (g_tempColorAuto && !isnan(g_tempC) && g_tempValid) {
    // Mapujemy temperaturę z zakresu 15°C - 28°C na kąt Hue (44000 = Błękit, 0 = Czerwień)
    // Constrain chroni przed wyjściem poza bezpieczny zakres
    float tConstrained = constrain(g_tempC, 15.0f, 28.0f);

    // Prosta matematyka odwrócona: im wyższa temp, tym mniejszy Hue (bliżej czerwieni)
    uint16_t tempHue = map((int)(tConstrained * 10), 150, 280, 44000, 0);
    colorTemp = strip.gamma32(strip.ColorHSV(tempHue, 255, 255));
  }

  // 2. Przygotowanie pozostałych standardowych kolorów statycznych
  uint32_t colorHM = strip.Color(g_colTimeR, g_colTimeG, g_colTimeB);
  uint32_t colorSeconds = strip.Color(g_colSecR, g_colSecG, g_colSecB);
  uint32_t colorColon = strip.Color(g_colColonR, g_colColonG, g_colColonB);
  uint32_t colorAmbient = strip.Color(g_colAmbientR, g_colAmbientG, g_colAmbientB);

  // Zsynchronizowany czas dla płynnego ruchu tęczy (dzielenie przez 5 reguluje prędkość fali)
  uint32_t rainbowTimeOffset = millis() / 5;

  // Cykl: 15s czas, 5s temperatura (okna: 10-14, 30-34, 50-54)
  bool showTempNow = ((g_second % 20) >= 10 && (g_second % 20) < 15);

  if (showTempNow && g_tempValid && !isnan(g_tempC)) {
    // =========================================================================
    //   TRYB TEMPERATURY SMART: Kolor dostosowuje się sam do pogody w pokoju!
    // =========================================================================
    int temp = (int)roundf(g_tempC);

    if (temp > -9 && temp < 100) {
      uint8_t t0 = (temp >= 10) ? segForDigit(temp / 10) : FONT_BLANK;
      uint8_t t1 = segForDigit(temp % 10);

      setDigitNeo(0, FONT_BLANK, colorHM);
      setDigitNeo(1, FONT_BLANK, colorHM);

      // Cyfry termometru lśnią kolorem wyliczonym z temperatury
      setDigitNeo(2, t0, colorTemp);
      setDigitNeo(3, t1, colorTemp);
      setDigitNeo(4, FONT_DEGREE, colorTemp);
      setDigitNeo(5, FONT_C, colorTemp);

      setColonNeo(0, false, colorColon);
      setColonNeo(1, false, colorColon);
    }
  } else {
    // =========================================================================
    //   TRYB STANDARDOWY / EFEKTÓW: Czas, Sekundy i Dwukropki
    // =========================================================================
    uint8_t h0 = g_hour / 10;
    uint8_t h1 = g_hour % 10;
    uint8_t m0 = g_minute / 10;
    uint8_t m1 = g_minute % 10;
    uint8_t s0 = g_second / 10;
    uint8_t s1 = g_second % 10;

    uint8_t patternH0 = (h0 == 0) ? FONT_BLANK : FONT_HEX[h0];

    // Sprawdzamy ustawiony tryb efektów specjalnych (np. g_ledEffectMode)
    if (g_ledEffectMode == 2) {
      // --- NOWOŚĆ: TRYB TOTALNEJ TĘCZY PRZESTRZENNEJ (Płynący gradient przez cały zegar) ---
      // Każda cyfra rozszczepia światło niezależnie, tworząc spektakularną falę
      setDigitNeoRainbow(0, patternH0, rainbowTimeOffset);
      setDigitNeoRainbow(1, FONT_HEX[h1], rainbowTimeOffset);
      setDigitNeoRainbow(2, FONT_HEX[m0], rainbowTimeOffset);
      setDigitNeoRainbow(3, FONT_HEX[m1], rainbowTimeOffset);
      setDigitNeoRainbow(4, FONT_HEX[s0], rainbowTimeOffset);
      setDigitNeoRainbow(5, FONT_HEX[s1], rainbowTimeOffset);

      // Dwukropki w tym trybie też dostają kolor z fali tęczy dla spójności
      bool colonState = (g_second % 2 == 0);
      setColonNeo(0, colonState, strip.gamma32(strip.ColorHSV(rainbowTimeOffset + (42 * 400), 255, 255)));
      setColonNeo(1, colonState, strip.gamma32(strip.ColorHSV(rainbowTimeOffset + (86 * 400), 255, 255)));

    } else {
      // --- TRYB STANDARDOWY MULTI-COLOR (Stałe kolory lub tęcza tylko na sekundniku) ---
      setDigitNeo(0, patternH0, colorHM);
      setDigitNeo(1, FONT_HEX[h1], colorHM);
      setDigitNeo(2, FONT_HEX[m0], colorHM);
      setDigitNeo(3, FONT_HEX[m1], colorHM);

      // Jeśli g_ledEffectMode == 1, tylko sekundy płyną w czasie
      uint32_t finalSecColor = colorSeconds;
      if (g_ledEffectMode == 1) {
        finalSecColor = strip.gamma32(strip.ColorHSV(g_second * 1092, 255, 255));
      }
      // setDigitNeo(4, FONT_HEX[s0], finalSecColor);
      // setDigitNeo(5, FONT_HEX[s1], finalSecColor);
      // Przekazujemy rysowanie do silnika Fade
      setDigitNeoFade(4, FONT_HEX[s0], finalSecColor);
      setDigitNeoFade(5, FONT_HEX[s1], finalSecColor);

      bool colonState = (g_second % 2 == 0);
      setColonNeo(0, colonState, colorColon);
      setColonNeo(1, colonState, colorColon);
    }
  }

  // Łuna Ambient z tyłu obudowy
  setAmbientNeo(colorAmbient);
}
#endif

// =============================================================================
//               --- DIAGNOSTYKA I MONITORING SYSTEMU WIFI ---
// =============================================================================
void wifiWatchdog() {
  wl_status_t st = WiFi.status();

  if (st == WL_CONNECTED) {
    if (!wifiWasConnected) {
      wifiWasConnected = true;
      g_forceWifiDot = false;
      Serial.println("🌐 SYSTEM: Połączenie WiFi ustanowione pomyślnie.");
      beep(1200, 60, false);
      vTaskDelay(pdMS_TO_TICKS(60));
      beep(1800, 60, false);
    }
  } else {
    if (wifiWasConnected) {
      wifiWasConnected = false;
      g_forceWifiDot = true;
      Serial.println("⚠️ SYSTEM: Utracono sygnał WiFi!");
      beep(1800, 60, false);
      vTaskDelay(pdMS_TO_TICKS(60));
      beep(1200, 60, false);
    }
  }
}

// =============================================================================
//         --- ZARZĄDZANIE PAMIĘCIĄ NIEULOTNĄ NVS (PREFERENCES) ---
// =============================================================================
// W loadSettings():

// W saveSettings():

// W resetSettings():


void saveSettings() {
  prefs.begin("clock", false);
  prefs.putUChar("bright", g_brightness);
  prefs.putBool("autoB", g_autoBrightness);
  prefs.putFloat("tOffset", g_tempOffset);
  prefs.putInt("rDark", g_rawDark);
  prefs.putInt("rBright", g_rawBright);
  prefs.putInt("alH", g_alarmH);
  prefs.putInt("alM", g_alarmM);
  prefs.putBool("alOn", g_alarmActive);
  prefs.putBool("mMute", g_masterMute);
  prefs.putInt("alMel", g_alarmMelody);
  prefs.putUChar("alDays", g_alarmDays);
  prefs.putInt("bzVol", g_buzzerVol);
  prefs.putBool("hChime", g_hourlyChime);
  prefs.putInt("hNStart", g_hNightStart);
  prefs.putInt("hNEnd", g_hNightEnd);
  prefs.putBool("nLedOff", g_nightLedOff);
#ifdef DISPLAY_TYPE_NEOPIXEL
  prefs.putUChar("tcR", g_colTimeR);
  prefs.putUChar("tcG", g_colTimeG);
  prefs.putUChar("tcB", g_colTimeB);
  prefs.putUChar("scR", g_colSecR);
  prefs.putUChar("scG", g_colSecG);
  prefs.putUChar("scB", g_colSecB);
  prefs.putUChar("ccR", g_colColonR);
  prefs.putUChar("ccG", g_colColonG);
  prefs.putUChar("ccB", g_colColonB);
  prefs.putUChar("tmR", g_colTempR);
  prefs.putUChar("tmG", g_colTempG);
  prefs.putUChar("tmB", g_colTempB);
  prefs.putUChar("amR", g_colAmbientR);
  prefs.putUChar("amG", g_colAmbientG);
  prefs.putUChar("amB", g_colAmbientB);
  prefs.putUChar("fxMode", g_ledEffectMode);
  prefs.putBool("tColAuto", g_tempColorAuto);
#endif
  prefs.end();

  // Akustyczne potwierdzenie zapisu ustawień
  beep(1200, 50, false);
  vTaskDelay(pdMS_TO_TICKS(50));
  beep(1800, 50, false);
}

void resetSettings() {
  const uint8_t DEFAULT_BRIGHT = 128;
  const bool DEFAULT_AUTO = true;
  const float DEFAULT_TEMP_OFFSET = 0.0f;
  const int DEFAULT_RAW_DARK = 3900;
  const int DEFAULT_RAW_BRIGHT = 900;
  const int DEFAULT_AL_H = 7;
  const int DEFAULT_AL_M = 0;
  const bool DEFAULT_AL_ON = false;
  const bool DEFAULT_M_MUTE = false;
  const int DEFAULT_AL_MELODY = 0;
  const uint8_t DEFAULT_AL_DAYS = 127;
  const int DEFAULT_BUZ_VOL = 50;
  const bool DEFAULT_H_CHIME = true;
  const int DEFAULT_NIGHT_START = 22;
  const int DEFAULT_NIGHT_END = 6;
  const bool DEFAULT_NIGHT_LED_OFF = false;

  prefs.begin("clock", false);
  prefs.putUChar("bright", DEFAULT_BRIGHT);
  prefs.putBool("autoB", DEFAULT_AUTO);
  prefs.putFloat("tOffset", DEFAULT_TEMP_OFFSET);
  prefs.putInt("rDark", DEFAULT_RAW_DARK);
  prefs.putInt("rBright", DEFAULT_RAW_BRIGHT);
  prefs.putInt("alH", DEFAULT_AL_H);
  prefs.putInt("alM", DEFAULT_AL_M);
  prefs.putBool("alOn", DEFAULT_AL_ON);
  prefs.putBool("mMute", DEFAULT_M_MUTE);
  prefs.putInt("alMel", DEFAULT_AL_MELODY);
  prefs.putUChar("alDays", DEFAULT_AL_DAYS);
  prefs.putInt("bzVol", DEFAULT_BUZ_VOL);
  prefs.putBool("hChime", DEFAULT_H_CHIME);
  prefs.putInt("hNStart", DEFAULT_NIGHT_START);
  prefs.putInt("hNEnd", DEFAULT_NIGHT_END);
  prefs.putBool("nLedOff", DEFAULT_NIGHT_LED_OFF);
#ifdef DISPLAY_TYPE_NEOPIXEL
  prefs.putBool("tColAuto", true);
#endif
  prefs.end();

  g_brightness = DEFAULT_BRIGHT;
  g_autoBrightness = DEFAULT_AUTO;
  g_tempOffset = DEFAULT_TEMP_OFFSET;
  g_rawDark = DEFAULT_RAW_DARK;
  g_rawBright = DEFAULT_RAW_BRIGHT;
  g_alarmH = DEFAULT_AL_H;
  g_alarmM = DEFAULT_AL_M;
  g_alarmActive = DEFAULT_AL_ON;
  g_masterMute = DEFAULT_M_MUTE;
  g_alarmMelody = DEFAULT_AL_MELODY;
  g_alarmDays = DEFAULT_AL_DAYS;
  g_buzzerVol = DEFAULT_BUZ_VOL;
  g_hourlyChime = DEFAULT_H_CHIME;
  g_hNightStart = DEFAULT_NIGHT_START;
  g_hNightEnd = DEFAULT_NIGHT_END;
  g_nightLedOff = DEFAULT_NIGHT_LED_OFF;
#ifdef DISPLAY_TYPE_NEOPIXEL
  g_tempColorAuto = true;
#endif
  beep(1200, 50, false);
  vTaskDelay(pdMS_TO_TICKS(50));
  beep(1800, 50, false);
}

void loadSettings() {
  prefs.begin("clock", false);
  g_brightness = prefs.getUChar("bright", 128);
  g_autoBrightness = prefs.getBool("autoB", true);
  g_tempOffset = prefs.getFloat("tOffset", 0.0f);
  g_rawDark = prefs.getInt("rDark", 3900);
  g_rawBright = prefs.getInt("rBright", 900);
  g_alarmH = prefs.getInt("alH", 7);
  g_alarmM = prefs.getInt("alM", 0);
  g_alarmActive = prefs.getBool("alOn", false);
  g_masterMute = prefs.getBool("mMute", false);
  g_alarmMelody = prefs.getInt("alMel", 0);
  g_alarmDays = prefs.getUChar("alDays", 127);
  g_buzzerVol = prefs.getInt("bzVol", 50);
  g_hourlyChime = prefs.getBool("hChime", true);
  g_hNightStart = prefs.getInt("hNStart", 22);
  g_hNightEnd = prefs.getInt("hNEnd", 6);
  g_nightLedOff = prefs.getBool("nLedOff", false);
#ifdef DISPLAY_TYPE_NEOPIXEL
  g_colTimeR = prefs.getUChar("tcR", 255);
  g_colTimeG = prefs.getUChar("tcG", 0);
  g_colTimeB = prefs.getUChar("tcB", 0);
  g_colSecR = prefs.getUChar("scR", 46);
  g_colSecG = prefs.getUChar("scG", 49);
  g_colSecB = prefs.getUChar("scB", 56);
  g_colColonR = prefs.getUChar("ccR", 40);
  g_colColonG = prefs.getUChar("ccG", 215);
  g_colColonB = prefs.getUChar("ccB", 70);
  g_colTempR = prefs.getUChar("tmR", 0);
  g_colTempG = prefs.getUChar("tmG", 180);
  g_colTempB = prefs.getUChar("tmB", 255);
  g_colAmbientR = prefs.getUChar("amR", 2);
  g_colAmbientG = prefs.getUChar("amG", 2);
  g_colAmbientB = prefs.getUChar("amB", 110);
  g_ledEffectMode = prefs.getUChar("fxMode", 0);
  g_tempColorAuto = prefs.getBool("tColAuto", true);
#endif
  prefs.end();
}
// =============================================================================
//               --- KONFIGURACJA CZASU SYSTEMOWEGO (NTP + DST) ---
// =============================================================================
/* TESTY ZMIANY CZASU
  //configTzTime("CET-1CEST,M3.5.0/2,M4.3.1/9", "tempus1.gum.gov.pl", "pl.pool.ntp.org", "tempus2.gum.gov.pl");
  M3.5.0/2 — początek czasu letniego
  Format: M<miesiąc>.<tydzień>.<dzień tygodnia>/<godzina>
  M3 – marzec
  5 – piąty tydzień miesiąca (czyli ostatnia niedziela marca)
  0 – niedziela
  /2 – o godzinie 2:00 czasu lokalnego
  Start DST: ostatnia niedziela marca o 02:00

  M10.5.0/3 — koniec czasu letniego
  M10 – październik
  5 – piąty tydzień (ostatnia niedziela października)
  0 – niedziela
  /3 – o godzinie 3:00 czasu lokalnego
  Koniec DST: ostatnia niedziela października o 03:00
*/
void setupTime() {
  // Pełna konfiguracja strefy czasowej dla Polski z uwzględnieniem czasu letniego i zimowego
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3",
               "tempus1.gum.gov.pl",
               "tempus2.gum.gov.pl",
               "pl.pool.ntp.org");

  struct tm timeinfo;
  unsigned long start = millis();

  // Aktywna pętla oczekiwania na szybką synchronizację przy rozruchu (max 3 sekundy)
  while (millis() - start < 3000) {
    esp_task_wdt_reset();  // Karmienie psa, aby pętla nie wywołała resetu przy braku sieci
    if (getLocalTime(&timeinfo)) {
      if (timeinfo.tm_year + 1900 > 2020) {
        int lastSec = timeinfo.tm_sec;
        unsigned long secWaitStart = millis();

        // Dodatkowy mikro-doskok dla idealnego zgrania startu sekundy
        while (millis() - secWaitStart < 1500) {
          getLocalTime(&timeinfo);
          if (timeinfo.tm_sec != lastSec) return;
          delay(1);
        }
        return;
      }
    }
    delay(10);
  }
  // If we get here: no NTP sync; LogicTask will show ----
}
// Funkcja wywoływana automatycznie po udanej synchronizacji
void timeSyncCallback(struct timeval *tv) {
  Serial.println("----------------------------------------------");
  Serial.println("🔔 SUKCES: Czas został zsynchronizowany z NTP!");

  struct tm ti;
  getLocalTime(&ti);

  // Formatowanie stempla ostatniej udanej synchronizacji czasu dla WebUI
  snprintf(g_lastSyncTimeStr, sizeof(g_lastSyncTimeStr), "%02d.%02d %02d:%02d:%02d ✔",
           ti.tm_mday, ti.tm_mon + 1, ti.tm_hour, ti.tm_min, ti.tm_sec);

  Serial.print("Aktualny czas: ");
  Serial.println(&ti, "%A, %B %d %Y %H:%M:%S");
  Serial.println("----------------------------------------------");

  beep(3500, 25, false);  // Krótkie dyskretne piknięcie potwierdzające sukces sieciowy
}

// =============================================================================
//             --- GŁÓWNE ZADANIE SIECIOWE (WIFITASK & WEB SERWER) ---
// =============================================================================
void WiFiTask(void *pv) {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  // Generowanie unikalnego ID urządzenia na podstawie adresu MAC
  snprintf(id, sizeof(id), "%02X%02X", mac[4], mac[5]);
  g_hostName = String("esp32-clock-") + id;
  g_deviceId = id;

  // SELEKTOR ARCHITEKTURY: Uruchomienie właściwego ekranu powitalnego ID
#ifdef DISPLAY_TYPE_NEOPIXEL
  //showBootId6(); // Wersja na 6 cyfr NeoPixel
#else
  showBootId4();  // Wersja standardowa na 4 cyfry LED
#endif

  // Konfiguracja menedżera portalu AutoConnect
  portalConfig.autoReconnect = true;
  portalConfig.reconnectInterval = 6;
  portalConfig.retainPortal = true;
  portalConfig.apid = String("ESP32-Clock-") + id;
  portalConfig.psk = "Al@m@kot@";
  portalConfig.hostName = g_hostName.c_str();
  portalConfig.menuItems |= AC_MENUITEM_DELETESSID;
  portalConfig.boundaryOffset = 64;  // Zwiększa stabilność bufora przy dużym HTML
  portalConfig.immediateStart = false;
  portalConfig.homeUri = "/config";  // Wymusza stronę główną portalu
  portalConfig.bootUri = AC_ONBOOTURI_HOME;

  // Konfiguracja diody statusu WiFi (Ticker systemowy)
  // The AutoConnect ticker indicates the WiFi connection status in the following three flicker patterns:
  // Short blink: The ESP module stays in AP_STA mode.
  // Short-on and long-off: No STA connection state. (i.e. WiFi.status != WL_CONNECTED)
  // No blink: WiFi connection with access point established and data link enabled. (i.e. WiFi.status = WL_CONNECTED)
  portalConfig.ticker = true;
  portalConfig.tickerPort = PIN_LED;
  portalConfig.tickerOn = HIGH;
  portal.config(portalConfig);

  // Rejestracja callbacków bezprzewodowej aktualizacji OTA
  ota.onStart([]() {
    beep(1200, 60, false);
    g_otaActive = true;
#ifdef DISPLAY_TYPE_NEOPIXEL
    uint32_t otaColor = strip.Color(g_colTimeR, g_colTimeG, g_colTimeB);
    //clearNeoDisplay();  // Czyszczenie ekranu przed zalaniem danymi OTA
    strip.clear();
    setDigitNeo(0, FONT_HEX[0], otaColor);
    setDigitNeo(1, FONT_T, otaColor);
    setDigitNeo(2, FONT_HEX[10], otaColor);
    strip.show();
#endif
  });

  ota.onEnd([]() {
    g_otaActive = false;
    Serial.println("🌐 OTA: Aktualizacja zakończona. Restart systemu za 5 sekund...");
    beep(1200, 60, false);
    vTaskDelay(pdMS_TO_TICKS(60));
    beep(1800, 60, false);

    // W pełni bezpieczne asynchroniczne odliczenie do restartu
    xTaskCreate([](void *p) {
      vTaskDelay(pdMS_TO_TICKS(5000));
      ESP.restart();
    },
                "reboot", 2048, NULL, 5, NULL);
  });

  // Przekierowanie głównego adresu IP na panel konfiguracyjny zegara
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Location", "/config", true);
    server.send(302, "text/plain", "");
  });

  // ===========================================================================
  //               --- ENDPOINT STATUSU DIAGNOSTYCZNEGO (/status) ---
  // ===========================================================================
  server.on("/status", []() {
    char s[1024];
    int out = 0;
    struct tm ti;
    char dateBuf[32] = "--.--.----";
    const char *days[] = { "Niedziela", "Poniedziałek", "Wtorek", "Środa", "Czwartek", "Piątek", "Sobota" };
    const char *dayName = "---";

    if (getLocalTime(&ti, 0)) {
      snprintf(dateBuf, sizeof(dateBuf), "%02d.%02d.%04d", ti.tm_mday, ti.tm_mon + 1, ti.tm_year + 1900);
      dayName = days[ti.tm_wday];
    }

    // Blok 1: Tożsamość i czas
    out += snprintf(s + out, sizeof(s) - out, "id=%s\nhostname=%s\ntime=%02d:%02d:%02d\ndate=%s\nday=%s\n",
                    g_deviceId.c_str(), g_hostName.c_str(), g_hour, g_minute, g_second, dateBuf, dayName);

    // Blok 2: NTP i Temperatura
    out += snprintf(s + out, sizeof(s) - out, "lastSync=%s\ntempC=%.1f\nd18b20_res=%d\ntOff=%.1f\n",
                    g_lastSyncTimeStr, g_tempC, getDS18B20Resolution(), g_tempOffset);

    // Blok 3: LDR i Jasność / +RGB
    int rawLDR = analogRead(PIN_LDR_ADC);
    out += snprintf(s + out, sizeof(s) - out, "rDark=%d\nrBright=%d\nrawLDR=%d\nbrightness=%d\nautoBrightness=%d\n",
                    g_rawDark, g_rawBright, rawLDR, g_brightness, (g_autoBrightness ? 1 : 0));
#ifdef DISPLAY_TYPE_NEOPIXEL
    out += snprintf(s + out, sizeof(s) - out, "effect=%d\n", g_ledEffectMode);
    out += snprintf(s + out, sizeof(s) - out, "hcolor=#%02x%02x%02x\n", g_colTimeR, g_colTimeG, g_colTimeB);
    out += snprintf(s + out, sizeof(s) - out, "scolor=#%02x%02x%02x\n", g_colSecR, g_colSecG, g_colSecB);
    out += snprintf(s + out, sizeof(s) - out, "ccolor=#%02x%02x%02x\n", g_colColonR, g_colColonG, g_colColonB);
    out += snprintf(s + out, sizeof(s) - out, "tcolor=#%02x%02x%02x\n", g_colTempR, g_colTempG, g_colTempB);
    out += snprintf(s + out, sizeof(s) - out, "acolor=#%02x%02x%02x\n", g_colAmbientR, g_colAmbientG, g_colAmbientB);
    out += snprintf(s + out, sizeof(s) - out, "tAuto=%d\n", g_tempColorAuto ? 1 : 0);
#endif
    // Blok 4: Budzik i tryb nocny
    out += snprintf(s + out, sizeof(s) - out, "hasBuzzer=%d\nbzVol=%d\nisAlarming=%d\nalTime=%02d:%02d\nalActive=%d\n",
                    (HAS_BUZZER ? 1 : 0), g_buzzerVol, (g_isAlarming ? 1 : 0), g_alarmH, g_alarmM, (g_alarmActive ? 1 : 0));
    out += snprintf(s + out, sizeof(s) - out, "alDays=%d\nalMel=%d\nmMute=%d\nhChime=%d\n",
                    g_alarmDays, g_alarmMelody, (g_masterMute ? 1 : 0), (g_hourlyChime ? 1 : 0));
    out += snprintf(s + out, sizeof(s) - out, "night=%d\nnStart=%d\nnEnd=%d\nnLedOff=%d\n",
                    (isItNightRightNow() ? 1 : 0), g_hNightStart, g_hNightEnd, (g_nightLedOff ? 1 : 0));

    // Blok 5: Sieć i Parametry połączenia
    out += snprintf(s + out, sizeof(s) - out, "wifi=%s\n", (WiFi.status() == WL_CONNECTED ? "connected" : "not_connected"));
    if (WiFi.status() == WL_CONNECTED) {
      out += snprintf(s + out, sizeof(s) - out, "ip=%s\nrssi=%d\nmdns=http://%s.local/\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI(), g_hostName.c_str());
    }

    // Blok 6: Wersja Oprogramowania (Firmware)
    out += snprintf(s + out, sizeof(s) - out, "ver=%s\n", FW_VERSION);

    server.send(200, "text/plain", s);
  });
  // ===========================================================================
  //        --- STRONA GŁÓWNA WEBUI W PORCJACH PO 1KB (Chunked Transfer) ---
  // ===========================================================================
  server.on("/config", HTTP_GET, []() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");  // Wysyłamy nagłówek HTTP

    const char *ptr = CONFIG_HTML;
    size_t fullLen = strlen_P(CONFIG_HTML);
    size_t sentLen = 0;
    const size_t chunkSize = 1024;  // Stabilne paczki 1KB, chroniące pamięć RAM Heap

    while (sentLen < fullLen) {
      size_t currentChunk = (fullLen - sentLen > chunkSize) ? chunkSize : (fullLen - sentLen);
      server.sendContent_P(ptr + sentLen, currentChunk);
      sentLen += currentChunk;

      // Oddajemy na moment czas procesora, aby stos WiFi na rdzeniu 0 nie gubił pakietów
      vTaskDelay(pdMS_TO_TICKS(1));
    }
    server.sendContent("");  // Zamykamy i kończymy transmisję HTML
  });

  // ===========================================================================
  //            --- PARAMETRYCZNY ENDPOINT ZMIAN W LOCIE (/set) ---
  // ===========================================================================
  server.on("/set", []() {
    if (server.hasArg("bright")) g_brightness = server.arg("bright").toInt();
    if (server.hasArg("auto")) g_autoBrightness = (server.arg("auto") == "1");
    if (server.hasArg("tOff")) g_tempOffset = server.arg("tOff").toFloat();
    if (server.hasArg("rDark")) g_rawDark = server.arg("rDark").toInt();
    if (server.hasArg("rBright")) g_rawBright = server.arg("rBright").toInt();

    if (server.hasArg("nStart")) {
      int val = server.arg("nStart").toInt();
      g_hNightStart = constrain(val, 0, 23);
    }
    if (server.hasArg("nEnd")) {
      int val = server.arg("nEnd").toInt();
      g_hNightEnd = constrain(val, 0, 23);
    }

    // SELEKTOR SPRZĘTOWY: applyBrightness steruje fizycznym PWM tylko w architekturze LED
#ifndef DISPLAY_TYPE_NEOPIXEL
    applyBrightness(g_brightness);
#endif

    if (server.hasArg("alTime")) {
      String t = server.arg("alTime");
      g_alarmH = t.substring(0, 2).toInt();
      g_alarmM = t.substring(3, 5).toInt();
    }
    if (server.hasArg("alOn")) g_alarmActive = (server.arg("alOn") == "1");
    if (server.hasArg("stopAlarm")) g_isAlarming = false;
    if (server.hasArg("alDays")) g_alarmDays = server.arg("alDays").toInt();
    if (server.hasArg("alMel")) g_alarmMelody = server.arg("alMel").toInt();
    if (server.hasArg("mMute")) g_masterMute = (server.arg("mMute") == "1");
    if (server.hasArg("hChime")) g_hourlyChime = (server.arg("hChime") == "1");

    if (server.hasArg("bzVol")) {
      g_buzzerVol = server.arg("bzVol").toInt();
      beep(2000, 30, false);  // Krótki odsłuch dźwięku w locie podczas ruchu suwaka głośności
    }

    if (server.hasArg("nLedOff")) {
      g_nightLedOff = (server.arg("nLedOff") == "1");
    }

// --- NOWA OBSŁUGA KOLORÓW RGB W FORMACIE HEX ---
#ifdef DISPLAY_TYPE_NEOPIXEL
    if (server.hasArg("hcolor")) {
      String hex = server.arg("hcolor");
      if (hex.startsWith("#")) hex = hex.substring(1);
      long number = strtol(hex.c_str(), NULL, 16);
      g_colTimeR = (number >> 16) & 0xFF;
      g_colTimeG = (number >> 8) & 0xFF;
      g_colTimeB = number & 0xFF;
    }
    if (server.hasArg("scolor")) {
      String hex = server.arg("scolor");
      if (hex.startsWith("#")) hex = hex.substring(1);
      long number = strtol(hex.c_str(), NULL, 16);
      g_colSecR = (number >> 16) & 0xFF;
      g_colSecG = (number >> 8) & 0xFF;
      g_colSecB = number & 0xFF;
    }
    if (server.hasArg("ccolor")) {
      String hex = server.arg("ccolor");
      if (hex.startsWith("#")) hex = hex.substring(1);
      long number = strtol(hex.c_str(), NULL, 16);
      g_colColonR = (number >> 16) & 0xFF;
      g_colColonG = (number >> 8) & 0xFF;
      g_colColonB = number & 0xFF;
    }
    if (server.hasArg("tcolor")) {
      String hex = server.arg("tcolor");
      if (hex.startsWith("#")) hex = hex.substring(1);
      long number = strtol(hex.c_str(), NULL, 16);
      g_colTempR = (number >> 16) & 0xFF;
      g_colTempG = (number >> 8) & 0xFF;
      g_colTempB = number & 0xFF;
    }
    if (server.hasArg("acolor")) {
      String hex = server.arg("acolor");
      if (hex.startsWith("#")) hex = hex.substring(1);
      long number = strtol(hex.c_str(), NULL, 16);
      g_colAmbientR = (number >> 16) & 0xFF;
      g_colAmbientG = (number >> 8) & 0xFF;
      g_colAmbientB = number & 0xFF;
    }
    if (server.hasArg("effect")) {
      g_ledEffectMode = server.arg("effect").toInt();
    }
    if (server.hasArg("tAuto")) g_tempColorAuto = (server.arg("tAuto") == "1");
#endif

    server.send(200, "text/plain", "OK");
  });

  // ===========================================================================
  //                --- ENDPOINT TRWAŁEGO ZAPISU DO NVS (/save) ---
  // ===========================================================================
  server.on("/save", []() {
    saveSettings();
    server.send(200, "text/plain", "Zapisano !");
  });

  server.on("/reset", []() {
    resetSettings();
    server.send(200, "text/plain", "OK: reset");
  });

  server.on("/reboot", []() {
    server.send(200, "text/plain", "Restartowanie...");
    xTaskCreate([](void *pv) {
      vTaskDelay(pdMS_TO_TICKS(2000));
      ESP.restart();
    },
                "reboot", 2048, NULL, 5, NULL);
  });

  server.onNotFound([]() {
    server.sendHeader("Location", "/config", true);
    server.send(302, "text/plain", "");
  });

  // Uruchomienie i montaż portalu AutoConnect
  ota.attach(portal);
  portal.begin();

  // Rozruch serwera Captive Portal DNS dla trybu Access Point (AP)
  vTaskDelay(pdMS_TO_TICKS(500));
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  Serial.print("DNS Server started on IP: ");
  Serial.println(WiFi.softAPIP());

  if (MDNS.begin(g_hostName.c_str())) {
    MDNS.addService("http", "tcp", 80);
  }

  bool firstSyncDone = false;

  // ===========================================================================
  //            --- NIESKOŃCZONA PĘTLA ZADANIA SIECIOWEGO WIFI ---
  // ===========================================================================
  for (;;) {
    // Automatyczna synchronizacja czasu po raz pierwszy, gdy tylko złapiemy zasięg routera
    if (!firstSyncDone && WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi połączone, uruchamiam NTP... v1.9");
      setupTime();
      firstSyncDone = true;
    }

    wifiWatchdog();

    if (WiFi.getMode() & WIFI_AP) {
      // W trybie punktu dostępowego (konfiguracji) serwer DNS wymaga pełnego priorytetu
      dnsServer.processNextRequest();
      portal.handleClient();
      taskYIELD();  // Elastyczne oddanie priorytetu FreeRTOS bez usypiania [INDEX]
    } else {
      // W trybie normalnej pracy odpytujemy klienta sieciowego rzadziej, oszczędzając CPU
      portal.handleClient();
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}


// =============================================================================
//          --- GŁÓWNA PROCEDURA URUCHOMIENIOWA SYSTEMU (SETUP) ---
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(10);  // Krótki czas na ustabilizowanie UART portu szeregowego
  Serial.println("\n🚀 SYSTEM: Rozpoczynam rozruch MyClock ESP32 v1.9...");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Optymalizacja parametrów sieciowych pod menedżer AutoConnect
  WiFi.setAutoReconnect(false);
  WiFi.persistent(false);

  // Ustalenie interwału odpytywania serwerów czasu (Standardowo: 1.5 godziny)
  sntp_set_sync_interval(5400000);

  // Rejestracja callbacku powiadomienia o udanej synchronizacji czasu NTP
  sntp_set_time_sync_notification_cb(timeSyncCallback);

  // Odczyt konfiguracji użytkownika z pamięci nieulotnej NVS
  loadSettings();

  // Wspólna inicjalizacja czujnika jasności LDR dla obu wersji hardware
  pinMode(PIN_LDR_ADC, INPUT);
  analogReadResolution(12);  // Ustawienie pełnej rozdzielczości 12-bit (0-4095)

  // SELEKTOR ARCHITEKTURY: Inicjalizacja startowa wyświetlaczy
#ifdef DISPLAY_TYPE_NEOPIXEL
  // SILNIK NEOPIXEL (SK6812):
  // Na starcie celowo milczymy. Hardware paska zostanie obudzony asynchronicznie
  // w BrightnessTask dopiero wtedy, gdy AutoConnect pomyślnie połączy się z WiFi! [INDEX]
  Serial.println("📺 SPRZĘT: Wybrano architekturę paska NeoPixel. Aktywowano opóźniony start (Late Binding).");
#else
  // STARY SYSTEM LED: Natychmiastowy start rejestrów przesuwnych i wpisanie kresek startowych
  Serial.println("📺 SPRZĘT: Wybrano architekturę klasycznych wyświetlaczy 7-Segmentowych LED.");
  initDisplayHardware();
  g_displayNext[0] = FONT_MINUS;
  g_displayNext[1] = FONT_MINUS;
  g_displayNext[2] = FONT_MINUS;
  g_displayNext[3] = FONT_MINUS;
  commitDisplayBuffer();

  allDigitsOff();
  write595(0);

  initBrightnessHardware();
#endif

  // Rozruch czujnika temperatury Dallas (1-Wire)
  sensors.begin();
  sensors.setWaitForConversion(false);  // Wyłączenie blokowania wątku na czas konwersji bitów [INDEX]

  // Konfiguracja sprzętowego Watchdoga (WDT) zabezpieczającego przed zawieszeniem pętli
  esp_task_wdt_init(30, true);  // 30 sekund tolerancji przed twardym resetem procesora
  esp_task_wdt_add(NULL);       // Objęcie monitoringiem głównego wątku (rdzeń 1, pętla loop)

  // SELEKTOR SPRZĘTOWY: Aktywacja timera przerwaniowego multipleksacji (Tylko dla starych kości LED) [INDEX]
#ifndef DISPLAY_TYPE_NEOPIXEL
  displayTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(displayTimer, &onDisplayTimer, true);
  timerAlarmWrite(displayTimer, FRAME_US, true);
  timerAlarmEnable(displayTimer);
#endif

  // Rozruch sekcji audio (Buzzer) oraz zadania odtwarzania melodii budzika
#if HAS_BUZZER
  initBuzzer();
  xTaskCreate(AlarmTask, "Alarm", 2048, NULL, 1, NULL);
#endif

  // ===========================================================================
  //        --- ARCHITEKTURA WIELOZADANIOWA FREERTOS (HARMONOGRAM) ---
  // ===========================================================================

  // Zadanie czasu: Rdzeń 1, Priorytet 2 (Wysoki priorytet dla sztywnych sekund)
  xTaskCreatePinnedToCore(TimeTask, "Time", 4096, nullptr, 2, nullptr, 1);

  // Zadanie temperatury: Przeniesione na RDZEŃ 0, Priorytet 1 (Izolacja krzemowa chroniąca pasek przed tęczą) [INDEX]
  xTaskCreatePinnedToCore(TempTask, "Temp", 4096, nullptr, 1, nullptr, 0);

  // Zadanie automatyki jasności i renderowania ekranu: Rdzeń 1, Priorytet 1
  xTaskCreatePinnedToCore(BrightnessTask, "Bright", 2048, nullptr, 1, nullptr, 1);

  // Zadanie portalu sieciowego i obsługi stron www: Rdzeń 0, Priorytet 1 (Mieszka obok TempTask) [INDEX]
  xTaskCreatePinnedToCore(WiFiTask, "WiFi", 8192, nullptr, 1, nullptr, 0);

  // Zadanie starej logiki (LED): Tworzone WYŁĄCZNIE gdy nie używamy paska NeoPixel! [INDEX]
#ifndef DISPLAY_TYPE_NEOPIXEL
  xTaskCreatePinnedToCore(LogicTask, "Logic", 4096, nullptr, 1, nullptr, 1);
#endif

  Serial.println("📋 HARMONOGRAM: Wszystkie zadania FreeRTOS zostały pomyślnie przydzielone do rdzeni. Soft v1.9 aktywny.");
}

// =============================================================================
//      --- PĘTLA GŁÓWNA URZĄDZENIA (Zajmuje się wyłącznie monitoringiem) ---
// =============================================================================
void loop() {
  esp_task_wdt_reset();             // Regularne "karmienie" psa - brak tej linii wywoła restart po 30s
  vTaskDelay(pdMS_TO_TICKS(1000));  // Usypiamy główny wątek na sekundę, zwalniając zasoby dla FreeRTOS
}
