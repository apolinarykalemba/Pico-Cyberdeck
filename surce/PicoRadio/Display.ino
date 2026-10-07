
void StartTFT(){
 // === TFT ===
  tft.init();
  tft.setRotation(3);
 tft.invertDisplay(true); 
 tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  pinMode(TFT_BL, OUTPUT);
 analogWrite(TFT_BL, 128); // 50% 
 // digitalWrite(TFT_BL, HIGH);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);                                                                                                                                                                    
  tft.setTextSize(2);

}
//================  ZMIENNE RYSOWANIA ================
// Ile linii ma ekran (12 widocznych + 2 zapasowe)
const int UI_LINES = 15;// Maksymalna długość tekstu w jednej linii (40 znaków + null)
const int LINE_LEN = 41;// 40 znaków widocznych + 1 znak '\0' na końcu stringa 
struct Line { char text[LINE_LEN]; }; // Bufor jednej linii ekranu 
Line screenBuffer[UI_LINES];  // AKTUALNY BUFOR EKRANU 
Line oldScreenBuffer[UI_LINES]; // STARY BUFOR EKRANU
 int uiStep = 0;       // Numer linii, którą sprawdzamy w danym obiegu loop() 
// ------------------ RYSOWANIE NA TFT ------------------
// ================== RYSOWANIE NA EKRANIE JEDNEJ LINII ==============
void drawLineText(int i, const char* txt) { 
const int lineHeight = 19; 
const int lineWidth = 480; 
const int y = (i + 1) * lineHeight; 
uint16_t fg; // Kolor pierwszej linii 
if (i == 0) fg = TFT_CYAN; // Kolor drugiej linii 
else if (i == 1) fg = TFT_WHITE; // Pozostałe linie 
else fg = TFT_GREEN; // Bufor pojedynczej linii obrazu 
uint16_t *lineBuf = (uint16_t*)malloc(lineWidth * lineHeight * 2); // Tło całej linii = czarne 
memset(lineBuf, TFT_BLACK, lineWidth * lineHeight * 2); // Narysowanie tekstu do RAM 
printToRAM(lineBuf, lineWidth, txt, fg); // Wysłanie tylko tej jednej linii na TFT 
tft.pushImage(0, y + 2, lineWidth, lineHeight, lineBuf); // Zwolnienie pamięci 
free(lineBuf); 
}

// =========== KONTROLA CZY TRZEBA RYSOWAĆ =================
void CheckRedraw() { 
  // Porównanie aktualnej linii z poprzednią 
if (strcmp(screenBuffer[uiStep].text, oldScreenBuffer[uiStep].text) != 0) { // Linia się zmieniła → rysujemy ją 
        // DEBUG – linia się zmieniła
 //       Serial.printf("REDRAW [%02d]: \"%s\"\n", uiStep,screenBuffer[uiStep].text);
drawLineText(uiStep, screenBuffer[uiStep].text); // Zapamiętujemy nową zawartość jako aktualnie // wyświetlaną na TFT 
strcpy(oldScreenBuffer[uiStep].text, screenBuffer[uiStep].text); } // Przechodzimy do następnej linii 
uiStep++; // Po 14 liniach wracamy do początku 
if (uiStep >= UI_LINES) uiStep = 0; 
}

void setLine(int i, const char* newText) { 
int len = strlen(newText); // Maksymalnie 40 znaków 
if (len >= LINE_LEN) len = LINE_LEN - 1; // Kopiujemy właściwy tekst 
memcpy(screenBuffer[i].text, newText, len); // Resztę linii wypełniamy spacjami 
for (int j = len; j < LINE_LEN - 1; j++) screenBuffer[i].text[j] = ' '; // Koniec stringa 
screenBuffer[i].text[LINE_LEN - 1] = '\0'; 
}



// =========== CZYSZCZENIE BUFORA EKRANU =================
void clearScreenBuffer() { 
  for (int i = 0; i < UI_LINES; i++) { // Pusta linia = 40 spacji 
  for (int j = 0; j < LINE_LEN - 1; j++) screenBuffer[i].text[j] = ' '; // Koniec stringa 
  screenBuffer[i].text[LINE_LEN - 1] = '\0'; } // Po zmianie ekranu zaczynamy sprawdzanie od linii 0 
  uiStep = 0; 
  }
void drawCharToRAM(uint16_t *buf, int bufWidth, int x, int y, uint8_t c, uint16_t color)
{
    const uint8_t *glyph = font12x16[c];
    for (int col = 0; col < 12; col++) {
        uint16_t bits = glyph[col * 2] | (glyph[col * 2 + 1] << 8);
        for (int row = 0; row < 16; row++) {   // <-- PEŁNE 16 WYSOKOŚCI
            if (bits & (1 << row)) {
                // FLIP Y (bo font jest odwrócony)
                int px = x + col;
                int py = y + row;
                buf[py * bufWidth + px] = color;
            }
        }
    }
}


void printToRAM(uint16_t *buf, int bufWidth, const char *txt, uint16_t color)
{
    int cx = 0;
    for (int i = 0; txt[i]; i++) {
        drawCharToRAM(buf, bufWidth, cx, 0, txt[i], color);
        cx += 12;  // szerokość fontu 12x16
    }
}



void offBacklight() {
  if (millis() - lastTFToff >= 30000) {  // 10 sekund
    digitalWrite(TFT_BL, LOW);
    lastTFToff = millis();
  set_sys_clock_khz(25000, true); // 25 MHz  
  }
}

void printTerminalText(const String& text) {
     tft.setTextSize(2); 
      tft.fillRect(0, 100, 320, 40, TFT_BLACK);   
     tft.setCursor(0, 300);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    tft.print(text);
}
void drawTopBar() {
    const int BAR_H = 19;

    tft.fillRect(0, 0, 480, BAR_H, TFT_BLACK);
    tft.drawLine(0, BAR_H, 480, BAR_H, TFT_RED);

//    updateGPSStatus();
    updateWiFiStatus();  
 updateMessageCount() ;  
    updateModeSymbol();
//    updateBattVoltage();
}

void updateBattVoltage() {
    const int BAR_H = 18;

    // czyścimy obszar baterii (po WiFi)
    tft.fillRect(270, 0, 80, BAR_H, TFT_BLACK);

    tft.setTextSize(2);
    tft.setCursor(270, 0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

//    float vb = readBattery();   // Twoja funkcja
 //   tft.print(vb, 2);
 //   tft.print("V");
}



void updateGPSStatus() {
    const int BAR_H = 18;

    // Czyścimy tylko obszar GPS (lewa część belki)
    tft.fillRect(0, 0, 80, BAR_H, TFT_BLACK);

    tft.setTextSize(2);

    // ---------------------------------------------------------
    //  FIX / X
    // ---------------------------------------------------------
    bool hasFix = gps.location.isValid() && gps.location.age() < 2000;

    tft.setCursor(2, 0);
    if (hasFix) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print("FIX");
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print("X");
    }

    // ---------------------------------------------------------
    //  SAT: liczba satelitów
    // ---------------------------------------------------------
    tft.setCursor(60, 0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print("SAT:");
    tft.print(gps.satellites.value());

}
void updateMessageCount() {
    const int BAR_H = 18;

//    tft.fillRect(480 - 20, 0, 20, BAR_H, TFT_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(480 - 120, 0);
tft.print("IN ");

 tft.print(recvCount);
}

//============== UPGRADE TOP BAR 
void updateModeSymbol() {
    const int BAR_H = 18;

//    tft.fillRect(480 - 20, 0, 20, BAR_H, TFT_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(480 - 20, 0);
    if (fnLock == false) tft.print("a");
    else if (fnLock == true) tft.print("A");
    else tft.print("#");
}
void updateWiFiStatus() {
    const int BAR_H = 18;

    // Czyścimy środkową część belki (między GPS a MODE)
    tft.fillRect(130, 0, 140, BAR_H, TFT_BLACK);

    tft.setTextSize(2);

    bool connected = (WiFi.status() == WL_CONNECTED);

    // WiFi / brak WiFi
    tft.setCursor(130, 0);
    if (connected) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print("WiFi ");
        tft.print(WiFi.RSSI());
        tft.print("dBm ");
     
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print("WiFi ");
        tft.print("-- ");
 
    }
}
