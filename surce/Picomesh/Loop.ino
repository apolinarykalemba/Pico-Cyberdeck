void loop() {
     // --- PROFILER: START ---
 
t_loopStart = millis() ;
 // key = checkKeyboard();

static uint32_t lastKeyScan = 0;

if (millis() - lastKeyScan >= 30) {
    lastKeyScan = millis();
key = checkKeyboard() ;
}
 // ====================== 1) ODBIÓR LoRa======================================
  checkForRequest();
  // ======================= 2) PARSER=====================================
  if (rxReady) {
    processPacket(rxPkt);
    rxReady = false;
  }
  // ==================== 3) UI========================================
  uiHandle();
 CheckRedraw();  // CZY EKRAN WYMAGA RYSOWANIA.
  // ====================== 4) RETRY======================================
  checkPendingRetries();
  // ===================== 5) TX=======================================
  sendFromQueue();
    // ================== 6) AUDIO + BEACON==========================================
  if (millis() - lastBEA > 60000) {
    sendBEA("ALL");
    lastBEA = millis();  // zapisz czas wysłania BEA
  }
    // TX co 10 sekund
    if(!transmitFlag && nextTxTime >= millis() ) {

        Packet p;
        p.type = "MSG";
        p.src  = NODE_ID;
        p.dst  = "PICO";
        p.mid  = random(100, 999);
        p.ttl  = 5;
        p.data = "Wiad. od wezla - MESH";

  enqueuePacket(p);

        nextTxTime = millis() + 360000;
    }

    // =====PROFILER ===============
    static uint32_t last = 0;   // lokalne, nie globalne

    uint32_t now = millis();
    if (now - last >= 10000) {  // co 10 sekund
        last = now;

        uint32_t hz = clock_get_hz(clk_sys);
        Serial.print("CPU: ");
        Serial.print(hz / 1000000);
        Serial.println(" MHz");
    }
   t_loopEnd = millis();
  t_lastLoop = t_loopEnd - t_loopStart;

  if (t_lastLoop > t_maxLoop) t_maxLoop = t_lastLoop;

  profilerPrint(); // Funkcja debugujaca 
    offBacklight();
  
}
