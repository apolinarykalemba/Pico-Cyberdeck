
// ===============================
// FUNKCJA: odtwórz krótki dźwięk
// freq = częstotliwość w Hz
// timeMs = czas trwania w ms
// volume = głośność 0.0–1.0
// ===============================
void StartSound(){
  i2s.setBitsPerSample(16);
  i2s.begin(sampleRate);
   playTone(3000, 30, 0.4f); 
}

void playTone(float freq, int timeMs, float volume) {

  float phase = 0.0f;
  float step = freq / sampleRate;

  int samples = (sampleRate * timeMs) / 1000;

  for (int i = 0; i < samples; i++) {

    float s = sin(phase * 6.283185f);
    int16_t out = (int16_t)(s * 15000 * volume);

    i2s.write(out);


    phase += step;
    if (phase >= 1.0f) phase -= 1.0f;
  }
}