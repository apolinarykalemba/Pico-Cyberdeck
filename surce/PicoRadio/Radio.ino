struct Station {
    char name[64];
    char url[256];
};

Station stations[32];
int stationCount = 0;
int currentStation = 1;
volatile bool metaReady = false;
char metaRaw[128];

const Station defaultStations[] = {
    { "TokFM", "http://poznan5-4.radio.pionier.net.pl:8000/tuba10-1.mp3" },
    { "Radio Nowy Swiat", "http://stream.rcs.revma.com/ypqt40u0x1zuv" },
    { "RMF FM Poznan", "http://195.150.20.242:8000/rmf_fm" },
    { "Poznan Nadaje", "http://stream4.nadaje.com:8578/poznan" },
    { "Antyradio", "http://poznan7.radio.pionier.net.pl:8000/tuba9-1.mp3" },
    { "Trojka", "http://mp3.polskieradio.pl:8904/" },
    { "Radio Pogoda", "http://stream13.radioagora.pl/tuba38-1.mp3" },
    { "Radio Rock", "http://stream13.radioagora.pl/tuba9004-1.mp3" },
    {"RadioBEAT-PARTY" , "http://rs102-krk.rmfstream.pl/rmf_polska_alternatywa"},
};

const int defaultStationCount =  sizeof(defaultStations) / sizeof(defaultStations[0]);



char redirectURL[512];  // Zmienne dla RADIRECT
bool redirectPending = false;
bool sdActive = false;  // Czy jest karta SD w slocie

bool loadStationsFromSD() {

    // --- 1. PRÓBA INICJALIZACJI SD ZA KAŻDYM RAZEM ---
    unsigned long t0 = millis();
    bool ok = SD.begin(SD_CS);

    if (!ok || (millis() - t0) > 150) {
        // --- SD NIE DZIAŁA → WYŁĄCZ SPI DLA SD ---
        if (sdActive) {
            Serial.println("SD OFF — brak karty lub timeout");
            sdActive = false;
        }
        return false;       // brak stacji z SD
    }

    // --- SD DZIAŁA → ZAPAMIĘTAJ STAN ---
    if (!sdActive) {
        Serial.println("SD ON — karta wykryta");
        sdActive = true;
    }

    // --- 2. WCZYTYWANIE STACJI ---
    File f = SD.open("/stations.txt");
    if (!f) return false;

    stationCount = 0;

    while (f.available() && stationCount < 32) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() < 3) continue;

        int sep = line.indexOf('|');
        if (sep < 0) continue;

        String name = line.substring(0, sep);
        String url  = line.substring(sep + 1);

        name.toCharArray(stations[stationCount].name, sizeof(stations[stationCount].name));
        url.toCharArray(stations[stationCount].url, sizeof(stations[stationCount].url));

        stationCount++;
    }

    f.close();
    return stationCount > 0;
}


String chooseStation() {

    // Jeśli SD nie dało stacji → fallback
    if (stationCount == 0) {
        stationCount = defaultStationCount;                     // liczba stacji domyślnych
        for (int i = 0; i < stationCount; i++) {                // kopiowanie nazw i URL
            strcpy(stations[i].name, defaultStations[i].name);
            strcpy(stations[i].url,  defaultStations[i].url);
   
        }
    }

    int count = stationCount;                                   // liczba stacji do wyświetlenia

    static int lastIndex = 0;                                   // ZAPAMIĘTANA POZYCJA MENU (static, nie global)

    // Przywracanie pozycji — jeśli zmieniła się liczba stacji, przycinamy
    if (lastIndex >= count) lastIndex = count - 1;              // zabezpieczenie przed wyjściem poza zakres
    if (lastIndex < 0)      lastIndex = 0;                      // zabezpieczenie dolne

    // Wyliczenie pozycji startowej kursora
    int row = lastIndex % 14;                                   // wiersz (0–13)
    int col = lastIndex / 14;                                   // kolumna (0–1)

    unsigned long lastKey = 0;                                  // debounce klawiszy

    drawStatusBar(false);                                       // rysujemy belkę INFO

    // --- Rysujemy linię HELP pod belką INFO ---
    tft.fillRect(0, 20, 480, 20, TFT_DARKGREY);                 // tło HELP
    tft.setCursor(4, 20);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
    tft.print("UP-DOWN   <>   DEL-start ESC-OUT");              // tekst pomocy

    // --- Funkcja rysująca jedną linię ---
    auto drawLine = [&](int r, int c, bool selected) {
        int index = c * 14 + r;                                 // obliczenie indeksu stacji
        if (index >= count) return;                             // nie rysujemy pustych pozycji

        int y = 40 + r * 20;                                    // Y zależne od wiersza (po HELP)
        int x = (c == 0 ? 0 : 240);                             // X zależne od kolumny

        tft.fillRect(x, y, 240, 20, TFT_BLACK);                 // tło linii
        tft.setCursor(x, y);
        tft.setTextSize(2);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);                 // kolor tekstu

        if (selected) tft.print("> ");                          // zaznaczenie
        else          tft.print("  ");

 char tmp[32];
strncpy(tmp, stations[index].name, 18); //    trymowanie do 18 znakow
tmp[18] = 0;
tft.print(tmp);                       // nazwa stacji
    };

    // --- Rysujemy tylko tyle linii, ile jest stacji ---
    int maxRows = min(count, 14);                               // max 14 wierszy na kolumnę

    for (int r = 0; r < maxRows; r++) {                         // rysujemy lewą kolumnę
        drawLine(r, 0, (r == row && col == 0));
    }
    for (int r = 0; r < maxRows; r++) {                         // rysujemy prawą kolumnę
        drawLine(r, 1, (r == row && col == 1));
    }

    // ---- Pętla wyboru ----
    while (true) {
        unsigned long now = millis();

        // GÓRA
        if (mcp.digitalRead(rows[2]) == LOW) {
            if (now - lastKey > 150) {
                lastKey = now;

                int oldRow = row;
                int oldCol = col;

                row--;                                          // przesuwamy w górę
                if (row < 0) row = maxRows - 1;                 // zawijanie

                drawLine(oldRow, oldCol, false);                // odznaczamy starą linię
                drawLine(row, col, true);                       // zaznaczamy nową
            }
        }

        // DÓŁ
        if (mcp.digitalRead(rows[1]) == LOW) {
            if (now - lastKey > 150) {
                lastKey = now;

                int oldRow = row;
                int oldCol = col;

                row++;                                          // przesuwamy w dół
                if (row >= maxRows) row = 0;                    // zawijanie

                drawLine(oldRow, oldCol, false);
                drawLine(row, col, true);
            }
        }

        // PRAWO
        if (mcp.digitalRead(rows[5]) == LOW) {
            if (now - lastKey > 150) {
                lastKey = now;

                int oldRow = row;
                int oldCol = col;

                col = 1;                                        // przejście do prawej kolumny

                if (col * 14 + row >= count) col = oldCol;      // jeśli brak stacji → wróć

                drawLine(oldRow, oldCol, false);
                drawLine(row, col, true);
            }
        }

        // LEWO
        if (mcp.digitalRead(rows[4]) == LOW) {
            if (now - lastKey > 150) {
                lastKey = now;

                int oldRow = row;
                int oldCol = col;

                col = 0;                                        // przejście do lewej kolumny

                drawLine(oldRow, oldCol, false);
                drawLine(row, col, true);
            }
        }

        // ZATWIERDZENIE (DEL)
        if (mcp.digitalRead(rows[6]) == LOW) {
            if (now - lastKey > 150) {
                lastKey = now;

                int index = col * 14 + row;                     // obliczenie indeksu stacji
                if (index < count) {

                    lastIndex = index;                          // ZAPAMIĘTANIE POZYCJI MENU

                    tft.fillScreen(TFT_BLACK);                  // czyszczenie ekranu
                    currentStation = index;                     // ustawienie aktualnej stacji
                    drawStatusBar(true);                        // rysowanie belki INFO
tft.drawRect(19, 95, 405, 70, TFT_CYAN);
 //   tft.fillRect(20, 100, 440, 60, TFT_BLACK);

                    return stations[currentStation].url;        // zwracamy URL
                }
            }
        }
    }
}




void drawStatusBar(bool isPlaying ) {

    tft.startWrite();  // TURBO MODE

    // Pasek 0–20 px
    tft.fillRect(0, 0, 240, 20, TFT_BLACK);

    // PLAY / STOP
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    if (isPlaying)
        tft.print("[>] ");
    else
        tft.print("[#] ");

    // Nazwa stacji
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.print(stations[currentStation].name);

    // Siła WiFi
    int rssi = WiFi.RSSI();
    int bars = 0;

    if (rssi > -55) bars = 4;
    else if (rssi > -65) bars = 3;
    else if (rssi > -75) bars = 2;
    else bars = 1;

    tft.setCursor(380, 0);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print("WiFi:");

    switch (bars) {
        case 1: tft.print("|"); break;
        case 2: tft.print("||"); break;
        case 3: tft.print("|||"); break;
        case 4: tft.print("||||"); break;
    }
     // --- Rysujemy linię HELP pod belką INFO ---
    tft.fillRect(0, 20, 480, 20, TFT_DARKGREY);                 // tło HELP
    tft.setCursor(4, 20);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
    tft.print("UP/DN - VOL DEL-st/stop ESC-OUT <> NEXT");              // tekst pomocy 

    tft.endWrite();  // KONIEC TURBO  
}


void radioNext() {
    currentStation++;
    if (currentStation >= stationCount) currentStation = 0;
    radioStop();
    radioStart(stations[currentStation].url);
}
void radioPrev() {
    currentStation--;
    if (currentStation < 0) currentStation = stationCount - 1 ;

    radioStop();
    radioStart(stations[currentStation].url);
}

void radioStart(const char* initialUrl) {

    Serial.print("Łączenie do stacji: ");
    Serial.println(initialUrl);

    WiFiClient client;
    HTTPClient http;
    const char* headerNames[] = { "Location" };
    String location = initialUrl;

    // --- sprawdzamy redirect HTTP ---
    http.begin(client, location);
    http.collectHeaders(headerNames, 1);

    int httpCode = http.GET();
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    switch (httpCode) {

        case 200:
            Serial.println("200 OK - Połączenie udane");
            break;

        case 301:
        case 302:
            Serial.println(String(httpCode) + " - Przekierowanie strumienia");
            location = http.header("Location");
            Serial.print("Redirect do: ");
            Serial.println(location);
            break;

        case 400: Serial.println("400 Bad Request - następna stacja");  radioNext();
        case 401: Serial.println("401 Unauthorized - następna stacja");  radioNext();
        case 403: Serial.println("403 Forbidden - następna stacja");  radioNext();
        case 404: Serial.println("404 Not Found - następna stacja");  radioNext();
        case 405: Serial.println("405 Method Not Allowed - następna stacja");  radioNext();
        case 500: Serial.println("500 Internal Server Error - następna stacja");  radioNext();

        default:
            Serial.print(httpCode);
            Serial.println(" - Nieobsługiwany kod - następna stacja");
            radioNext() ;
    }

    http.end();
drawStatusBar(true) ;
    // --- finalny URL po redirect ---
    location.toCharArray(url, location.length() + 1);

    // --- kasujemy stare obiekty audio ---
    if (mp3) { mp3->stop(); delete mp3; mp3 = nullptr; }
    if (buff) { delete buff; buff = nullptr; }
    if (file) { delete file; file = nullptr; }

    // --- tworzymy nowe obiekty audio ---
    file = new AudioFileSourceICYStream(url);
    file->useHTTP10();  // Twoja biblioteka wymaga wersji bez argumentu
    file->RegisterMetadataCB(MDCallback, (void*)"ICY");
    buff = new AudioFileSourceBuffer(file, 131072);
    buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
    mp3 = new AudioGeneratorMP3();
    mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
    mp3->begin(buff, out);
    
    Serial.print("Start radia: ");
    Serial.println(url);
    return;


}



void radioStop() {
    if (mp3 && mp3->isRunning()) mp3->stop();
    if (file) file->close();
    Serial.println("Radio STOP");

}

void setupRadio() {
tft.drawRect(19, 95, 405, 70, TFT_CYAN);
String l2 = "WiFi connect";
int a = (480 - tft.textWidth(l2)) / 2;
tft.setCursor(a, 120);
tft.print(l2);

// --- WiFi ---
WiFi.begin(ssid, pass);

int x = 10;

while (WiFi.status() != WL_CONNECTED) {

    delay(200);
}

l2 = "WiFi connected";
a = (480 - tft.textWidth(l2)) / 2;
tft.setCursor(a, 120);
tft.print(l2);
    delay(200);


    // --- Audio I2S ---
    out = new AudioOutputI2S();
    out->SetPinout(1, 2, 3);
    out->SetGain(volume);
loadStationsFromSD();
String url = chooseStation();
radioStart(url.c_str());

}

void audioLoop() {
    static unsigned long lastKey = 0;
    unsigned long now = millis();
key = checkKeyboard() ;
    // --- AUDIO: priorytet 1 ---
    for (int i = 0; i < 5; i++) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();
                break;
            }
        }
    }

    // --- METADANE: priorytet 2 ---
    if (metaReady) {
        metaReady = false;
    metaReady = false;

    static String lastMeta = "";   // <-- JEDYNY static, lokalny i czysty

    String clean = MDchangeLetter(metaRaw);

    // jeśli metadane są takie same → NIE RYSUJEMY
    if (clean == lastMeta) {
        return;
    }

    lastMeta = clean;

        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();   
            }
        }
        char l1[32], l2[32], l3[32];
        MDdivideString(clean.c_str(), l1, l2, l3);
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();   
            }
        }
        MDdrawString(l1, l2, l3);
    }
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();   
            }
        }
    // --- KLAWISZE: priorytet 3 ---
    // ESC → wybór stacji
    if (mcp.digitalRead(rows[0]) == LOW) {
        if (now - lastKey > 150) {
            lastKey = now;

            radioStop();
            tft.fillScreen(TFT_BLACK);

            String url = chooseStation();
            if (url.length() > 0) {
                radioStart(url.c_str());
                drawStatusBar(true);
            }
        }
    }
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();   
            }
        }
    // VOLUME DOWN
    if (mcp.digitalRead(rows[1]) == LOW) {
        if (now - lastKey > 150) {
            lastKey = now;
            volume -= 0.05;
            if (volume < 0.0) volume = 0.0;
            out->SetGain(volume);
        }
    }
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();   
            }
        }
    // VOLUME UP
    if (mcp.digitalRead(rows[2]) == LOW) {
        if (now - lastKey > 150) {
            lastKey = now;
            volume += 0.05;
            if (volume > 1.0) volume = 1.0;
            out->SetGain(volume);
        }
    }
         if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();   
            }
        }
    // NEXT
    if (mcp.digitalRead(rows[5]) == LOW) {
        if (now - lastKey > 150) {
            lastKey = now;
            drawStatusBar(false);
            radioNext();
        }
    }
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();   
            }
        }
    // PREV
    if (mcp.digitalRead(rows[4]) == LOW) {
        if (now - lastKey > 150) {
            lastKey = now;
            drawStatusBar(false);
            radioPrev();
        }
    }

    // DEL → START/STOP
    if (mcp.digitalRead(rows[6]) == LOW) {
        if (now - lastKey > 150) {
            lastKey = now;

            static bool playing = true;

            if (playing) {
                radioStop();
                drawStatusBar(false);
            } else {
                radioStart(url);
                drawStatusBar(true);
            }

            playing = !playing;
        }
    }
}
// -------------------- Metadane -----------------
void MDdrawString(const char* l1, const char* l2, const char* l3) {

    tft.startWrite();
    tft.fillRect(20, 100, 400, 60, TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    int16_t x;

    if (l1[0]) {
        x = (480 - tft.textWidth(l1)) / 2;
        tft.setCursor(x, 100);
        tft.print(l1);
    }

    if (l2[0]) {
        x = (480 - tft.textWidth(l2)) / 2;
        tft.setCursor(x, 120);
        tft.print(l2);
    }

    if (l3[0]) {
        x = (480 - tft.textWidth(l3)) / 2;
        tft.setCursor(x, 140);
        tft.print(l3);
    }

    tft.endWrite();
}

void MDdivideString(const char* src, char* l1, char* l2, char* l3) {

    const int MAX = 25;

    // --- LINIA 1 ---
    int len = strlen(src);
    int cut = (len > MAX ? MAX : len);

    while (cut > 0 && src[cut] != ' ' && cut < len) cut--;
    if (cut == 0) cut = (len > MAX ? MAX : len);

    strncpy(l1, src, cut);
    l1[cut] = 0;

    const char* rest = src + cut;
    if (*rest == ' ') rest++;

    // --- LINIA 2 ---
    len = strlen(rest);
    if (len > 0) {
        cut = (len > MAX ? MAX : len);
        while (cut > 0 && rest[cut] != ' ' && cut < len) cut--;
        if (cut == 0) cut = (len > MAX ? MAX : len);

        strncpy(l2, rest, cut);
        l2[cut] = 0;

        rest = rest + cut;
        if (*rest == ' ') rest++;
    } else {
        l2[0] = 0;
    }

    // --- LINIA 3 ---
    if (strlen(rest) > 0) {
        strncpy(l3, rest, MAX);
        l3[MAX] = 0;
    } else {
        l3[0] = 0;
    }
}

String MDchangeLetter(const char* src) {
    String s(src);

    s.replace("ę","e"); s.replace("ó","o");
    s.replace("ą","a"); s.replace("Ś","S");
    s.replace("ś","s"); s.replace("Ł","L");
    s.replace("ł","l"); s.replace("Ż","Z");
    s.replace("ż","z"); s.replace("Ź","Z");
    s.replace("ź","z"); s.replace("ń","n");
    s.replace("Ć","C"); s.replace("ć","c");
    s.replace("ß","s"); s.replace("ð","o");
    s.replace("Ð","D"); s.replace("æ","ae");
    s.replace("Æ","Ae"); s.replace("ə","e");
    s.replace("Ə","e"); s.replace("µ","mi");
    s.replace("ä","ae"); s.replace("Ä","A");
    s.replace("ö","o"); s.replace("Ö","O");
    s.replace("ü","u"); s.replace("Ü","U");

    return s;
}



void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string) {
    if (!string) return;

    strncpy(metaRaw, string, sizeof(metaRaw) - 1);
    metaRaw[sizeof(metaRaw) - 1] = 0;

    metaReady = true;
}




// -------------------- Status -----------------
void StatusCallback(void *cbData, int code, const char *string) {
    const char *ptr = reinterpret_cast<const char *>(cbData);

    // Wyświetl lost synchronization tylko raz
    if (code == 257 && strstr(string, "lost synchronization") != nullptr) {
        if (!lostSyncPrinted) {
            Serial.printf("STATUS(%s): %d - %s\n", ptr, code, string);
            lostSyncPrinted = true;
        }
        return;
    }

    // Reset flagi przy innych statusach
    if (code != 257) lostSyncPrinted = false;

    char s1[64];
    strncpy_P(s1, string, sizeof(s1));
    s1[sizeof(s1)-1] = 0;

    Serial.printf("STATUS(%s): %d - %s\n", ptr, code, s1);
    Serial.flush();
}

