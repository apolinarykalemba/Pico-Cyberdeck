

void StartSD() {
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
     SD.end(); 
  SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));  

  }