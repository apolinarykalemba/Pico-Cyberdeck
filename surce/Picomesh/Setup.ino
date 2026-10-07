
//================ SETUP =================
void setup() {
  Serial.begin(115200);
tft.setSwapBytes(true);
//set_sys_clock_khz(25000, true); // 25 MHz
                          //  set_sys_clock_khz(50000, true); // 48 MHz
//                           set_sys_clock_khz(100000, true); // 100 MHz 
StartTFT() ;
StartGPS() ;
StartLoRa() ;
StartKeyboard() ;
StartSound() ;
StartSD() ;
//StartWiFi("NETIASPOT-2.4GHz-s5HB", "jvH3452M");
  drawTopBar();
   updateNode("ALL", 0, 0.0, 0.0, 0.0, 0.0, radio.getRSSI()); 
   updateNode("OLO", 0, 0.0, 0.0, 0.0, 0.0, radio.getRSSI()); 

  sendBEA("ALL");
  lastBEA = millis();  // zapisz czas wysłania BEA
  uiDrawIdleScreen() ;
}