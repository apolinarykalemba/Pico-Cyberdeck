
 /*
 =======================================================
 PicoPaint
 ST7796 + FT6236
 Raspberry Pi Pico 2 W
 =======================================================
*/

#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>

#define TOUCH_SDA 4
#define TOUCH_SCL 5
#define TOUCH_RST 6
#define TOUCH_INT 7

#define FT6236_ADDR 0x38

// =======================================================
// KALIBRACJA
// =======================================================

#define TOUCH_X_MIN 17
#define TOUCH_X_MAX 298

#define TOUCH_Y_MIN 11
#define TOUCH_Y_MAX 442

// =======================================================
// TFT
// =======================================================

TFT_eSPI tft = TFT_eSPI();

const uint16_t TOOLBAR_W = 60;

uint16_t drawColor = TFT_YELLOW;

// =======================================================
// KOLORY
// =======================================================

const uint16_t colors[] =
{
    TFT_RED,
    TFT_GREEN,
    TFT_BLUE,
    TFT_YELLOW,
    TFT_MAGENTA,
    TFT_CYAN,
    TFT_WHITE,
    TFT_BLACK
};

const char *colorNames[] =
{
    "RED",
    "GRN",
    "BLU",
    "YEL",
    "MAG",
    "CYN",
    "WHT",
    "BLK"
};

const uint8_t COLOR_COUNT = 8;

uint8_t selectedColor = 3;   // startowo YEL


// =======================================================
// RYSOWANIE
// =======================================================

bool drawing = false;

uint16_t lastX = 0;
uint16_t lastY = 0;


// =======================================================
// NIEBLOKUJĄCY DOTYK
// =======================================================

const uint32_t TOUCH_INTERVAL = 10;

uint32_t lastTouchRead = 0;


// =======================================================
// FT6236
// =======================================================

bool touchReadRaw(uint16_t &x, uint16_t &y)
{
    Wire.beginTransmission(FT6236_ADDR);
    Wire.write(0x02);

    if (Wire.endTransmission(false) != 0)
        return false;

    Wire.requestFrom(FT6236_ADDR, 5);

    if (Wire.available() < 5)
        return false;

    uint8_t td_status = Wire.read() & 0x0F;

    if (td_status == 0)
        return false;

    uint16_t xh = Wire.read() & 0x0F;
    uint16_t xl = Wire.read();

    uint16_t yh = Wire.read() & 0x0F;
    uint16_t yl = Wire.read();

    x = (xh << 8) | xl;
    y = (yh << 8) | yl;

    return true;
}


// =======================================================
// MAPOWANIE
// =======================================================

void mapTouch(
    uint16_t rawXv,
    uint16_t rawYv,
    uint16_t &sx,
    uint16_t &sy)
{
    uint16_t rx = rawYv;
    uint16_t ry = rawXv;

    sx = map(
        rx,
        TOUCH_Y_MAX,
        TOUCH_Y_MIN,
        0,
        tft.width() - 1);

    sy = map(
        ry,
        TOUCH_X_MIN,
        TOUCH_X_MAX,
        0,
        tft.height() - 1);

    sx = constrain(sx, 0, tft.width() - 1);
    sy = constrain(sy, 0, tft.height() - 1);
}


// =======================================================
// TOOLBAR
// =======================================================

const uint16_t COLOR_BUTTON_H = 35;

const uint16_t CLR_Y = 280;
const uint16_t CLR_H = 40;


void drawToolbar()
{
//    tft.fillScreen(TFT_BLACK);

    // -------------------------------
    // KOLORY
    // -------------------------------

    for (uint8_t i = 0; i < COLOR_COUNT; i++)
    {
        uint16_t y = i * COLOR_BUTTON_H;

        tft.fillRect(
            0,
            y,
            TOOLBAR_W,
            COLOR_BUTTON_H,
            colors[i]);

        // Kolor tekstu
        if (i == 2 || i == 4 ||  i == 7)
            tft.setTextColor(TFT_WHITE);
        else
            tft.setTextColor(TFT_BLACK);

        tft.drawCentreString(
            colorNames[i],
            TOOLBAR_W / 2,
            y + 9,
            2);
    }

    // -------------------------------
    // CZARNA OBWÓDKA WYBRANEGO
    // -------------------------------

    uint16_t selectedY =
        selectedColor * COLOR_BUTTON_H;
    tft.drawRect(
        0,
        selectedY,
        TOOLBAR_W - 1,
        COLOR_BUTTON_H - 1,
        TFT_WHITE);
    tft.drawRect(
        0,
        selectedY,
        TOOLBAR_W - 2,
        COLOR_BUTTON_H - 2,
        TFT_WHITE);
    tft.drawRect(
        0,
        selectedY,
        TOOLBAR_W - 3,
        COLOR_BUTTON_H - 3,
        TFT_WHITE);
    // -------------------------------
    // CLR
    // -------------------------------

    tft.fillRect(
        0,
        CLR_Y,
        TOOLBAR_W,
        CLR_H,
        TFT_DARKGREY);

    tft.setTextColor(TFT_WHITE);

    tft.drawCentreString(
        "CLR",
        TOOLBAR_W / 2,
        CLR_Y + 12,
        2);

    // -------------------------------
    // GRANICA
    // -------------------------------

    tft.drawLine(
        TOOLBAR_W,
        0,
        TOOLBAR_W,
        tft.height() - 1,
        TFT_DARKGREY);
}


// =======================================================
// TOOLBAR – OBSŁUGA
// =======================================================

bool handleToolbar(uint16_t x, uint16_t y)
{
    if (x >= TOOLBAR_W)
        return false;

    // -------------------------------
    // KOLOR
    // -------------------------------

    if (y < COLOR_COUNT * COLOR_BUTTON_H)
    {
        uint8_t index =
            y / COLOR_BUTTON_H;

        selectedColor = index;
        drawColor = colors[index];

        drawing = false;

        // Rysujemy toolbar ponownie
        // żeby obwódka przeszła na nowy kolor
        drawToolbar();

        return true;
    }

    // -------------------------------
    // CLR
    // -------------------------------

    if (y >= CLR_Y)
    {
        tft.fillRect(
            TOOLBAR_W + 1,
            0,
            tft.width() - TOOLBAR_W - 1,
            tft.height(),
            TFT_BLACK);

        drawing = false;

        return true;
    }

    return true;
}


// =======================================================
// SETUP
// =======================================================

void setup()
{
    Serial.begin(115200);

    // I2C
    Wire.setSDA(TOUCH_SDA);
    Wire.setSCL(TOUCH_SCL);
    Wire.begin();

    // RESET FT6236
    pinMode(TOUCH_RST, OUTPUT);

    digitalWrite(TOUCH_RST, LOW);
    delay(5);

    digitalWrite(TOUCH_RST, HIGH);
    delay(50);

    pinMode(TOUCH_INT, INPUT_PULLUP);

    // TFT
    tft.init();
  tft.invertDisplay(true); 
  
    tft.setRotation(3);

    drawToolbar();

    tft.setTextColor(TFT_GREEN);

    tft.drawString(
        "PicoPaint",
        TOOLBAR_W + 10,
        10,
        2);
}


// =======================================================
// LOOP – NIEBLOKUJĄCY
// =======================================================

void loop()
{
    uint32_t now = millis();

    // -------------------------------
    // Odczyt dotyku co 10 ms
    // -------------------------------

    if (now - lastTouchRead < TOUCH_INTERVAL)
        return;

    lastTouchRead = now;

    // -------------------------------
    // Odczyt FT6236
    // -------------------------------

    uint16_t tx;
    uint16_t ty;

    if (!touchReadRaw(tx, ty))
    {
        drawing = false;
        return;
    }

    // -------------------------------
    // Mapowanie
    // -------------------------------

    uint16_t sx;
    uint16_t sy;

    mapTouch(
        tx,
        ty,
        sx,
        sy);

    // -------------------------------
    // Toolbar
    // -------------------------------

    if (handleToolbar(sx, sy))
        return;

    // -------------------------------
    // RYSOWANIE
    // -------------------------------

    if (!drawing)
    {
        lastX = sx;
        lastY = sy;

        drawing = true;
    }
    else
    {
        tft.drawLine(
            lastX,
            lastY,
            sx,
            sy,
            drawColor);

        lastX = sx;
        lastY = sy;
    }
}
