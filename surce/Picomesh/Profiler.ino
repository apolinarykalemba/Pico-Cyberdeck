

void profilerPrint() {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint < 1000) return;
    lastPrint = millis();

    tft.setTextSize(2);
    tft.setCursor(0, 300);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    // Podstawowa linia
    
    tft.printf("Lp:%6lu  Max:%6lu", t_lastLoop, t_maxLoop);
    tft.setTextSize(1);
    // Jeśli jest FIX → dopisz GPS
    if (gps.location.isValid() && gps.location.age() < 2000) {
        tft.printf("  Lat:%2.6f  Lon:%2.6f",
            gps.location.lat(),
            gps.location.lng()
        );
    }

    tft.println();
}
