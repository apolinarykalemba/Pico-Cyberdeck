/* ===============================================================
   PICOMESH – EDYTOR WIADOMOŚCI + SZCZEGÓŁY WĘZŁA (nodeList-only)
   =============================================================== */

// ====== DEKLARACJE ZEWNĘTRZNE, KTÓRE JUŻ MASZ W INNYCH PLIKACH ======
// ==== Deklaracje Funkcji
void uiDrawWiFiMenu();
void uiHandleWiFiMenu(const String &key);

void uiDrawWiFiScan();
void uiHandleWiFiScan(const String &key);

void uiDrawWiFiSelect();
void uiHandleWiFiSelect(const String &key);

void uiDrawWiFiStatus();
void uiHandleWiFiStatus(const String &key);

// Struktura Node i tablica nodeList są w innym pliku – tu tylko deklarujemy:
extern NodeInfo nodeList[];
extern int nodeCount;
extern int sentCount ;
extern int recvCount ;
  int recvIndex;   // Index menu odebranych
  int sentIndex ;  // Index meny wysyłanych
  int viewMessageIndex ; // index ogladanej wiadomośći
int idleCursor = 0;


const char* idleMenu[] = {
  "Adresy",
  "Inbox",
  "Outbox",
  "WiFi",
  "Radio",
  "Snake"
};
 const int idleItems = sizeof(idleMenu) / sizeof(idleMenu[0]);

enum UiState {
  UI_IDLE = 0,
  UI_MENU_ADDR,
  UI_NODE_DETAILS,
  UI_MENU_TYPE,
  UI_EDIT_MESSAGE,
  UI_CONFIRM_SEND,
  UI_INBOX_LIST,
  UI_OUTBOX_LIST,
  UI_VIEW_MESSAGE,
  UI_MENU_WIFI,
  UI_MENU_RADIO,
  UI_WIFI_SCAN,
  UI_WIFI_SELECT,
  UI_WIFI_STATUS,
  UI_WIFI_AUTOCONNECT,
  UI_WIFI_PASSWORD,
  UI_WIFI_CONNECTING,
  UI_SNAKE,

};

UiState uiState = UI_IDLE;

// Kontekst sesji – zostaje bez zmian
struct UiSession {
  int addrIndex;   // indeks wybranego węzła w nodeList
  int typeIndex;   // indeks typu wiadomości

  String dstId;    // ID docelowego węzła (np. "ALL" lub "123")
  String srcID ;    // Adresat
  String msgType;  // "MSG" lub "CMD"
  String text;     // treść wiadomości do wyslania(edytowana)

};

UiSession ui;



// ======================= TYPY WIADOMOŚCI ===================

#define MSG_TYPE_COUNT 2

String msgTypes[MSG_TYPE_COUNT] = {
  "MSG",
  "CMD"
};

// ======================= FILTR ZNAKÓW ===================
bool isForbiddenChar(char c) {
  // Znaki, które rozwalają format protokołu
  return (c == '#' || c == '|' || c == '\'');
}
// =========== Poszukiwanie wezla pi adresie  ==============
int findNodeIndexById(const String &id) {
  for (int i = 0; i < nodeCount; i++) {
    if (nodeList[i].id == id) return i;
  }
  return -1;
}

// ------------------ RADIO MENU  ------------------

void uiDrawRadioMenu() {
    clearScreenBuffer();
    setLine(0, "        RADIO MENU (WIP)");
}
void uiHandleRadio(const String &key) {
  if (key == "") return;




  if (key == "esc") {
    uiState = UI_IDLE;
clearScreenBuffer();    
    uiHandleIdle(key);
    return;
  }


}

//====  MAIN MENU ===============
void uiDrawIdleScreen() {
    clearScreenBuffer();
    drawTopBar();

setLine(0, "             PicoMesh  Ready");
setLine(1, ("             -MAIN MENU-  " + String(NODE_ID)).c_str());

    for (int i = 0; i < idleItems; i++) {
        char buf[40];
        if (i == idleCursor)
            sprintf(buf, "> %s", idleMenu[i]);
        else
            sprintf(buf, "  %s", idleMenu[i]);

        setLine(2 + i, buf);
    }
//setLine(8,  "########################################");
setLine(9,  " ____  _          __  __           _   ");
setLine(10,  "|  _ \\(_) ___ ___|  \\/  | ___ ___ | |_ ");
setLine(11,  "| |_) | |/ __/ _ \\ |\\/| |/ _ / __||  _\\"); 
setLine(12,  "|  __/| | (_( (_) )|  | |  __\\__ \\| || |");
setLine(13,  "|_|   |_|\\___\\___/_|  |_|\\___|___/|_||_|");
//setLine(13, "                                        ");
}
// ------------------ HANDLE NAIN MENU ------------------
void uiHandleIdle(const String &key) {
    if (key == "") return;

    if (key == "up") {
        idleCursor--;
        if (idleCursor < 0) idleCursor = idleItems - 1;
        uiDrawIdleScreen();
        return;
    }

    if (key == "dn") {
        idleCursor++;
        if (idleCursor >= idleItems) idleCursor = 0;
        uiDrawIdleScreen();
        return;
    }

    if (key == "ent") {
        switch (idleCursor) {
            case 0: uiState = UI_MENU_ADDR; uiDrawAddrMenu(); return;
            case 1: uiState = UI_INBOX_LIST; uiDrawInbox(); return;
            case 2: uiState = UI_OUTBOX_LIST; uiDrawOutbox(); return;
            case 3: uiState = UI_MENU_WIFI; uiDrawWiFiMenu(); return;
            case 4: uiState = UI_MENU_RADIO; uiDrawRadioMenu(); return;
            case 5: uiState = UI_SNAKE; uiSnakeStartScreen(); return;
           
        }
    }

    if (key == "esc") {
        uiDrawIdleScreen();
        return;
    }
}

// ============== INBOX  ===============
void uiDrawInbox() {
    static int scroll = 0;
    const int maxVisible = UI_LINES - 2;

    clearScreenBuffer();

    setLine(0, "Lista Odebranych:");
    setLine(1, "ESC-Wyjscie Enter-Szczegoly   ^   v");

    if (recvCount > MSG_HISTORY) recvCount = MSG_HISTORY;

    if (recvCount == 0) {
        setLine(2, "Brak wiadomosci!");
        return;
    }

    if (recvIndex < 0) recvIndex = 0;
    if (recvIndex >= recvCount) recvIndex = recvCount - 1;

    if (recvIndex < scroll)
        scroll = recvIndex;

    if (recvIndex >= scroll + maxVisible)
        scroll = recvIndex - maxVisible + 1;

    if (scroll < 0) scroll = 0;
    if (scroll > recvCount - maxVisible)
        scroll = max(0, recvCount - maxVisible);

    char buf[128];
    int line = 2;

    int start = scroll;
    int end   = min(scroll + maxVisible, recvCount);

    for (int pos = start; pos < end && line < UI_LINES; pos++, line++) {

        int i = pos;  // newest-first
//DEBUG 
Serial.print("INBOX [");
Serial.print(i);
Serial.print("] src=");
Serial.print(recv[i].src);
Serial.print(" dst=");
Serial.print(recv[i].dst);
Serial.print(" timestamp=");
Serial.print(recv[i].timestamp);
Serial.print(" mid=");
Serial.print(recv[i].mid);
Serial.print(" text=");
Serial.println(recv[i].text);
// END DEBUG
        char shortTxt[40];
        strncpy(shortTxt, recv[i].text, 30);
        shortTxt[30] = '\0';
        if (strlen(recv[i].text) > 30) strcat(shortTxt, "...");

        snprintf(buf, sizeof(buf), "%s%u. %s | %s",
                 (i == recvIndex ? "> " : "  "),
                 i + 1,
                 recv[i].src,
                 shortTxt);

        setLine(line, buf);
    }
}
void uiHandleInbox(const String &key) {
  if (key == "") return;

  if (key == "esc") {
    uiState = UI_IDLE;
clearScreenBuffer();    
    uiHandleIdle(key);
    return;
  }

  if (recvCount == 0) return;

  if (key == "up") {
    recvIndex--;
    if (recvIndex < 0) recvIndex = recvCount - 1;
    uiDrawInbox();
    return;
  }

  if (key == "dn") {
    recvIndex++;
    if (recvIndex >= recvCount) recvIndex = 0;
    uiDrawInbox();
    return;
  }

if (key == "ent") {
    viewMessageIndex = recvIndex;
    uiState = UI_VIEW_MESSAGE;
    uiDrawViewMessage();
    return;
}
  uiDrawInbox();
}
// ===============  OUTBOX =====================
void uiDrawOutbox() {
    static int scroll = 0;
    const int maxVisible = UI_LINES - 2;

    clearScreenBuffer();

    setLine(0, "Lista Wyslanych:");
    setLine(1, "ESC-Wyjscie Enter-Szczegoly   ^   v");

    if (sentCount > MSG_HISTORY) sentCount = MSG_HISTORY;

    if (sentCount == 0) {
        setLine(2, "Brak wiadomosci!");
        return;
    }

    // clamp indeksu
    if (sentIndex < 0) sentIndex = 0;
    if (sentIndex >= sentCount) sentIndex = sentCount - 1;

    // scroll dopasowany do newest-first
    if (sentIndex < scroll)
        scroll = sentIndex;

    if (sentIndex >= scroll + maxVisible)
        scroll = sentIndex - maxVisible + 1;

    if (scroll < 0) scroll = 0;
    if (scroll > sentCount - maxVisible)
        scroll = max(0, sentCount - maxVisible);

    char buf[128];
    int line = 2;

    int start = scroll;
    int end   = min(scroll + maxVisible, sentCount);

    for (int pos = start; pos < end && line < UI_LINES; pos++, line++) {

        int i = pos;  // newest-first: sent[0] = najnowsza
// DEBUG
Serial.print("OUTBOX [");
Serial.print(i);
Serial.print("] src=");
Serial.print(sent[i].src);
Serial.print(" dst=");
Serial.print(sent[i].dst);
Serial.print(" timestamp=");
Serial.print(sent[i].timestamp);
Serial.print(" mid=");
Serial.print(sent[i].mid);
Serial.print(" delivered=");
Serial.print(sent[i].delivered);
Serial.print(" text=");
Serial.println(sent[i].text);
// END DEBUG
        char shortTxt[40];
        strncpy(shortTxt, sent[i].text, 30);
        shortTxt[30] = '\0';
        if (strlen(sent[i].text) > 30) strcat(shortTxt, "...");

        char status = sent[i].delivered ? ' ' : 'X';

        snprintf(buf, sizeof(buf), "%s%c %u. %s | %s",
                 (i == sentIndex ? "> " : "  "),
                 status,
                 i + 1,
                 sent[i].dst,
                 shortTxt);

        setLine(line, buf);
    }
}
// ============ Handle OUTBOX =================
void uiHandleOutbox(const String &key) {
  if (key == "") return;

  if (key == "esc") {
    uiState = UI_IDLE;
 clearScreenBuffer();   
    uiHandleIdle(key);
    return;
  }
  
  if (sentCount == 0) return;
 if (key == "up") {
    sentIndex--;
    if (sentIndex < 0) sentIndex = sentCount - 1;
    uiDrawOutbox();
    return;
  }
 if (key == "dn") {
    sentIndex++;
    if (sentIndex >= sentCount) sentIndex = 0;
    uiDrawOutbox();
    return;
  }
 if (key == "ent") {
    // 1. Pobieramy ID odbiorcy wiadomości
    String dstId = sent[sentIndex].dst;
    viewMessageIndex = sentIndex; // << tego brakowało
    // 2. Szukamy węzła o takim ID
    int found = -1;
    for (int i = 0; i < nodeCount; i++) {
      if (nodeList[i].id == dstId) {
        found = i;
        break;
      }
    }

    // 3. Jeśli znaleziono — ustawiamy jako bieżący węzeł
    if (found >= 0) {
      ui.addrIndex = found;
    } else {
      ui.addrIndex = 0; // fallback
    }

    // 4. Przechodzimy do SZCZEGÓŁÓW WĘZŁA
    uiState = UI_NODE_DETAILS;

    // 5. Przekazujemy tekst wysłanej wiadomości
uiDrawNodeDetails(sent[sentIndex].mid);
    return;
  }
  uiDrawOutbox();
}

//======== ADRESS MENU =====================
void uiDrawAddrMenu() { // GOTOWE
clearScreenBuffer() ;
    // belka + guide
    setLine(0, "Wybierz wezel:");
    setLine(1, "ESC-Wyjscie Enter-Szczegoly");

    if (nodeCount == 0) {
        setLine(2, "Brak wezlow w sieci!");
        return;
    }
    // lista węzłów od linii 2 w dół
    for (int i = 0; i < nodeCount && i < (UI_LINES - 2); i++) {
        String line = (i == ui.addrIndex ? "> " : "  ");
        line += nodeList[i].id;          // dokładnie tak jak w Twoim kodzie
        setLine(i + 2, line.c_str());    // +2, bo 0=tytuł, 1=guide
    }
}
// ============= HANDLE ADRESS MENU ====================

void uiHandleMenuAddr(const String &key) {
  
  if (key == "") return;

  if (key == "esc") {
    // ESC – powrót do IDLE
    uiState = UI_IDLE;
   clearScreenBuffer(); 
    uiDrawIdleScreen();
    return;
  }

  if (nodeCount == 0) return;

  if (key == "up") {
    ui.addrIndex--;
    if (ui.addrIndex < 0) ui.addrIndex = nodeCount - 1;
    uiDrawAddrMenu();
    return;
  }

  if (key == "dn") {
    ui.addrIndex++;
    if (ui.addrIndex >= nodeCount) ui.addrIndex = 0;
    uiDrawAddrMenu();
    return;
  }

  if (key == "ent") {
    // przechodzimy do SZCZEGÓŁÓW WĘZŁA.
    uiState = UI_NODE_DETAILS;
 clearScreenBuffer() ; 

    uiDrawNodeDetails(-1);
    return;
  }
   if (key == "lt") {
    // przechodzimy do INBOX.
uiState = UI_INBOX_LIST ;

uiHandleInbox(key);
    return;
  } 
     if (key == "rt") {
    // przechodzimy do OUTBOX.
uiState = UI_OUTBOX_LIST ;

uiHandleOutbox(key);
    return;
  }
}

//================ NODE DETAIL =====================

void uiDrawNodeDetails(int mid) {

    clearScreenBuffer();

    // ---------------------------------------------------------
    //   PODSTAWOWE DANE WĘZŁA
    // ---------------------------------------------------------

    if (nodeCount == 0 || ui.addrIndex < 0 || ui.addrIndex >= nodeCount) {
        setLine(0, "Brak danych wezla!");
        return;
    }

    auto &n = nodeList[ui.addrIndex];
    char buf[64];

    snprintf(buf, sizeof(buf), "Wezel: %s", n.id.c_str());
    setLine(0, buf);

    setLine(1, "ESC-Wyjscie  ENT-Edytuj  > - Ponow");

    snprintf(buf, sizeof(buf), "RSSI: %d", n.rssi);
    setLine(2, buf);

    snprintf(buf, sizeof(buf), "HOPS: %d", n.hops);
    setLine(3, buf);

    // ---------------------------------------------------------
    //   GPS
    // ---------------------------------------------------------

    if (n.lat != 0 && n.lon != 0) {

        double dist = TinyGPSPlus::distanceBetween(
            gps.location.lat(),
            gps.location.lng(),
            n.lat,
            n.lon);

        double az = TinyGPSPlus::courseTo(
            gps.location.lat(),
            gps.location.lng(),
            n.lat,
            n.lon);

        snprintf(buf, sizeof(buf), "Dist: %.1f m", dist);
        setLine(4, buf);

        snprintf(buf, sizeof(buf), "Az: %.1f deg", az);
        setLine(5, buf);

    } else {
        setLine(4, "GPS: brak danych");
        setLine(5, "");
    }
    // ---------------------------------------------------------
    //   STATUS DOSTARCZENIA (szukamy po MID)
    // ---------------------------------------------------------

    int found = -1;

    for (int i = 0; i < sentCount; i++) {
        if (sent[i].mid == mid) {
            found = i;
            break;
        }
    }

    if (found >= 0) {
        if (sent[found].delivered) {
            setLine(6, "Status: DOSTARCZONA");
        } else {
            setLine(6, "Status: NIE DOSTARCZONA");
        }
    } else {
        setLine(6, "Status: odebrana");
    }

    // ---------------------------------------------------------
    //   TREŚĆ WIADOMOŚCI (łamanie na linie)
    // ---------------------------------------------------------

    setLine(7, "MSG:");

    if (found >= 0) {

        const int wrap = 40;
        int lineIndex = 8;

        String txt = sent[found].text;

        while (txt.length() > 0 && lineIndex < UI_LINES) {
            String part = txt.substring(0, wrap);
            txt = txt.substring(min(wrap, txt.length()));
            setLine(lineIndex, part.c_str());
            lineIndex++;
        }
    }

}
//======== HANDLE NODE DETAIL =============

void uiHandleNodeDetails(const String &key) {
  if (key == "") return;

  if (key == "esc") {
    // Powrót do listy węzłów
    uiState = UI_MENU_ADDR;
  clearScreenBuffer();  
    uiDrawAddrMenu();
    return;
  }

  if (key == "ent") {
   clearScreenBuffer() ; 
  
    // Ustawiamy docelowy ID na podstawie wybranego węzła
    ui.dstId = nodeList[ui.addrIndex].id;
    ui.typeIndex = 0;
    uiState = UI_MENU_TYPE;
    uiDrawTypeMenu();
    return;
  }

if (key == "rt") {
    Serial.println("RT");

    Serial.print("viewMessageIndex = ");
    Serial.println(viewMessageIndex);

    Serial.print("sentIndex = ");
    Serial.println(sentIndex);

    Serial.print("tekst = ");
    Serial.println(sent[viewMessageIndex].text);    Packet p;
    p.type = "MSG";  // tutaj wpisz właściwy typ MSG, jeśli nie jest 0
    p.src = NODE_ID;
    p.dst = sent[viewMessageIndex].dst;
    p.mid = random(100, 999);       // NOWA transmisja
    p.ttl = 5;
    p.data = sent[viewMessageIndex].text;
    p.retries = 0;
Serial.println("=== RETRANSMISJA ===");

Serial.print("src: ");
Serial.println(p.src);

Serial.print("dst: ");
Serial.println(p.dst);

Serial.print("mid: ");
Serial.println(p.mid);

Serial.print("ttl: ");
Serial.println(p.ttl);

Serial.print("data: ");
Serial.println(p.data);

Serial.println("====================");
    enqueuePacket(p);
    setLine(UI_LINES - 2, "WYSLANE PONOWNIE !!!");
    return;
}
}
//========== MENU MESSAGE TYPE ==============
void uiDrawTypeMenu() {
clearScreenBuffer() ;
    setLine(0, "Typ wiadomosci:");
    setLine(1, "ESC-Wyjscie  Enter- Message Edition  ");
    for (int i = 0; i < MSG_TYPE_COUNT && i + 1 < UI_LINES; i++) {

        char buf[32];
snprintf(buf, sizeof(buf), "%s%s",
         (i == ui.typeIndex ? "> " : "  "),
         msgTypes[i].c_str());


        setLine(i + 2, buf);
    }
}
// =========== HANDE MESSAGE TYPE =============
void uiHandleMenuType(const String &key) {
  if (key == "") return;

  if (key == "esc") {
    // ESC – powrót do SZCZEGÓŁÓW WĘZŁA (nie do listy)
    uiState = UI_NODE_DETAILS;
clearScreenBuffer();    
    uiDrawNodeDetails(-1);
    return;
  }

  if (key == "up") {
    ui.typeIndex--;
    if (ui.typeIndex < 0) ui.typeIndex = MSG_TYPE_COUNT - 1;
    uiDrawTypeMenu();
    return;
  }

  if (key == "dn") {
    ui.typeIndex++;
    if (ui.typeIndex >= MSG_TYPE_COUNT) ui.typeIndex = 0;
    uiDrawTypeMenu();
    return;
  }

  if (key == "ent") {
  clearScreenBuffer() ; 
   
    ui.msgType = msgTypes[ui.typeIndex];
    ui.text = "";
    uiState = UI_EDIT_MESSAGE;
 clearScreenBuffer();   
    uiDrawEditScreen();
    return;
  }
}
//=============================EDIT MESSAGE =================
void uiDrawEditScreen() {
clearScreenBuffer() ;

    char buf[64];

    // -------------------------
    //  NAGŁÓWEK (stały)
    // -------------------------
    snprintf(buf, sizeof(buf), "Edit Message To: %s (%s) ",
             ui.dstId.c_str(),
             ui.msgType.c_str());
    setLine(0, buf);

    setLine(1, "ESC - wstecz, ENT - dalej Any Key ");
    setLine(2, " Wpisz tekst wiadomosci ");

    // -------------------------
    //  TREŚĆ (dynamiczna)
    // -------------------------
    const int wrap = 28;
    int line = 3;

    int len = ui.text.length();
    const char* src = ui.text.c_str();

    for (int i = 0; i < len && line < UI_LINES; ) {

        int chunk = min(wrap, len - i);

        char tmp[32];
        memcpy(tmp, &src[i], chunk);
        tmp[chunk] = 0;

        setLine(line, tmp);
        line++;
        i += chunk;
    }

    // Jeśli tekst krótszy niż linie — wyczyść resztę
    while (line < UI_LINES) {
        setLine(line, "");
        line++;
    }
}
//============ HANDLE EDIT ===============

void uiHandleEditMessage(const String &key) {

   if (key == "") return;
  if (key == "esc") {
    // ESC – powrót do wyboru typu
    uiState = UI_MENU_TYPE;
  clearScreenBuffer();  
    uiDrawTypeMenu();
    return;
  }

  if (key == "ent") {
    // ENT – przejście do potwierdzenia
    uiState = UI_CONFIRM_SEND;
 clearScreenBuffer();   
    uiDrawConfirmScreen();
    return;
  }

  if (key == "del") {  // backspace
    if (ui.text.length() > 0) {
      ui.text.remove(ui.text.length() - 1);
      uiDrawEditScreen();
    }
    return;
  }

  if (key.length() == 1) {
    char c = key[0];
    if (!isForbiddenChar(c)) {
      ui.text += c;
      uiDrawEditScreen();
    }
    return;
  }
}
// ========= CONFIRM MESSAGE  ===================
void uiDrawConfirmScreen() {
 clearScreenBuffer() ; 
    setLine(0, "Wyslac wiadomosc?");
        setLine(1, ">ENT< Confirm >ESC< Cancel");
    char buf[64];
   snprintf(buf, sizeof(buf), "Do: %s", ui.dstId.c_str());
    setLine(2, buf);
    snprintf(buf, sizeof(buf), "Typ: %s", ui.msgType.c_str());
    setLine(3, buf);
    setLine(4, "Tresc:");
    setLine(5, ui.text.c_str());

}
//======== HANDLE CONFIRM MESSAGE ==============
void uiHandleConfirmSend(const String &key) {
  if (key == "") return;

  if (key == "esc") {
    // ESC – powrót do edycji
    uiState = UI_EDIT_MESSAGE;
   clearScreenBuffer(); 
    uiDrawEditScreen();
    return;
  }

  if (key == "ent") {
    // ENT – faktyczna wysyłka
    Packet p;
    p.type = ui.msgType;
    p.src = NODE_ID;
    p.dst = ui.dstId;
    p.mid = random(100, 999);
    p.ttl = 5;
    p.data = ui.text;
    p.retries = 0;

    enqueuePacket(p);
    if (p.dst != "ALL") {    addPendingRetry(p); }
    uiState = UI_IDLE;
    uiDrawIdleScreen();
    return;
  }
}
// =========== Przegląd otrzymanej ================
void uiDrawViewMessage() {
    clearScreenBuffer();

    if (viewMessageIndex < 0 || viewMessageIndex >= recvCount)
        return;

    setLine(0, "ODEBRANA WIADOMOSC");
    setLine(1, "ESC - Wyjscie  ENT - Odpowiedz");
    char buf[64];

    snprintf(buf, sizeof(buf), "Od: %s", recv[viewMessageIndex].src);
    setLine(2, buf);

    // ---------------------------------------------------------
    //   PODSTAWOWE DANE WĘZŁA
    // ---------------------------------------------------------

    auto &n = nodeList[ui.addrIndex];
    snprintf(buf, sizeof(buf), "RSSI: %d", n.rssi);
    setLine(3, buf);

    snprintf(buf, sizeof(buf), "HOPS: %d", n.hops);
    setLine(4, buf);

    // ---------------------------------------------------------
    //   GPS
    // ---------------------------------------------------------

    if (n.lat != 0 && n.lon != 0) {

        double dist = TinyGPSPlus::distanceBetween(
            gps.location.lat(),
            gps.location.lng(),
            n.lat,
            n.lon);

        double az = TinyGPSPlus::courseTo(
            gps.location.lat(),
            gps.location.lng(),
            n.lat,
            n.lon);

        snprintf(buf, sizeof(buf), "Dist: %.1f m", dist);
        setLine(5, buf);

        snprintf(buf, sizeof(buf), "Az: %.1f deg", az);
        setLine(6, buf);

    } else {
        setLine(5, "GPS: brak danych");
        setLine(6, "");
    }

    setLine(7, "TEXT WIADOMOSCI:");
    String txt = recv[viewMessageIndex].text;

    int line = 8;
    const int wrap = 40;

    while (txt.length() > 0 && line < UI_LINES) {
        String part = txt.substring(0, wrap);
        txt = txt.substring(min(wrap, txt.length()));

        setLine(line, part.c_str());
        line++;
    }
}
//      Obsługa otrzymanej  recv[viewMessageIndex].src)
void uiHandleViewMessage(const String &key) {
    if (key == "") return;
  if (key == "ent") {
   clearScreenBuffer() ; 
  
    // Ustawiamy docelowy ID na podstawie wybranego węzła
    ui.dstId = recv[viewMessageIndex].src;
    ui.typeIndex = 0;
    uiState = UI_MENU_TYPE;
    uiDrawTypeMenu();
    return;
  }

    if (key == "esc") {
        uiState = UI_INBOX_LIST;
        uiDrawInbox();
        return;
    }
}

// ------------------ GŁÓWNA FUNKCJA UI ------------------
void uiHandle() {
  if(key !="") Serial.println(key) ;
    // AutoConnect i Scan muszą działać BEZ klawiszy
    // Snake musi działać BEZ klawiszy
    if (key.length() == 0 &&
        uiState != UI_WIFI_SCAN &&
        uiState != UI_WIFI_AUTOCONNECT &&
        uiState != UI_SNAKE) {
        return;
    }
switch (uiState) {
    case UI_IDLE: uiHandleIdle(key); break;
    case UI_MENU_ADDR: uiHandleMenuAddr(key); break;
    case UI_NODE_DETAILS: uiHandleNodeDetails(key); break;
    case UI_MENU_TYPE: uiHandleMenuType(key); break;
    case UI_EDIT_MESSAGE: uiHandleEditMessage(key); break;
    case UI_CONFIRM_SEND: uiHandleConfirmSend(key); break;

    case UI_INBOX_LIST: uiHandleInbox(key); break;
    case UI_OUTBOX_LIST: uiHandleOutbox(key); break;
case UI_VIEW_MESSAGE: uiHandleViewMessage(key); break;

    // ---- WIFI ----
    case UI_MENU_WIFI: uiHandleWiFiMenu(key); break;
    case UI_WIFI_SCAN: uiHandleWiFiScan(key); break;
    case UI_WIFI_SELECT: uiHandleWiFiSelect(key); break;
    case UI_WIFI_PASSWORD: uiHandleWiFiPassword(key); break;
    case UI_WIFI_CONNECTING: uiHandleWiFiConnecting(key); break;   // 
    case UI_WIFI_STATUS: uiHandleWiFiStatus(key); break;
    case UI_WIFI_AUTOCONNECT: uiHandleWiFiAutoConnect(key); break; // 
    // ---- GAME ---
    case UI_MENU_RADIO:    uiHandleRadio(key);    break;   
    case UI_SNAKE:    uiHandleSnake(key);    break;
  }
}
