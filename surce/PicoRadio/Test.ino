void test(){
// ===== MEMORY TEST
static uint32_t lastBufferDisplay = 0;
static uint32_t lastFill = 0;

if (buff && millis() - lastBufferDisplay >= 500) {

    uint32_t now = millis();
    uint32_t fill = buff->getFillLevel();

    uint32_t dt = now - lastBufferDisplay;

    // Zmiana poziomu bufora
    int32_t delta = (int32_t)fill - (int32_t)lastFill;

    // KB/s — dodatnie = bufor się napełnia
    //        ujemne = bufor się opróżnia
    int32_t flow = (delta * 1000L) / dt / 1024L;

    uint32_t percent = (fill * 100UL) / 131072UL;

    char bufText[64];

    snprintf(bufText, sizeof(bufText),
             "BUF:%luK %lu%%  FLOW:%ldK/s",
             fill / 1024UL,
             percent,
             flow);

//    Serial.println( bufText);
setLine(14 , bufText) ;
    lastFill = fill;
    lastBufferDisplay = now;
}

}