float readBattery() {
    uint32_t acc = 0;

    // 24–32 próbek daje stabilny wynik
    for (int i = 0; i < 15; i++) {
        acc += analogRead(28);   // 10-bit: 0–1023
        sleep_us(50);            // RP2040 lubi małą przerwę
    }

    float raw = acc / 32.0;

    // 10-bit ADC, Vref ~3.3 V
    float v_adc = raw * 3.3 / 1023.0;

    // dzielnik 150k / 100k → mnożnik 2.8 (skalibrowany)
    float v_bat = v_adc * 2 * 3;

    return v_bat;
}
