
#include "hardware/clocks.h"
#include <Arduino.h>
#include <RadioLib.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>

#include <TinyGPSPlus.h>
#include <TFT_eSPI.h>
#include "font12x16.h"
#include <I2S.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#define NODE_ID "BAX"  // unikalne ID węzła (String)
// RZĘDY – GPB0..GPB7
uint8_t rows[8] = {8, 9, 10, 11, 12, 15, 13, 14};
// KOLUMNY – GPA0..GPA6
uint8_t cols[7] = {0, 1, 2, 3, 4, 5, 6};
// LED SHIFT = GPA7
#define SHIFT_LED 7 // dioda podpieta do MCP

// MAPA KLAWISZY — TUTAJ WPISZEMY TWOJE
  String key = "";
String keymap[8][7] = {
    {"]",  "esc",  "7",  "u",  "`",  "j",  "["},
    {"v",  "r",  "dn",  ".", "4", "m", "f"},
    {"-", ";", "up", "=", "fn", ",", "'"},
    {"z", "q", "8", "i", "1", "k", "a"},
    {"n", "y", "lt", " ", "6", "ent", "h"},
    {"b", "t", "rt", "/", "5", "\\", "g"},
    {"c", "e", "0", "p", "3", "del", "d"},
    {"x", "w", "9", "o", "2", "l", "s"}
};
String keymapFN[8][7] = {
    {"{",  "esc",  "&",  "U",  "~",  "J",  "}"},
    {"V",  "R",  "dn",  ">",  "$",  "M",  "F"},
    {"_",  ":",  "up",  "+",  "fn",  "<",  "\""},
    {"Z",  "Q",  "*",  "I",  "!",  "K",  "A"},
    {"N",  "Y",  "lt", " ",  "^",  "ent", "H"},
    {"B",  "T",  "rt", "?",  "%",  "|",  "G"},
    {"C",  "E",  ")",  "P",  "#",  "del", "D"},
    {"X",  "W",  "(",  "O",  "@",  "L",  "S"}
};
#define GPS_OFF 0
// I2S
#define pBCLK 1
#define pWS   2
#define pDOUT 3

// I2C (MCP23017 + Touch)
#define MCP_SDA 4
#define MCP_SCL 5

// Touch panel (I2C)
#define TOUCH_SDA 4
#define TOUCH_SCL 5
#define TOUCH_RST 6
#define TOUCH_INT 7
// SD card (SPI0)
#define SD_CS    8
// LoRa (SPI1)
#define LORA_BUSY   9
#define LORA_SCK   10
#define LORA_MOSI  11
#define LORA_MISO  12
#define LORA_CS    13
#define LORA_DIO1  14
#define LORA_RST   15

// GPS

#define GPS_RX   17

// TFT ST7796 (SPI0)
#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_BL   20
#define TFT_CS   21
#define TFT_DC   22
#define TFT_RST  26
#define TFT_MISO -1
#define SD_MISO 16


// Battery ADC
#define VBAT_PIN 28   // ADC2

// Tworzymy I2S
I2S i2s(OUTPUT, pBCLK, pDOUT, pWS);

int brightness = 128;  // start 50%
const int sampleRate = 16000;   // częstotliwość próbkowania
bool DEBUG = false;

// ======= Definicje ============
Adafruit_MCP23X17 mcp;
TinyGPSPlus gps;
SX1262 radio = new Module(
  LORA_CS,
  LORA_DIO1,
  LORA_RST,
  LORA_BUSY,
  SPI1
);
TFT_eSPI tft = TFT_eSPI();
const char* ssid = "NETIASPOT-2.4GHz-s5HB";
const char* pass = "jvH3452M";
// ======================= Struktura pakietu ===================
//           MSG|-OD-|-DO-|NUMER|ttl|wiadomosc
// definicja struktury pakietu używana w systemie
struct Packet {  // format: type|src|dst|mid|ttl|data
  String type;   // typ: "MSG", "ACK", "BEA", "CMD"
  String src;    // ID nadawcy
  String dst;    // ID odbiorcy lub "ALL"
  int mid;       // ID wiadomości (100-999)
  int ttl;       // TTL (ilość skoków dozwolonych)
  String data;   // dane/payload jako tekst
  int retries;   // licznik prób (przy wysyłaniu własnych MSG)
};
//################### BUFOR [RX]
String rxPkt = "";
volatile bool rxReady = false;
// ======================= Cache — ochrona przed duplikatami ==================
struct CacheEntry {
  String src;  // skąd pochodził pakiet
  int mid;     // identyfikator wiadomości
};
#define MAX_CACHE 20
CacheEntry packetCache[MAX_CACHE];
int cacheCount = 0;  // ile aktualnie elementów w cache

// ======================= Lista Węzłów (sąsiadów) ======================
#define MAX_NODES 30
struct NodeInfo {
  String id;               // ID węzła
  unsigned long lastSeen;  // czas ostatniego kontaktu (millis)
  int hops;                // ile hopów od nas (na bazie TTL)
  double lat;              // ostatnia znana szer. geogr.
  double lon;              // ostatnia znana dł. geogr.
  float alt;               // wysokość
  float battery;           // poziom baterii (V)
  int rssi;                // RSSI ostatniego pakietu
};
NodeInfo nodeList[MAX_NODES];
int nodeCount = 0;  // ile węzłów zapamiętano
// ============ STRUKTURA LISTY PAKIETOW RX-TX =============
int sentCount = 0;
int recvCount = 0;
#define MAX_MSG_LEN 120

struct MsgRecord {
  char src[8];
  char dst[8];
  char text[MAX_MSG_LEN];
  uint32_t timestamp;
  bool delivered;
  int mid;  // <--- DODANE
};
// ========== HISTORY RX TX ==========
#define MSG_HISTORY 25
MsgRecord sent[MSG_HISTORY];
MsgRecord recv[MSG_HISTORY];
// ======= Kolejka wysyłki =======
#define MAX_QUEUE 20
Packet sendQueue[MAX_QUEUE];  // tablica reprezentująca kolejkę (circular buffer)
int queueStart = 0;           // indeks początku kolejki
int queueEnd = 0;             // indeks końca kolejki (następne wolne miejsce)
// =========== PENDING RETRIES =================
struct PendingRetry {
  bool used;                  // czy slot jest używany
  Packet pkt;                 // skopiowany pakiet MSG
  uint8_t retries;            // ile prób wykonano
  unsigned long nextRetryAt;  // kiedy spróbować ponownie (millis)
};
#define MAX_PENDING 10
PendingRetry pendingRetries[MAX_PENDING];   // tablica slotów na pending retries
const unsigned long RETRY_INTERVAL = 15000;  // ms - odstęp pomiędzy retry
const uint8_t MAX_MSG_RETRIES = 5;          // maksymalna liczba prób dla jednego MSG
// ============ LoRa ================
int transmissionState = RADIOLIB_ERR_NONE;
bool transmitFlag = false;
volatile bool operationDone = false;
// =======  TIMERS =======
unsigned long lastBEA = 0;                               // czas ostatniego wysłania BEA (beacon)
unsigned long lastTFToff = 0;
unsigned long nextTxTime = 0;
// ????????????
unsigned long lastPress[8][7];
const unsigned long debounceTime = 300;
bool fnLock = false;

String lastRx = "";
String lastGps = "";

String received;
unsigned long lastTx = 0;

// ================= PROFILER =================
unsigned long t_loopStart = 0;
unsigned long t_loopEnd = 0;
unsigned long t_lastLoop = 0;
unsigned long t_maxLoop = 0;

//============== FLAGA ODBIORU PAKIETU ===================
void setFlag(void) {
  operationDone = true;
}



