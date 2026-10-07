void StartLoRa(){
  // === LoRa (SPI1) ===
  SPI1.setSCK(LORA_SCK);
  SPI1.setTX(LORA_MOSI);
  SPI1.setRX(LORA_MISO);
  SPI1.begin();

  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(868.0);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }

  radio.setBandwidth(250.0);
  radio.setSpreadingFactor(7);
  radio.setCodingRate(5);
  radio.setSyncWord(0x12);
  radio.setCRC(true);

  radio.setDio1Action(setFlag);


  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }
}

void checkForRequest() {
   if(!operationDone) return;
    operationDone = false;
    if(transmitFlag) {
        // TX DONE
        if (transmissionState == RADIOLIB_ERR_NONE) {
 //           Serial.println(F("transmission finished! - RECIVE !!!"));
        } else {
            Serial.print(F("failed, code "));
            Serial.println(transmissionState);
        }
      transmitFlag = false;
        radio.startReceive();
        return;
    }
    // RX DONE
    String pkt;
    int state = radio.readData(pkt);
    if (state == RADIOLIB_ERR_NONE) {
    rxPkt = pkt;
    rxReady = true;

 //       playTone(3000, 30, 0.4f);
        Serial.print(F("[SX1262] [RX]:\t\t"));
        Serial.println(pkt);


    }
    radio.startReceive();
}
void radioSend(const Packet &p) {

    // Złożenie pakietu do formatu tekstowego
    String pkt =
        p.type + "|" +
        p.src  + "|" +
        p.dst  + "|" +
        String(p.mid) + "|" +
        String(p.ttl) + "|" +
        p.data;

    Serial.print(F("[SX1262] TX: "));
    Serial.println(pkt);

    // Start transmisji
    transmissionState = radio.startTransmit(pkt);
    transmitFlag = true;
}
