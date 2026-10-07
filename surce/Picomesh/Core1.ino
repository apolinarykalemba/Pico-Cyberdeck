void setup1() {

  // AUDIO
  i2s.setBitsPerSample(16);
  i2s.begin(sampleRate);

  playTone(3000, 30, 0.4f);
}


void loop1() {

  // GPS
  if (Serial1.available()) {
    gps.encode(Serial1.read());
  }

  tight_loop_contents();
}