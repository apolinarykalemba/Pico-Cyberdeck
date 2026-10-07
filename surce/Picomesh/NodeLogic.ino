// ======================= ODBIÓR I PRZETWARZANIE PAKIETU =======================
void processPacket(String pkt) {

  // Usuń CR/LF i spacje z początku oraz końca pakietu
  pkt.trim();

  // Ignoruj pusty pakiet
  if (pkt.length() == 0) return;

  // DEBUG - wyświetlenie surowego pakietu
  // printTerminalText(pkt);

  // --------------------------------------------------------------------------
  // Kopia String do klasycznego bufora C
  // Dzięki temu możemy szybko wyszukiwać separatory bez tworzenia wielu Stringów
  // --------------------------------------------------------------------------
  char buf[256];

  int len = pkt.length();

  // Zabezpieczenie przed przepełnieniem bufora
  if (len >= (int)sizeof(buf))
    len = sizeof(buf) - 1;

  // Kopiuj String do tablicy znaków
  pkt.toCharArray(buf, len + 1);

  // --------------------------------------------------------------------------
  // Znajdź pozycje separatorów '|'
  // Format pakietu:
  // TYPE|SRC|DST|MID|TTL|DATA
  // --------------------------------------------------------------------------
  int idx[5];
  int sepCount = 0;

  for (int i = 0; i < len && sepCount < 5; i++) {
    if (buf[i] == '|') {
      idx[sepCount++] = i;
    }
  }

  // Jeżeli nie znaleziono wszystkich 5 separatorów,
  // pakiet jest uszkodzony lub niekompletny
  if (sepCount < 5) {
    if (DEBUG)
      Serial.println("Malformed packet - ignoring");
    return;
  }

  // --------------------------------------------------------------------------
  // Funkcja pomocnicza wycinająca fragment tekstu pomiędzy separatorami
  // --------------------------------------------------------------------------
  auto makeString = [&](int start, int end) -> String {

    // Kontrola poprawności zakresów
    if (start < 0 || end < start || end > len)
      return String("");

    int l = end - start;

    return String(buf + start).substring(0, l);
  };

  // --------------------------------------------------------------------------
  // Rozbicie pakietu na pola
  // --------------------------------------------------------------------------
  String type   = makeString(0, idx[0]);            // typ pakietu
  String src    = makeString(idx[0] + 1, idx[1]);  // źródło
  String dst    = makeString(idx[1] + 1, idx[2]);  // adres docelowy
  String midStr = makeString(idx[2] + 1, idx[3]);  // Message ID
  String ttlStr = makeString(idx[3] + 1, idx[4]);  // Time To Live
  String data   = makeString(idx[4] + 1, len);     // payload


  // DEBUG - podgląd wszystkich pól pakietu

  if (DEBUG) {
    Serial.print("TYPE: |"); Serial.print(type);
    Serial.print("| len=");  Serial.println(type.length());

    Serial.print("SRC : |"); Serial.print(src);
    Serial.print("| len=");  Serial.println(src.length());

    Serial.print("DST : |"); Serial.print(dst);
    Serial.print("| len=");  Serial.println(dst.length());

    Serial.print("MID : |"); Serial.print(midStr);
    Serial.print("| len=");  Serial.println(midStr.length());

    Serial.print("TTL : |"); Serial.print(ttlStr);
    Serial.print("| len=");  Serial.println(ttlStr.length());

    Serial.print("DATA: |"); Serial.print(data);
    Serial.print("| len=");  Serial.println(data.length());
  }


  // Zamiana tekstowych wartości MID i TTL na liczby
  int mid = midStr.toInt();
  int ttl = ttlStr.toInt();

  // ========================================================================
  // OCHRONA PRZED DUPLIKATAMI
  // ========================================================================

  // Pakiet już wcześniej przetworzony?
  if (inCache(src, mid)) {

    if (DEBUG)
      Serial.println("⛔ DUPLIKAT — pomijam");

    return;
  }
  else {

    // Dodaj pakiet do cache
    addCache(src, mid);

    // Szacowana liczba przeskoków
    // Zakładamy że pakiet startował z TTL=10
    int hops = 5 - ttl;

    // Aktualizacja informacji o źródłowym węźle
    updateNode(
      src,
      hops,
      0.0,
      0.0,
      0.0,
      0.0,
      radio.getRSSI()
    );
  }

  // ========================================================================
  // BEACON
  // ========================================================================
  if (type == "BEA") {

    // Funkcja pobierająca pojedynczą wartość z payloadu
    // np. BAT=4.1;LAT=52.1;LON=21.0;
    auto parseValue = [&](const char* key) -> double {

      int keyLen = strlen(key);

      int start = data.indexOf(key);

      // Brak pola
      if (start < 0)
        return 0.0;

      start += keyLen;

      int end = data.indexOf(';', start);

      if (end == -1)
        end = data.length();

      String valStr = data.substring(start, end);

      return strtod(valStr.c_str(), NULL);
    };

    // Odczyt wartości z payloadu BEA
    float  bat = parseValue("BAT=");
    double lat = parseValue("LAT=");
    double lon = parseValue("LON=");
    float  alt = parseValue("ALT=");

    int hops = 10 - ttl;

    if (DEBUG) {

      Serial.print("Lat: ");
      Serial.println(lat);

      Serial.print("Lon: ");
      Serial.println(lon);

      Serial.print("Alt: ");
      Serial.print(alt);
      Serial.println(" m");
    }

    // Aktualizacja informacji o węźle
    updateNode(
      src,
      hops,
      lat,
      lon,
      alt,
      bat,
      radio.getRSSI()
    );
  }

  // ========================================================================
  // FORWARDING
  // Pakiet nie jest adresowany do mnie
  // ========================================================================
  if (dst != NODE_ID && ttl > 0) {

    // Zmniejsz TTL przed retransmisją
    ttl--;

    Serial.println("FORWARDING");

    forwardPacket({
      type,
      src,
      dst,
      mid,
      ttl,
      data,
      0
    });
    if (dst != "ALL") {
        return;
    }
  }

  // ========================================================================
  // PAKIET DO MNIE LUB BROADCAST
  // ========================================================================
  if (dst == NODE_ID || dst == "ALL") {
      // Włącz podświetlenie ekranu
 analogWrite(TFT_BL, brightness);  // SCREEN ON  PROCESOIR FULL SPEED
             lastTFToff = millis();  
  set_sys_clock_khz(150000, true); // 150 MHz                 
            
    updateMessageCount() ;  
          
    // ======================================================================
    // WIADOMOŚĆ PRYWATNA
    // ======================================================================
    if (type == "MSG" && dst == NODE_ID) {

   

      if (DEBUG)
        Serial.println("MSG do mnie: " + data);

      // Dodaj wiadomość do historii
      pushRecvMessage(
        src.c_str(),
        NODE_ID,
        data.c_str()
      );

      // ------------------------------------------------------
      // Przygotuj ACK
      // ------------------------------------------------------
      Packet ack;

      ack.type = "ACK";
      ack.src = NODE_ID;
      ack.dst = src;

      ack.mid = random(100, 999);

      ack.ttl = 10;

      // ACK zawiera MID potwierdzanego pakietu
      ack.data = String(mid);

      ack.retries = 0;

      // Dodaj ACK do cache
      addCache(ack.src, ack.mid);
      if (DEBUG)
        Serial.println("Wysyłam Potwierdzenie");
delay(random(20 , 100)) ;
      SendFast(ack);
   uiState = UI_INBOX_LIST; uiDrawInbox();  // Wyswietlanie menu INBOX 
    }
    // ======================================================================
    // WIADOMOŚĆ BROADCAST
    // ======================================================================
    if (type == "MSG" && dst == "ALL") {
      // Włącz podświetlenie ekranu
 analogWrite(TFT_BL, brightness);  // SCREEN ON  PROCESOIR FULL SPEED
             lastTFToff = millis();  
  set_sys_clock_khz(150000, true); // 150 MHz     
      if (DEBUG) {
        Serial.print("Pakiet do Wszystkich - ");
        Serial.println(data);
      }
      // Dodaj wiadomość do historii
      pushRecvMessage(
        src.c_str(),
        NODE_ID,
        data.c_str()
      );
      // Reset licznika wygaszania ekranu
      lastTFToff = millis();
      uiState = UI_INBOX_LIST; uiDrawInbox(); // Wyswietlanie menu INBOX   
    }

    // ======================================================================
    // ACK
    // ======================================================================
    if (type == "ACK") {

      // MID potwierdzanego pakietu
      int ackMID = data.toInt();

      // Oznacz wiadomość jako dostarczoną
      markDelivered(ackMID);

      // Usuń z kolejki wiadomości oczekujących
      removeMSGbyMID(ackMID);

      // Usuń z kolejki retransmisji
      removePendingByMID(ackMID);

      // DEBUG - pokaż historię
      debugPrintHistory();
    }

    // ======================================================================
    // KOMENDA
    // ======================================================================
    if (type == "CMD") {

      // ------------------------------------------------------
      // ACK dla CMD TYLKO JEZELI JESTEM ODBIORCĄ
      // ------------------------------------------------------
      if (src != "ALL"){
      Packet ack;

      ack.type = "ACK";
      ack.src = NODE_ID;
      ack.dst = src;

      ack.mid = random(100, 999);

      ack.ttl = 10;

      // Potwierdzamy MID otrzymanej komendy
      ack.data = String(mid);

      ack.retries = 0;

      addCache(ack.src, ack.mid);

      SendFast(ack);
      }
      // ------------------------------------------------------
      // Obsługa komend
      // ------------------------------------------------------

      if (data == "BEA")
        sendBEA(src);

      if (data == "RSS") {
        // przyszła obsługa RSS
      }
      if (data == "LEDON") {
        mcp.digitalWrite(SHIFT_LED , HIGH ) ; // Włacza LED
      }
        if (data == "LEDOFF") {
        mcp.digitalWrite(SHIFT_LED , LOW);  // Wyłacza LED
      }    
    }
  }
}


// ================== ANTYDUPLIKAT =========================
void addCache(String src, int mid) {
  if (inCache(src, mid)) return;  // jeśli już w cache -> nic nie rób

  if (cacheCount < MAX_CACHE) {
    packetCache[cacheCount++] = { src, mid };  // dopisz na koniec jeśli miejsce
  } else {
    // jeśli brak miejsca -> FIFO: usuń najstarszy, przesuwając elementy
    for (int i = 1; i < MAX_CACHE; i++) {
      packetCache[i - 1] = packetCache[i];
    }
    packetCache[MAX_CACHE - 1] = { src, mid };
  }
}
//## sprawdź czy dany (src,mid) jest już w cache
bool inCache(String src, int mid) {
  for (int i = 0; i < cacheCount; i++) {
    if (packetCache[i].src == src && packetCache[i].mid == mid) return true;
  }
  return false;
}
// ========END ANTYDUPLIKAT =============================
// ============== AKTUALIZACJA NODE LIST =====================
void updateNode(String id, int hops, double lat, double lon, float alt, float bat, int rssi)
 {
  int i = findNode(id);  // sprawdź czy węzeł już istnieje

  if (i >= 0) {                       // jeżeli istnieje - uaktualnij pola
    nodeList[i].lastSeen = millis();  // aktualny czas "lastSeen"
    nodeList[i].hops = hops;
    nodeList[i].rssi = rssi;
    // jeśli otrzymaliśmy sensowne współrzędne (nie 0.0) - zaktualizuj
    if (lat != 0.0 && lon != 0.0) {
      nodeList[i].lat = lat;
      nodeList[i].lon = lon;
      nodeList[i].alt = alt;
    }
    // jeśli bateria > 0.1V (sensowna wartość) - uaktualnij
    if (bat > 0.1) nodeList[i].battery = bat;
    return;
  }

  // jeśli nie istnieje i jest miejsce - dodaj nowy wpis
  if (nodeCount < MAX_NODES) {
     playTone(3000, 30, 0.4f);  
    nodeList[nodeCount++] = { id, millis(), hops, lat, lon, alt, bat, rssi };
  } else {
    // jeśli nie ma miejsca -> FIFO: usuń najstarszy (przesuń w lewo), dodaj na koniec
    for (int j = 1; j < MAX_NODES; j++) nodeList[j - 1] = nodeList[j];
    nodeList[MAX_NODES - 1] = { id, millis(), hops, lat, lon, alt, bat, rssi };
  }
}
// -------- pomocnicze - znajdź węzeł po ID ---------
int findNode(String id) {
  for (int i = 0; i < nodeCount; i++) {
    if (nodeList[i].id == id) return i;  // jeśli znajdzie -> zwróć indeks
  }
  return -1;  // nie znaleziono
}
// ================== END NODE LIST ===============================
// ======================= Forward pakietu ===================
// Funkcja używana do forwardowania pakietów (zmniejsza TTL i enqueues)
void forwardPacket(Packet p) {
  if (p.ttl > 0) {
    // jeśli TTL nadal > 0 -> spróbuj dodać do kolejki aby forwardować dalej
    if (!enqueuePacket(p)) {
  if (DEBUG)     Serial.println("Kolejka pełna — forward odrzucony");
    }
  }
}
// ======================= Kolejka pakietów (circular buffer) ==================
// enqueue: dodaj pakiet do kolejki; zwraca true jeśli się udało
bool enqueuePacket(Packet p) {
  int next = (queueEnd + 1) % MAX_QUEUE;  // oblicz indeks następnego wolnego
  if (next == queueStart) return false;   // kolejka pełna (warunek przepełnienia)
    // Dodaj TYLKO własne pakiety do cache
  if (p.src == NODE_ID) {
      addCache(p.src, p.mid);
  }
  sendQueue[queueEnd] = p;                // zapisz pakiet w tablicy
  queueEnd = next;                        // przesuń "koniec" kolejki
  return true;
}
//#################### dequeue: pobierz pakiet z kolejki; zwraca false jeśli pusta
bool dequeuePacket(Packet &p) {
  if (queueStart == queueEnd) return false;   // pusta kolejka
  p = sendQueue[queueStart];                  // pobierz pierwszy element
  queueStart = (queueStart + 1) % MAX_QUEUE;  // przesuń początek
  return true;
}
// rozmiar kolejki: policz aktualną liczbę elementów
int queueSize() {
  if (queueEnd >= queueStart) return queueEnd - queueStart;
  return MAX_QUEUE - queueStart + queueEnd;
}
// ############################Usuń MSG z kolejki po odebraniu ACK (usuwa pierwsze wystąpienie)
void removeMSGbyMID(int mid) {
  int i = queueStart;
  while (i != queueEnd) {  // przejdź przez wszystkie elementy w kolejce
    if (sendQueue[i].type == "MSG" && sendQueue[i].mid == mid) {
      // przesuwamy elementy w lewo, nadpisując znaleziony
      int j = i;
      while (j != (queueEnd - 1 + MAX_QUEUE) % MAX_QUEUE) {
        int nextIdx = (j + 1) % MAX_QUEUE;
        sendQueue[j] = sendQueue[nextIdx];
        j = nextIdx;
      }
      // zmniejszamy queueEnd (cofa indeks wolnego miejsca)
      queueEnd = (queueEnd - 1 + MAX_QUEUE) % MAX_QUEUE;
      // usunęliśmy jedno wystąpienie - przerwij pętlę
      break;
    }
    i = (i + 1) % MAX_QUEUE;  // idź do następnego indeksu circular
  }
}
//############################ Send packet from Queue
void sendFromQueue() {
    if (queueEmpty()) return;
    static uint32_t nextSend = 0;
    if (millis() < nextSend) return;   // jeszcze nie czas

    Packet p;
    if (!dequeuePacket(p)) return;


    // 1) ZAPISZ DO HISTORII TX
if (p.type == "MSG" && p.src == NODE_ID) {
    pushSentMessage(p);
}

    // dopisujemy do cache, żeby nie forwardować własnego pakietu
    addCache(p.src, p.mid);  // dopisz do rejestru wysyłanych/odebranych

    // budujemy string i wysyłamy
    String pkt = p.type + "|" + p.src + "|" + p.dst + "|" + String(p.mid) + "|" + String(p.ttl) + "|" + p.data;
radioSend(p) ;
 //       playTone(500, 30, 0.4f);
    // ustaw czas kolejnej
    nextSend = millis() + 500 + random(-100, 100);  // jitter 0–200 ms
}
//############## Check for empty QUEUE 
bool queueEmpty() {
  return queueStart == queueEnd;  // prawda jeśli pusta
}
// ============= END QUEUE ===========
// ======= Kolejka wysłanych
void pushSentMessage(const Packet &p) {

    // przesuwamy tablicę w dół (starsze lecą w dół)
    for (int i = MSG_HISTORY - 1; i > 0; i--) {
        sent[i] = sent[i - 1];
    }

    // nowa wiadomość na górze
    strncpy(sent[0].src, p.src.c_str(), sizeof(sent[0].src));
    strncpy(sent[0].dst, p.dst.c_str(), sizeof(sent[0].dst));
    strncpy(sent[0].text, p.data.c_str(), sizeof(sent[0].text));

    sent[0].timestamp = millis();
    sent[0].delivered = false;
    sent[0].mid = p.mid;

    if (sentCount < MSG_HISTORY) sentCount++;
}
// ====== PENDIND RETRIES =========================
// ======================= Pending retries (inicjalizacja i operacje) ===================
void initPending() {
  for (int i = 0; i < MAX_PENDING; i++) pendingRetries[i].used = false;  // wszystkie sloty wolne
}

// znajdź indeks pending po MID (zwraca -1 jeśli brak)
int findPendingByMID(int mid) {
  for (int i = 0; i < MAX_PENDING; i++) {
    if (pendingRetries[i].used && pendingRetries[i].pkt.mid == mid) return i;
  }
  return -1;
}

// dodaj pakiet MSG do pendingRetries (będziemy nim zarządzać dopóki nie dostaniemy ACK)
void addPendingRetry(Packet p) {
  int freeIdx = -1;
  for (int i = 0; i < MAX_PENDING; i++) {
    if (!pendingRetries[i].used) {
      freeIdx = i;
      break;
    }  // znajdź wolny slot
  }
  if (freeIdx < 0) {
 if (DEBUG)    Serial.println("No free pending slots for retry!");  // brak miejsca
    return;
  }
  pendingRetries[freeIdx].used = true;
  pendingRetries[freeIdx].pkt = p;                                  // kopiujemy pakiet
  pendingRetries[freeIdx].retries = 0;                              // zero prób dotychczas
  pendingRetries[freeIdx].nextRetryAt = millis() + RETRY_INTERVAL;  // harmonogram następnej próby
}

// usuń pending po MID (ACK otrzymany)
void removePendingByMID(int mid) {
  int idx = findPendingByMID(mid);
  if (idx >= 0) pendingRetries[idx].used = false;  // zwolnij slot
}

// sprawdź pendingRetries, jeżeli czas -> spróbuj ponownie (enqueue)
void checkPendingRetries() {
  unsigned long now = millis();

  for (int i = 0; i < MAX_PENDING; i++) {

    if (!pendingRetries[i].used) continue;  // slot wolny -> pomiń

    // osiągnięto maksymalną liczbę prób -> porzuć pakiet
    if (pendingRetries[i].retries >= MAX_MSG_RETRIES) {
      if (DEBUG) Serial.print("MSG MID=");
      if (DEBUG) Serial.print(pendingRetries[i].pkt.mid);
      if (DEBUG) Serial.println(" reached max retries -> dropping");

      pendingRetries[i].used = false;
      continue;
    }

    // czy nadszedł czas retry?
    if ((long)(now - pendingRetries[i].nextRetryAt) >= 0) {

      // ================================
      // 1. SKOPIUJEMY pakiet z pending
      // ================================
      Packet rp = pendingRetries[i].pkt;

      // ================================
      // 2. NADAJEMY NOWY MID (unikalny)
      // ================================
      rp.mid = random(100, 999);

      // ================================
      // 3. AKTUALIZUJEMY pending, aby ACK mogło znaleźć po MID
      // ================================
      pendingRetries[i].pkt.mid = rp.mid;

      // ================================
      // 4. DODAJEMY DO CACHE, aby retry nie wrócił jako duplikat
      // ================================
      addCache(rp.src, rp.mid);

      // ================================
      // 5. PRÓBUJEMY WRZUCIĆ DO QUEUE
      // ================================
      if (enqueuePacket(rp)) {

        pendingRetries[i].retries++;  // zliczamy retry

        // ustawiamy następny czas retry z lekkim backoffem
        pendingRetries[i].nextRetryAt =
            now + RETRY_INTERVAL + random(0, 200);

        if (DEBUG) {
          Serial.print("Re-enqueued MSG MID=");
          Serial.print(rp.mid);
          Serial.print(" retries=");
          Serial.println(pendingRetries[i].retries);
        }

      } else {
        // kolejka pełna -> spróbujemy za 500 ms
        pendingRetries[i].nextRetryAt = now + 500;
        if (DEBUG) Serial.println("Queue full, postponing pending retry");
      }
    }
  }
}
// ====== END ====================
//##################### SEND FAST ACK #######################
void SendFast(const Packet &ack){
     // dopisujemy do cache, żeby nie forwardować własnego pakietu
    addCache(ack.src, ack.mid);  // dopisz do rejestru wysyłanych/odebranych 
    String pkt = ack.type + "|" + ack.src + "|" + ack.dst + "|" + String(ack.mid) + "|" + String(ack.ttl) + "|" + ack.data;
radioSend(ack) ;

 if (DEBUG)   Serial.print("[TX - Fast] ");
 if (DEBUG)   Serial.println(pkt);

}
//=============DODAWANIE DO KOLEJKI [RX] ==================
void pushRecvMessage(const char* src, const char* dst, const char* text) {
    for (int i = MSG_HISTORY - 1; i > 0; i--) {
        recv[i] = recv[i - 1];
    }

    strncpy(recv[0].src, src, sizeof(recv[0].src));
    strncpy(recv[0].dst, dst, sizeof(recv[0].dst));
    strncpy(recv[0].text, text, sizeof(recv[0].text));

    recv[0].timestamp = millis();
    recv[0].delivered = true;

    if (recvCount < MSG_HISTORY) recvCount++;
    updateMessageCount() ;  
 
}

//========= OZNACZANIE DOSTARCZONYCH WIADOMOSCI =============
void markDelivered(int mid) {
    for (int i = 0; i < sentCount; i++) {
        if (sent[i].mid == mid) {
            sent[i].delivered = true;     
           return;
        }
    }
}

// ======== SEND BEA =====================
void sendBEA(String dest) {
//  float bat = readBattery();  // odczytaj napięcie baterii
  double lat = gps.location.lat();
  double lon = gps.location.lng();
  double alt = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
  Packet bea;
  bea.type = "BEA";
  bea.src = NODE_ID;
  bea.dst = dest;  // rozgłoszenie
  bea.mid = random(100, 999);
  bea.ttl = 5;
  // zbuduj payload w formacie klucz=wartość;klucz=wartość;...
  bea.data = "BAT=" + String(0.0, 1) + ";LAT=" + String(lat, 6) + ";LON=" + String(lon, 6) + ";ALT=" + String(alt, 1);
  bea.retries = 0;
  addCache(bea.src, bea.mid);  // dodaj BEA do cache (unika powtórzeń)
  if (!enqueuePacket(bea)) Serial.println("Queue full - BEA dropped");
}
