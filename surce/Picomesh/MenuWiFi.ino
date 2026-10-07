
void StartWiFi(String ssid, String pass) {
    clearScreenBuffer();
    drawTopBar();

    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    setLine(0, "         WiFi CONNECT");
    setLine(2, "  Laczenie z siecia...");
    
    char buf[40];
    sprintf(buf, "  SSID: %s", ssid.c_str());
    setLine(3, buf);

    sprintf(buf, "  Proba polaczenia...");
    setLine(5, buf);

    CheckRedraw();   // ← jeśli masz buforowanie

    Serial.println("\n[WiFi] Connecting...");
    Serial.print("[WiFi] SSID=");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(200);

    WiFi.begin(ssid.c_str(), pass.c_str());

    int retry = 0;
    const int maxRetry = 10;

    // ---- POSTĘP NA TFT ----
    String dots = "";

    while (WiFi.status() != WL_CONNECTED && retry < maxRetry) {
        delay(1000);

        dots += ".";
        if (dots.length() > 10) dots = ".";

        char buf2[40];
        sprintf(buf2, "  Czekaj%s", dots.c_str());
        setLine(6, buf2);

        CheckRedraw();

        Serial.print(".");
        retry++;
    }

    // ---- WYNIK ----
    if (WiFi.status() == WL_CONNECTED) {
        setLine(8, "  Polaczono!");
        CheckRedraw();

        Serial.println("\n[WiFi] CONNECTED!");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
    } else {
        setLine(8, "  BLAD polaczenia!");
        CheckRedraw();

        Serial.println("\n[WiFi] FAILED!");
    }
}


// =====================================================
//                 ZMIENNE GLOBALNE WiFi
// =====================================================

int wifiCursor = 0;
int wifiSelectCursor = 0;

String wifiPassword = "";
unsigned long wifiConnectStart = 0;

int wifiScanCount = 0;
String selectedSSID;

// AutoConnect
bool wifiAutoConnectStarted = false;
unsigned long wifiAutoConnectT0 = 0;


// =====================================================
//                 MENU WiFi (lista opcji)
// =====================================================

const char* wifiMenu[] = {
  "AutoConnect",
  "Skanuj sieci",
  "Wybierz siec",
  "Status WiFi",
  "Rozlacz WiFi"   // ← NOWE
};


const int wifiItems = sizeof(wifiMenu) / sizeof(wifiMenu[0]);


// =====================================================
//                 RYSOWANIE MENU WiFi
// =====================================================

void uiDrawWiFiMenu() {
    clearScreenBuffer();
    drawTopBar();

    setLine(0, "               WiFi MENU");
    setLine(1, "----------------------------------------");

    for (int i = 0; i < wifiItems; i++) {
        char buf[40];
        if (i == wifiCursor)
            sprintf(buf, "> %s", wifiMenu[i]);
        else
            sprintf(buf, "  %s", wifiMenu[i]);

        setLine(2 + i, buf);
    }

    setLine(8, "ESC - powrot");
}
void wifiDisconnect() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("[WiFi] Disconnecting...");
    Serial.println("[WiFi] Disconnecting...");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(300);

    tft.println("WiFi rozlaczone.");
    Serial.println("[WiFi] DISCONNECTED");
//set_sys_clock_khz(25000, true); // 25 MHz
}


// =====================================================
//                 OBSŁUGA MENU WiFi
// =====================================================

void uiHandleWiFiMenu(const String &key) {
    if (key == "") return;

    if (key == "up") {
        wifiCursor--;
        if (wifiCursor < 0) wifiCursor = wifiItems - 1;
        uiDrawWiFiMenu();
        return;
    }

    if (key == "dn") {
        wifiCursor++;
        if (wifiCursor >= wifiItems) wifiCursor = 0;
        uiDrawWiFiMenu();
        return;
    }

    if (key == "ent") {
        switch (wifiCursor) {

            case 0: // AutoConnect
                uiState = UI_WIFI_AUTOCONNECT;
                set_sys_clock_khz(200000, true); // 100 MHz  
                uiDrawWiFiAutoConnect();
                return;

            case 1: // Skanowanie
                uiState = UI_WIFI_SCAN;
                           set_sys_clock_khz(200000, true); // 100 MHz       
                uiDrawWiFiScan();
                return;

            case 2: // Wybór sieci
                uiState = UI_WIFI_SELECT;
                          set_sys_clock_khz(200000, true); // 100 MHz        
                uiDrawWiFiSelect();
                return;

            case 3: // Status
                uiState = UI_WIFI_STATUS;
                uiDrawWiFiStatus();
                return;
case 4: // Rozlacz WiFi
    wifiDisconnect();
    uiState = UI_WIFI_STATUS;
    uiDrawWiFiStatus();
    return;

             
        }
    }

    if (key == "esc") {
        uiState = UI_IDLE;
        uiDrawIdleScreen();
        return;
    }
}


// =====================================================
//                 AUTOCONNECT
// =====================================================


void uiDrawWiFiAutoConnect() {
    clearScreenBuffer();
    drawTopBar();

    setLine(0, "         WiFi AutoConnect");
    setLine(2, "  Laczenie z zapisanym SSID...");
    setLine(8, "ESC - powrot");

    wifiAutoConnectStarted = false;   // ← KLUCZOWE
}

void uiHandleWiFiAutoConnect(const String &key) {
    if (key == "esc") {
        uiState = UI_MENU_WIFI;
        uiDrawWiFiMenu();
        return;
    }

    String ssid, pass;
    if (!loadWiFiCredentials(ssid, pass)) {
        setLine(4, " Brak zapisanych danych!");
        return;
    }

    selectedSSID = ssid;
    wifiPassword = pass;

    StartWiFi(ssid, pass);

    uiState = UI_WIFI_STATUS;
    uiDrawWiFiStatus();
}




// =====================================================
//                 SKANOWANIE SIECI
// =====================================================

void uiDrawWiFiScan() {
    clearScreenBuffer();
    drawTopBar();

    setLine(0, "            WiFi SCAN");
    setLine(2, "  Skanowanie sieci...");
    setLine(8, "ESC - powrot");

    wifiScanCount = WiFi.scanNetworks();
}

void uiHandleWiFiScan(const String &key) {
    if (key == "esc") {
        uiState = UI_MENU_WIFI;
        uiDrawWiFiMenu();

        return;
    }

    // Po zakończeniu skanu — automatycznie przechodzimy dalej
    uiState = UI_WIFI_SELECT;
    uiDrawWiFiSelect();
}


// =====================================================
//                 WYBÓR SIECI
// =====================================================

void uiDrawWiFiSelect() {
    clearScreenBuffer();
    drawTopBar();

    setLine(0, "          Wybierz siec");
    setLine(1, "----------------------------------------");

    if (wifiScanCount <= 0) {
        setLine(3, " Brak sieci!");
        setLine(8, "ESC - powrot");
        return;
    }

    for (int i = 0; i < wifiScanCount && i < 6; i++) {
        char buf[40];
        const char* ssid = WiFi.SSID(i);

        if (i == wifiSelectCursor)
            sprintf(buf, "> %s (%d dBm)", ssid, WiFi.RSSI(i));
        else
            sprintf(buf, " %s (%d dBm)", ssid, WiFi.RSSI(i));

        setLine(2 + i, buf);
    }

    setLine(9, "ESC - powrot");
}

void uiHandleWiFiSelect(const String &key) {
    if (key == "" &&
    uiState != UI_WIFI_SCAN &&
    uiState != UI_WIFI_AUTOCONNECT &&
    uiState != UI_SNAKE) return;

    if (key == "up") {
        wifiSelectCursor--;
        if (wifiSelectCursor < 0) wifiSelectCursor = wifiScanCount - 1;
        uiDrawWiFiSelect();
        return;
    }

    if (key == "dn") {
        wifiSelectCursor++;
        if (wifiSelectCursor >= wifiScanCount) wifiSelectCursor = 0;
        uiDrawWiFiSelect();
        return;
    }

    if (key == "ent") {
        selectedSSID = String(WiFi.SSID(wifiSelectCursor));
        selectedSSID.trim();

        wifiPassword = "";
        uiState = UI_WIFI_PASSWORD;
        uiDrawWiFiPassword();
        return;
    }

    if (key == "esc") {
        uiState = UI_MENU_WIFI;
        uiDrawWiFiMenu();
        return;
    }
}


// =====================================================
//                 WPROWADZANIE HASŁA
// =====================================================

void uiDrawWiFiPassword() {
    clearScreenBuffer();
    drawTopBar();

    setLine(0, "        Wprowadz haslo");
    setLine(1, "----------------------------------------");

    char buf[40];
    sprintf(buf, " SSID: %s", selectedSSID.c_str());
    setLine(2, buf);

    sprintf(buf, " Haslo: %s", wifiPassword.c_str());
    setLine(4, buf);

    setLine(6, " ent - zatwierdz");
    setLine(7, " del - usun znak");
    setLine(8, " esc - powrot");
}

void uiHandleWiFiPassword(const String &key) {
    if (key == "") return;

if (key == "ent") {
    StartWiFi(selectedSSID, wifiPassword);
    uiState = UI_WIFI_STATUS;
 saveWiFiCredentials(selectedSSID, wifiPassword);
   
    uiDrawWiFiStatus();
    return;
}


    if (key == "esc") {
        uiState = UI_WIFI_SELECT;
        uiDrawWiFiSelect();
        return;
    }

    if (key == "del") {
        if (wifiPassword.length() > 0)
            wifiPassword.remove(wifiPassword.length() - 1);
        uiDrawWiFiPassword();
        return;
    }

    if (key == "up" || key == "dn" || key == "lt" || key == "rt" || key == "alt") {
        return;
    }

    if (key.length() == 1) {
        wifiPassword += key;
        uiDrawWiFiPassword();
        return;
    }
}


// =====================================================
//                 ŁĄCZENIE Z SIECIĄ
// =====================================================

void uiDrawWiFiConnecting() {
    clearScreenBuffer();
    drawTopBar();

    setLine(0, "          Laczenie WiFi");

    char buf[40];
    sprintf(buf, " SSID: %s", selectedSSID.c_str());
    setLine(2, buf);

    setLine(4, " Haslo: ********");
    setLine(7, " Czekaj...");

    wifiConnectStart = millis();

    WiFi.begin(selectedSSID.c_str(), wifiPassword.c_str());
}

void uiHandleWiFiConnecting(const String &key) {
    if (millis() - wifiConnectStart > 8000) {
        if (WiFi.status() == WL_CONNECTED) {
            saveWiFiCredentials(selectedSSID, wifiPassword);
            uiState = UI_WIFI_STATUS;
            uiDrawWiFiStatus();
        } else {
            setLine(9, " BLAD POLACZENIA!");
        }
    }
}


// =====================================================
//                 STATUS WiFi
// =====================================================

void uiDrawWiFiStatus() {
    clearScreenBuffer();
    drawTopBar();

    setLine(0, "           WiFi STATUS");
    setLine(1, "----------------------------------------");

    if (WiFi.status() != WL_CONNECTED) {
        setLine(3, " Brak polaczenia.");
        setLine(8, "ESC - powrot");
        return;
    }

    char buf[40];

    sprintf(buf, " SSID: %s", selectedSSID.c_str());
    setLine(3, buf);

    sprintf(buf, " IP: %s", WiFi.localIP().toString().c_str());
    setLine(4, buf);

    sprintf(buf, " RSSI: %d dBm", WiFi.RSSI());
    setLine(5, buf);

    setLine(8, "ESC - powrot");
}

void uiHandleWiFiStatus(const String &key) {
    if (key == "esc") {
        uiState = UI_MENU_WIFI;
        uiDrawWiFiMenu();
    }
}


// =====================================================
//                 ZAPIS / ODCZYT WiFi (LittleFS)
// =====================================================

bool saveWiFiCredentials(String ssid, String pass) {
    File f = LittleFS.open("/wifi.txt", "w");
    if (!f) {
        Serial.println("[FS] ERROR: cannot open wifi.txt for write");
        return false;
    }

    f.println("SSID=" + ssid);
    f.println("PASS=" + pass);
    f.close();
Serial.print("[FS] SAVE SSID=[");
Serial.print(ssid);
Serial.println("]");
    Serial.println("[FS] WiFi credentials saved");
    return true;
}


bool loadWiFiCredentials(String &ssid, String &pass) {
    File f = LittleFS.open("/wifi.txt", "r");
    if (!f) {
        Serial.println("[FS] wifi.txt not found");
        return false;
    }

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();

        if (line.startsWith("SSID=")) ssid = line.substring(5);
        if (line.startsWith("PASS=")) pass = line.substring(5);
    }

    f.close();

    Serial.print("[FS] Loaded SSID: ");
    Serial.println(ssid);

    return (ssid.length() > 0);
}

