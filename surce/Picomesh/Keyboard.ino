// =====  STARTUP ============
void StartKeyboard(){
  if (!mcp.begin_I2C()) {
    Serial.println("MCP23X17 not found!");
    while (1);
  }
mcp.pinMode(SHIFT_LED, OUTPUT);
mcp.digitalWrite(SHIFT_LED, LOW);   // start: LED off
  // RZĘDY jako wyjścia (HIGH)
  for (int i = 0; i < 8; i++) {
    mcp.pinMode(rows[i], OUTPUT);
    mcp.digitalWrite(rows[i], HIGH);
  }
  // KOLUMNY jako wejścia z pull‑up
  for (int i = 0; i < 7; i++) {
    mcp.pinMode(cols[i], INPUT_PULLUP);
  }

  Serial.println("Klawiatura 8x7 gotowa.");  
    // ============ KEYBOARD ================
   Wire.setSDA(MCP_SDA);
  Wire.setSCL(MCP_SCL);
  Wire.begin();
}
// ==== Obsluga Klawiatury ==============
String checkKeyboard() {
    unsigned long now = millis();
    String result = "";
    for (int r = 0; r < 8; r++) {
        mcp.digitalWrite(rows[r], LOW);
        for (int c = 0; c < 7; c++) {
            if (mcp.digitalRead(cols[c]) == LOW) {
                if (now - lastPress[r][c] > debounceTime) {
                    lastPress[r][c] = now;
                   String raw = keymap[r][c];
               if (raw != "") {    
                 analogWrite(TFT_BL, brightness); 
             lastTFToff = millis();  
set_sys_clock_khz(150000, true); // 150 MHz                 
               }   
                   if (raw == "fn") {
                        fnLock = !fnLock;
       mcp.digitalWrite(SHIFT_LED, fnLock ? HIGH : LOW);  // SHUFT LED ON/OFF
         updateModeSymbol()  ; 
   drawTopBar() ;                     
                        result = "fn";
                    } else {
                        result = fnLock ? keymapFN[r][c] : raw;
                    }
                }
            }
        }
        mcp.digitalWrite(rows[r], HIGH);
    }
      if (result == "rt") {
    brightness += 20;
    if (brightness > 255) brightness = 255;

    analogWrite(TFT_BL, brightness);

  }

  if (result == "lt") {
    brightness -= 20;
    if (brightness < 20) brightness = 10;

    analogWrite(TFT_BL, brightness);
    delay(100);
  }

    return result;
}