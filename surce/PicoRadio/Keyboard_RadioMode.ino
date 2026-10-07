void StartKeyboardRadioMode() {

    // I2C na właściwych pinach
    Wire.setSDA(MCP_SDA);
    Wire.setSCL(MCP_SCL);
    Wire.begin();

    if (!mcp.begin_I2C()) {
        Serial.println("MCP23X17 not found!");
        while (1);
    }

    // Wszystkie RZĘDY jako OUTPUT HIGH
    // (żeby klawisze miały zasilanie)

        mcp.pinMode(cols[1], OUTPUT);
        mcp.digitalWrite(cols[1], LOW);
        mcp.pinMode(cols[2], OUTPUT);
        mcp.digitalWrite(cols[2], LOW);
        mcp.pinMode(cols[5], OUTPUT);
        mcp.digitalWrite(cols[5], LOW);
    // Wszystkie KOLUMNY jako INPUT_PULLUP
    for (int c = 0; c < 7; c++) {
        mcp.pinMode(rows[c], INPUT_PULLUP);
    }

    Serial.println("Klawiatura bez skanowania gotowa.");
}