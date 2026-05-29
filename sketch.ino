#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define TILE_BITMAP_W 20
#define TILE_BITMAP_H 20

enum GameState {
  STATE_TITLE,
  STATE_STAGE_START,
  STATE_PLAY,
  STATE_STAGE_CLEAR,
  STATE_ALL_CLEAR
};

const uint8_t POCHACCO_BMP[] PROGMEM = {
  0xc0, 0x8c, 0x30, 0xe0, 0x58, 0x70, 0xf3, 0x50, 0xf0, 0x70, 0xf9, 0xc0,
  0x3b, 0x07, 0x00, 0x1e, 0x03, 0x00, 0x04, 0x01, 0x00, 0x08, 0x00, 0x80,
  0x10, 0x00, 0x40, 0x10, 0x00, 0x40, 0x10, 0x00, 0x40, 0x10, 0x00, 0x40,
  0x30, 0x00, 0x60, 0x23, 0x06, 0x20, 0x63, 0x06, 0x30, 0x40, 0x60, 0x10,
  0x60, 0x00, 0x30, 0x30, 0x00, 0x60, 0x1c, 0x01, 0xc0, 0x07, 0xff, 0x00
};

const uint8_t SHIMAENAGA_BMP[] PROGMEM = {
  0x00, 0x8c, 0x00, 0x00, 0x58, 0x00, 0x03, 0x50, 0x00, 0x00, 0xf8, 0x00,
  0x03, 0x06, 0x00, 0x06, 0x01, 0x00, 0x08, 0x00, 0x80, 0x10, 0x00, 0xc0,
  0x33, 0x06, 0x40, 0x23, 0x66, 0x60, 0x70, 0x00, 0xf0, 0xf8, 0x01, 0xf0,
  0xfc, 0x03, 0xf0, 0x7c, 0x03, 0xe0, 0x3e, 0x07, 0x30, 0x4e, 0x06, 0x10,
  0x63, 0x04, 0x30, 0x30, 0x00, 0x60, 0x1c, 0x01, 0xc0, 0x07, 0xff, 0x00
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

GameState state = STATE_TITLE;

const int stageWidth = 3;
const int stageHeight = 3;

int cursorX = 0;
int cursorY = 0;
int pochaccoX = 1;
int pochaccoY = 1;

unsigned long stageStartMs = 0;
unsigned long stageElapsedMs = 0;
unsigned long totalMs = 0;
unsigned long stateEnterMs = 0;

bool titleShown = false;

void playMoveSe() {
  tone(BUZZER_PIN, 1200, 20);
}

void playClearSe() {
  tone(BUZZER_PIN, 1800, 80);
  delay(90);
  tone(BUZZER_PIN, 2200, 80);
}

void enterState(GameState next) {
  state = next;
  stateEnterMs = millis();
}

void randomizePochacco() {
  do {
    pochaccoX = random(0, stageWidth);
    pochaccoY = random(0, stageHeight);
  } while (pochaccoX == 0 && pochaccoY == 0);
}

bool moveCursor(int dx, int dy) {
  int nx = cursorX + dx;
  int ny = cursorY + dy;
  if (nx < 0 || ny < 0 || nx >= stageWidth || ny >= stageHeight) {
    return false;
  }
  cursorX = nx;
  cursorY = ny;
  playMoveSe();
  return true;
}

void drawTitle() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(18, 20);
  display.println("WHERE IS POCHACCO?");
  display.setCursor(18, 38);
  display.println("PRESS ANY ARROW");
  display.display();
}

void drawStageStart() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(18, 24);
  display.println("STAGE 1");
  display.display();
}

void drawPlay() {
  const int tileW = SCREEN_WIDTH / stageWidth;
  const int tileH = SCREEN_HEIGHT / stageHeight;

  display.clearDisplay();

  for (int y = 0; y < stageHeight; y++) {
    for (int x = 0; x < stageWidth; x++) {
      int tileX = x * tileW;
      int tileY = y * tileH;

      const uint8_t *bmp = (x == pochaccoX && y == pochaccoY) ? POCHACCO_BMP : SHIMAENAGA_BMP;
      int bmpX = tileX + (tileW - TILE_BITMAP_W) / 2;
      int bmpY = tileY + (tileH - TILE_BITMAP_H) / 2;

      display.drawBitmap(bmpX, bmpY, bmp, TILE_BITMAP_W, TILE_BITMAP_H, SSD1306_WHITE);

      if (x == cursorX && y == cursorY) {
        display.drawRect(tileX, tileY, tileW, tileH, SSD1306_WHITE);
      }
    }
  }

  display.display();
}

void drawStageClear() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(8, 12);
  display.println("CLEAR!");
  display.setTextSize(1);
  display.setCursor(8, 42);
  display.print("TIME: ");
  display.print(stageElapsedMs);
  display.println(" ms");
  display.display();
}

void drawAllClear() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(8, 10);
  display.println("ALL CLEAR");
  display.setTextSize(1);
  display.setCursor(8, 40);
  display.print("TOTAL: ");
  display.print(totalMs);
  display.println(" ms");
  display.display();
}

bool anyArrowPressed() {
  return digitalRead(BUTTON_UP) == LOW ||
         digitalRead(BUTTON_DOWN) == LOW ||
         digitalRead(BUTTON_LEFT) == LOW ||
         digitalRead(BUTTON_RIGHT) == LOW;
}

void setup() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  randomSeed((uint32_t)micros());

  enterState(STATE_TITLE);
}

void loop() {
  if (state == STATE_TITLE) {
    if (!titleShown) {
      drawTitle();
      titleShown = true;
    }
    if (anyArrowPressed()) {
      titleShown = false;
      enterState(STATE_STAGE_START);
    }
    delay(30);
    return;
  }

  if (state == STATE_STAGE_START) {
    drawStageStart();
    if (millis() - stateEnterMs >= 700) {
      cursorX = 0;
      cursorY = 0;
      randomizePochacco();
      stageStartMs = millis();
      enterState(STATE_PLAY);
    }
    delay(30);
    return;
  }

  if (state == STATE_PLAY) {
    bool moved = false;

    if (digitalRead(BUTTON_UP) == LOW) moved = moveCursor(0, -1);
    else if (digitalRead(BUTTON_DOWN) == LOW) moved = moveCursor(0, 1);
    else if (digitalRead(BUTTON_LEFT) == LOW) moved = moveCursor(-1, 0);
    else if (digitalRead(BUTTON_RIGHT) == LOW) moved = moveCursor(1, 0);

    drawPlay();

    if (cursorX == pochaccoX && cursorY == pochaccoY) {
      stageElapsedMs = millis() - stageStartMs;
      totalMs += stageElapsedMs;
      playClearSe();
      enterState(STATE_STAGE_CLEAR);
    }

    delay(moved ? 140 : 30);
    return;
  }

  if (state == STATE_STAGE_CLEAR) {
    drawStageClear();
    if (millis() - stateEnterMs >= 1200) {
      enterState(STATE_ALL_CLEAR);
    }
    delay(30);
    return;
  }

  if (state == STATE_ALL_CLEAR) {
    drawAllClear();
    if (anyArrowPressed()) {
      totalMs = 0;
      enterState(STATE_TITLE);
    }
    delay(30);
  }
}
