void StartSD() {  // ++++ zbedne
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);

    tft.fillRect(0, 40, 240, 40, TFT_BLACK);
    tft.setCursor(10, 40);
    tft.print("SD init...");

    Serial.println("[SD] Initializing...");

    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);   // CS idle

    if (!SD.begin(SD_CS)) {
        Serial.println("[SD] FAIL!");
        tft.setCursor(10, 60);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print("SD FAIL");
        return;
    }

    Serial.println("[SD] OK");
    tft.setCursor(10, 60);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("SD OK");

    // --- Lista plików (opcjonalnie) ---
    File root = SD.open("/");
    File file = root.openNextFile();

    Serial.println("[SD] File list:");
    while (file) {
        Serial.print(" - ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
}
void saveStationsToSD() {
    Serial.println("[SD] Tworzenie stations.txt...");

    File f = SD.open("/stations.txt", FILE_WRITE);
    if (!f) {
        Serial.println("[SD] ERROR: Nie mogę otworzyć stations.txt do zapisu!");
        return;
    }

    for (int i = 0; i < stationCount; i++) {
        f.print(stations[i].name);
        f.print("|");
        f.println(stations[i].url);
    }

    f.close();

    Serial.println("[SD] stations.txt zapisany OK");
}