// ==========================================
//   SNAKE – wersja PicoMesh 480×320 ST7796
//   - segmenty jako cyfry 0–9
//   - jedzenie jako cyfra 0–9
//   - brak migotania (kasowanie tylko ogona)
//   - kolizja ze ścianami i ogonem

// ==========================================

// ---- Stałe gry ----
#define CELL 10                     // wielkość jednego pola w pikselach
#define GRID_W 48                   // liczba pól w poziomie (480 / 10)
#define GRID_H 32                   // liczba pól w pionie (320 / 10)

// ---- Tablice pozycji węża ----
int snakeX[512];                    // tablica X segmentów węża
int snakeY[512];                    // tablica Y segmentów węża
int snakeLen = 5;                   // aktualna długość węża

// ---- Kierunek ruchu ----
int dirX = 1;                       // kierunek X (1 = prawo)
int dirY = 0;                       // kierunek Y (0 = brak ruchu góra/dół)

// ---- Jedzenie ----
int foodX = 10;                     // pozycja X jedzenia
int foodY = 10;                     // pozycja Y jedzenia

// ---- Timing ----
unsigned long lastStep = 0;         // czas ostatniego kroku gry
int speedMs = 150;                  // prędkość gry w ms

// ---- Ostatni segment do skasowania ----
int lastTailX = -1;                 // poprzednia pozycja ogona X
int lastTailY = -1;                 // poprzednia pozycja ogona Y

// ---- Stan gry ----
bool snakeGameOver = false;         // czy gra jest w stanie GAME OVER
bool snakeStartScreen = true;       // czy jesteśmy na ekranie startowym
bool snakePauseMenu = false;   // czy jesteśmy w ESC-menu

// ==========================================
//   Losowanie jedzenia (nie na krawędzi)
// ==========================================
void spawnFood() {

    while (true) {                                      // powtarzaj aż znajdziesz dobre miejsce
        foodX = random(1, GRID_W - 1);                  // losuj X od 1 do GRID_W-2
        foodY = random(1, GRID_H - 1);                  // losuj Y od 1 do GRID_H-2

        bool ok = true;                                 // flaga poprawności

        for (int i = 0; i < snakeLen; i++) {            // sprawdź czy jedzenie nie jest na wężu
            if (snakeX[i] == foodX && snakeY[i] == foodY) {
                ok = false;                             // kolizja z wężem → losuj dalej
                break;
            }
        }

        if (ok) return;                                 // poprawna pozycja → koniec
    }
}
//==================== START SCREEN ================
void uiSnakeStartScreen() {

    tft.fillScreen(TFT_BLACK);                          // wyczyść ekran
    tft.drawRect(0, 0, 480, 320, TFT_YELLOW);           // ramka

    tft.setTextColor(TFT_GREEN, TFT_BLACK);             // kolor tytułu
    tft.setTextSize(3);                                 // duża czcionka
    tft.setCursor(120, 80);                             // pozycja tytułu
    tft.print("S N A K E");                             // tytuł gry

    tft.setTextColor(TFT_WHITE, TFT_BLACK);             // kolor instrukcji
    tft.setTextSize(2);                                 // średnia czcionka
    tft.setCursor(100, 150);                            // pozycja instrukcji
    tft.println("Press ENT to start");  
     tft.setCursor(100, 170);                      // instrukcja startu
    tft.println("Press ESC to Main Menu");                    // instrukcja startu

    tft.setTextSize(1);                                 // mała czcionka
    tft.setCursor(100, 200);                            // pozycja info
    tft.print("LEFT/RIGHT = turn");                     // sterowanie

    snakeStartScreen = true;                            // ustaw stan startowy
}
// ============ PAUZE MENU
void uiSnakePauseMenu() {

    snakePauseMenu = true;                                // zatrzymaj grę

    tft.fillRect(100, 100, 280, 120, TFT_BLACK);          // tło okna
    tft.drawRect(100, 100, 280, 120, TFT_YELLOW);         // ramka

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);              // kolor tytułu
    tft.setTextSize(2);                                   // większa czcionka
    tft.setCursor(140, 120);                              // pozycja
    tft.print("WYCHODZIMY??");                            // tekst

    tft.setTextColor(TFT_WHITE, TFT_BLACK);               // kolor instrukcji
    tft.setTextSize(1);                                   // mniejsza czcionka
    tft.setCursor(140, 160);
    tft.print("ENT = wracamy do gry");

    tft.setCursor(140, 190);
    tft.print("ESC = wychodzimy");
}

// ==========================================
//   Ekran GAME OVER
// ==========================================
void uiSnakeGameOver() {

    snakeGameOver = true;                               // ustaw stan GAME OVER

    tft.fillRect(100, 100, 280, 120, TFT_BLACK);        // czarne tło okna
    tft.drawRect(100, 100, 280, 120, TFT_RED);          // czerwona ramka

    tft.setTextColor(TFT_RED, TFT_BLACK);               // kolor napisu
    tft.setTextSize(2);                                 // rozmiar czcionki
    tft.setCursor(140, 120);                            // pozycja tekstu
    tft.print("GAME OVER");                             // napis GAME OVER

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);            // kolor wyniku
    tft.setCursor(140, 160);                            // pozycja wyniku
    tft.print("LEN: ");                                 // tekst LEN:
    tft.print(snakeLen);                                // długość węża

    tft.setTextColor(TFT_WHITE, TFT_BLACK);             // kolor instrukcji
    tft.setTextSize(1);                                 // mniejsza czcionka
    tft.setCursor(140, 200);                            // pozycja instrukcji
    tft.print("Press ENT to restart");                  // instrukcja restartu
}

// ==========================================
//   Inicjalizacja gry
// ==========================================
void uiSnakeInit() {

    tft.fillScreen(TFT_BLACK);                          // wyczyść ekran
    tft.drawRect(0, 0, 480, 320, TFT_YELLOW);           // narysuj ramkę

    snakeLen = 5;                                       // reset długości

    for (int i = 0; i < snakeLen; i++) {                // ustaw startową pozycję węża
        snakeX[i] = 20 - i;                             // segmenty obok siebie
        snakeY[i] = 15;
    }

    dirX = 1;                                           // start: w prawo
    dirY = 0;

    lastTailX = -1;                                     // brak poprzedniego ogona
    lastTailY = -1;

    snakeGameOver = false;                              // gra aktywna

    spawnFood();                                        // nowe jedzenie
    uiSnakeDraw();                                      // narysuj pierwszy stan
}

// ==========================================
//   Rysowanie gry (bez migotania)
// ==========================================
void uiSnakeDraw() {

    if (lastTailX >= 0) {                               // jeśli jest ogon do skasowania
        tft.fillRect(lastTailX * CELL,                  // kasuj X ogona
                      lastTailY * CELL,                 // kasuj Y ogona
                      CELL, CELL,                       // rozmiar pola
                      TFT_BLACK);                       // kolor tła
    }

    char foodBuf[2];                                    // bufor na cyfrę jedzenia
    foodBuf[0] = '0' + (snakeLen % 10);                 // jedzenie = cyfra 0–9
    foodBuf[1] = 0;                                     // koniec stringa

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);               // kolor jedzenia
    tft.setTextSize(1);                                 // rozmiar czcionki
    tft.setCursor(foodX * CELL + 2, foodY * CELL + 2);  // pozycja jedzenia
    tft.print(foodBuf);                                 // rysuj jedzenie

    for (int i = 0; i < snakeLen; i++) {                // rysuj każdy segment

        tft.fillRect(snakeX[i] * CELL,                  // X segmentu
                      snakeY[i] * CELL,                 // Y segmentu
                      CELL, CELL,                       // rozmiar pola
                      TFT_BLACK);                       // tło segmentu

        tft.setTextColor(TFT_GREEN, TFT_BLACK);         // kolor segmentu
        tft.setTextSize(1);                             // rozmiar czcionki

        char buf[2];                                    // bufor cyfry segmentu
        buf[0] = '0' + (i % 10);                        // cyfra 0–9
        buf[1] = 0;                                     // koniec stringa

        tft.setCursor(snakeX[i] * CELL + 2,             // pozycja tekstu X
                       snakeY[i] * CELL + 2);           // pozycja tekstu Y
        tft.print(buf);                                 // rysuj cyfrę segmentu
    }
}

// ==========================================
//   Jeden krok gry
// ==========================================
void uiSnakeStep() {

    int newHeadX = snakeX[0] + dirX;                    // oblicz nową pozycję głowy X
    int newHeadY = snakeY[0] + dirY;                    // oblicz nową pozycję głowy Y

    if (newHeadX <= 0 || newHeadX >= GRID_W - 1 ||      // kolizja ze ścianą X
        newHeadY <= 0 || newHeadY >= GRID_H - 1) {      // kolizja ze ścianą Y
        uiSnakeGameOver();                              // GAME OVER
        return;
    }

    for (int i = 0; i < snakeLen; i++) {                // kolizja z ogonem
        if (newHeadX == snakeX[i] && newHeadY == snakeY[i]) {
            uiSnakeGameOver();                          // GAME OVER
            return;
        }
    }

    lastTailX = snakeX[snakeLen - 1];                   // zapamiętaj ogon X
    lastTailY = snakeY[snakeLen - 1];                   // zapamiętaj ogon Y

    for (int i = snakeLen - 1; i > 0; i--) {            // przesuń ogon
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    snakeX[0] = newHeadX;                               // ustaw nową głowę X
    snakeY[0] = newHeadY;                               // ustaw nową głowę Y

    if (snakeX[0] == foodX && snakeY[0] == foodY) {     // zjedzenie jedzenia
  playTone(3000, 30, 0.4f);  
        snakeLen++;                                     // wydłuż węża
        spawnFood();                                    // nowe jedzenie
    }
}

// ==========================================
//   Obsługa sterowania (obrót L/P)
// ==========================================
void uiHandleSnake(const String &key) {
// ==========================
//   OBSŁUGA ESC-MENU
// ==========================
if (snakePauseMenu) {                       // jeśli jesteśmy w ESC-menu

    if (key == "ent") {                     // ENTER = wracamy do gry
        snakePauseMenu = false;             // zamknij menu
        uiSnakeDraw();                      // odśwież grę
    }

    if (key == "esc") {                     // ESC = wychodzimy
        snakePauseMenu = false;             // zamknij menu
        snakeStartScreen = true;            // wróć do ekranu startowego
        uiSnakeStartScreen();               // pokaż start
    }

    return;                                 // blokuj ruch gry
}
  
// ==========================
//   EKRAN STARTOWY
// ==========================
if (snakeStartScreen) {                                 // jeśli jesteśmy na ekranie startowym
if (key == "esc" ) {
    uiState = UI_IDLE;           // wracamy do głównego menu PicoMesh
     tft.fillScreen(TFT_BLACK);  // Czyscimy ekran   
  uiDrawIdleScreen() ; 
  uiHandle();               // rysujemy menu główne
    return;
}
    if (key == "ent") {                                 // ENTER = start gry
        snakeStartScreen = false;                       // wyłącz ekran startowy
        uiSnakeInit();                                  // uruchom grę
    }

    return;                                             // blokuj sterowanie gry
}

    if (snakeGameOver) {                                // jeśli GAME OVER
        if (key == "ent") {                             // ENTER = restart
            uiSnakeInit();                              // restart gry
        }
        return;                                         // blokuj ruch
    }
if (key == "esc") {                         // ESC podczas gry
    uiSnakePauseMenu();                     // pokaż okno
    return;
}

    if (key == "rt") {                                  // obrót w lewo
        int oldX = dirX;
        dirX = -dirY;
        dirY = oldX;
    }

    if (key == "lt") {                                  // obrót w prawo
        int oldX = dirX;
        dirX = dirY;
        dirY = -oldX;
    }

    unsigned long now = millis();                       // aktualny czas
    if (now - lastStep >= speedMs) {                    // czas na krok gry
        lastStep = now;                                 // zapisz czas
        uiSnakeStep();                                  // wykonaj krok
        uiSnakeDraw();                                  // narysuj stan
    }
}
