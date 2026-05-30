#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const uint8_t POCHACCO_3X3_BMP[] PROGMEM = {
    0xc0, 0x8c, 0x30, 0xe0, 0x58, 0x70, 0xf3, 0x50, 0xf0, 0x70, 0xf9, 0xc0,
    0x3b, 0x07, 0x00, 0x1e, 0x03, 0x00, 0x04, 0x01, 0x00, 0x08, 0x00, 0x80,
    0x10, 0x00, 0x40, 0x10, 0x00, 0x40, 0x10, 0x00, 0x40, 0x10, 0x00, 0x40,
    0x30, 0x00, 0x60, 0x23, 0x06, 0x20, 0x63, 0x06, 0x30, 0x40, 0x60, 0x10,
    0x60, 0x00, 0x30, 0x30, 0x00, 0x60, 0x1c, 0x01, 0xc0, 0x07, 0xff, 0x00};

const uint8_t SHIMAENAGA_3X3_BMP[] PROGMEM = {
    0x00, 0x8c, 0x00, 0x03, 0x58, 0x00, 0x00, 0xf8, 0x00, 0x03, 0x06, 0x00,
    0x0c, 0x01, 0x00, 0x10, 0x00, 0x80, 0x13, 0x06, 0x80, 0x23, 0x66, 0x40,
    0x20, 0x00, 0x60, 0xe0, 0x00, 0x70, 0xf0, 0x00, 0xf0, 0xf8, 0x01, 0xf0,
    0xfc, 0x01, 0xf0, 0x7c, 0x03, 0xe0, 0x7c, 0x03, 0xb0, 0x8e, 0x03, 0x10,
    0x86, 0x02, 0x10, 0x60, 0x00, 0x20, 0x18, 0x00, 0xc0, 0x07, 0xff, 0x00};

const uint8_t POCHACCO_5X5_BMP[] PROGMEM = {
    0xc4, 0x80, 0xca, 0xb0, 0x67, 0x30, 0x38, 0xe0, 0x10, 0x40, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x08, 0x80, 0x42, 0x10, 0x40, 0x10, 0x30, 0x60};

const uint8_t SHIMAENAGA_5X5_BMP[] PROGMEM = {
    0x15, 0x80, 0x0f, 0x00, 0x30, 0xc0, 0x40, 0x20, 0x4a, 0xa0, 0xc2, 0x30,
    0xc0, 0x30, 0xe0, 0x70, 0x60, 0x60, 0xb0, 0xd0, 0x80, 0x10, 0x60, 0x60};

const uint8_t POCHACCO_5X5_INV_BMP[] PROGMEM = {
    0x3b, 0x7f, 0x35, 0x4f, 0x98, 0xcf, 0xc7, 0x1f, 0xef, 0xbf, 0xdf, 0xdf,
    0xdf, 0xdf, 0xdf, 0xdf, 0xf7, 0x7f, 0xbd, 0xef, 0xbf, 0xef, 0xcf, 0x9f};

const uint8_t SHIMAENAGA_5X5_INV_BMP[] PROGMEM = {
    0xea, 0x7f, 0xf0, 0xff, 0xcf, 0x3f, 0xbf, 0xdf, 0xb5, 0x5f, 0x3d, 0xcf,
    0x3f, 0xcf, 0x1f, 0x8f, 0x9f, 0x9f, 0x4f, 0x2f, 0x7f, 0xef, 0x9f, 0x9f};

enum GameState
{
  STATE_TITLE,
  STATE_STAGE_START,
  STATE_PLAY,
  STATE_STAGE_CLEAR,
  STATE_ALL_CLEAR
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

GameState state = STATE_TITLE;

struct StageConfig
{
  int width;
  int height;
};

const StageConfig stages[] = {
    {3, 3}, // Stage 1
    {5, 5}, // Stage 2
    {5, 5}, // Stage 3
    {5, 5}, // Stage 4
    {5, 5}  // Stage 5
};
const int stageCount = sizeof(stages) / sizeof(stages[0]);
int currentStageIndex = 0;
int currentStageRotationQuarter = 0;
int tileRotationQuarter[5][5];

int cursorX = 0;
int cursorY = 0;
int pochaccoX = 1;
int pochaccoY = 1;

unsigned long stageStartMs = 0;
unsigned long stageElapsedMs = 0;
unsigned long totalMs = 0;
unsigned long stateEnterMs = 0;

bool titleShown = false;

int currentStageWidth()
{
  return stages[currentStageIndex].width;
}

int currentStageHeight()
{
  return stages[currentStageIndex].height;
}

const uint8_t *currentPochaccoBmp()
{
  if (currentStageIndex == 0)
  {
    return POCHACCO_3X3_BMP;
  }
  if (currentStageIndex == 3)
  {
    return POCHACCO_5X5_INV_BMP;
  }
  return POCHACCO_5X5_BMP;
}

const uint8_t *currentShimaenagaBmp()
{
  if (currentStageIndex == 0)
  {
    return SHIMAENAGA_3X3_BMP;
  }
  if (currentStageIndex == 3)
  {
    return SHIMAENAGA_5X5_INV_BMP;
  }
  return SHIMAENAGA_5X5_BMP;
}

int currentBmpWidth()
{
  return currentStageIndex == 0 ? 20 : 12;
}

int currentBmpHeight()
{
  return currentStageIndex == 0 ? 20 : 12;
}

int currentRotationQuarter()
{
  return (currentStageIndex == 2 || currentStageIndex == 4) ? currentStageRotationQuarter : 0;
}

bool isStage5()
{
  return currentStageIndex == 4;
}

void setupStageTileRotations()
{
  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      tileRotationQuarter[y][x] = 0;
    }
  }

  if (isStage5())
  {
    int w = currentStageWidth();
    int h = currentStageHeight();
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        tileRotationQuarter[y][x] = random(1, 4);
      }
    }
  }
}

bool readBitmapPixel(const uint8_t *bmp, int bmpW, int x, int y)
{
  int bytesPerRow = (bmpW + 7) / 8;
  int byteIndex = y * bytesPerRow + (x / 8);
  uint8_t b = pgm_read_byte(bmp + byteIndex);
  int bitIndex = 7 - (x % 8);
  return ((b >> bitIndex) & 0x01) != 0;
}

void drawBitmapRotated(int dstX, int dstY, const uint8_t *bmp, int bmpW, int bmpH, int quarterTurns)
{
  int turns = ((quarterTurns % 4) + 4) % 4;
  if (turns == 0)
  {
    display.drawBitmap(dstX, dstY, bmp, bmpW, bmpH, SSD1306_WHITE);
    return;
  }

  for (int sy = 0; sy < bmpH; sy++)
  {
    for (int sx = 0; sx < bmpW; sx++)
    {
      if (!readBitmapPixel(bmp, bmpW, sx, sy))
      {
        continue;
      }

      int dx = 0;
      int dy = 0;
      if (turns == 1)
      {
        dx = bmpH - 1 - sy;
        dy = sx;
      }
      else if (turns == 2)
      {
        dx = bmpW - 1 - sx;
        dy = bmpH - 1 - sy;
      }
      else
      {
        dx = sy;
        dy = bmpW - 1 - sx;
      }
      display.drawPixel(dstX + dx, dstY + dy, SSD1306_WHITE);
    }
  }
}

void playMoveSe()
{
  tone(BUZZER_PIN, 1200, 20);
}

void playClearSe()
{
  tone(BUZZER_PIN, 1800, 80);
  delay(90);
  tone(BUZZER_PIN, 2200, 80);
}

void enterState(GameState next)
{
  state = next;
  stateEnterMs = millis();
}

void randomizePochacco()
{
  int w = currentStageWidth();
  int h = currentStageHeight();
  do
  {
    pochaccoX = random(0, w);
    pochaccoY = random(0, h);
  } while (pochaccoX == 0 && pochaccoY == 0);
}

bool moveCursor(int dx, int dy)
{
  int w = currentStageWidth();
  int h = currentStageHeight();
  int nx = cursorX + dx;
  int ny = cursorY + dy;
  if (nx < 0 || ny < 0 || nx >= w || ny >= h)
  {
    return false;
  }
  cursorX = nx;
  cursorY = ny;
  playMoveSe();
  return true;
}

void drawTitle()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(18, 20);
  display.println("WHERE IS POCHACCO?");
  display.setCursor(18, 38);
  display.println("PRESS ANY ARROW");
  display.display();
}

void drawStageStart()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(18, 24);
  display.print("STAGE ");
  display.println(currentStageIndex + 1);
  display.display();
}

void drawPlay()
{
  const int stageWidth = currentStageWidth();
  const int stageHeight = currentStageHeight();
  const int tileSizeW = SCREEN_WIDTH / stageWidth;
  const int tileSizeH = SCREEN_HEIGHT / stageHeight;
  const int tileSize = tileSizeW < tileSizeH ? tileSizeW : tileSizeH;
  const int gridPixelW = tileSize * stageWidth;
  const int gridPixelH = tileSize * stageHeight;
  const int offsetX = (SCREEN_WIDTH - gridPixelW) / 2;
  const int offsetY = (SCREEN_HEIGHT - gridPixelH) / 2;
  const int bmpW = currentBmpWidth();
  const int bmpH = currentBmpHeight();
  const uint8_t *pochaccoBmp = currentPochaccoBmp();
  const uint8_t *shimaenagaBmp = currentShimaenagaBmp();
  const int rotationQuarter = currentRotationQuarter();

  display.clearDisplay();

  for (int y = 0; y < stageHeight; y++)
  {
    for (int x = 0; x < stageWidth; x++)
    {
      int tileX = offsetX + x * tileSize;
      int tileY = offsetY + y * tileSize;

      const uint8_t *bmp = (x == pochaccoX && y == pochaccoY) ? pochaccoBmp : shimaenagaBmp;
      int bmpX = tileX + (tileSize - bmpW) / 2;
      int bmpY = tileY + (tileSize - bmpH) / 2;
      int drawRotation = rotationQuarter;
      if (isStage5() && !(x == pochaccoX && y == pochaccoY))
      {
        drawRotation = tileRotationQuarter[y][x];
      }
      drawBitmapRotated(bmpX, bmpY, bmp, bmpW, bmpH, drawRotation);

      if (x == cursorX && y == cursorY)
      {
        display.drawRect(tileX, tileY, tileSize, tileSize, SSD1306_WHITE);
      }
    }
  }

  display.display();
}

void drawStageClear()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(8, 12);
  display.println("CLEAR!");
  display.setTextSize(1);
  display.setCursor(8, 30);
  display.print("STAGE ");
  display.print(currentStageIndex + 1);
  display.setCursor(8, 42);
  display.print("TIME: ");
  display.print(stageElapsedMs);
  display.println(" ms");
  display.display();
}

void drawAllClear()
{
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
  display.setCursor(8, 54);
  display.println("PRESS ARROW");
  display.display();
}

bool anyArrowPressed()
{
  return digitalRead(BUTTON_UP) == LOW ||
         digitalRead(BUTTON_DOWN) == LOW ||
         digitalRead(BUTTON_LEFT) == LOW ||
         digitalRead(BUTTON_RIGHT) == LOW;
}

void setup()
{
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

void loop()
{
  if (state == STATE_TITLE)
  {
    if (!titleShown)
    {
      drawTitle();
      titleShown = true;
    }
    if (anyArrowPressed())
    {
      titleShown = false;
      currentStageIndex = 0;
      totalMs = 0;
      enterState(STATE_STAGE_START);
    }
    delay(30);
    return;
  }

  if (state == STATE_STAGE_START)
  {
    drawStageStart();
    if (millis() - stateEnterMs >= 700)
    {
      if (currentStageIndex == 2 || currentStageIndex == 4)
      {
        currentStageRotationQuarter = random(1, 4);
      }
      else
      {
        currentStageRotationQuarter = 0;
      }
      setupStageTileRotations();
      cursorX = 0;
      cursorY = 0;
      randomizePochacco();
      stageStartMs = millis();
      enterState(STATE_PLAY);
    }
    delay(30);
    return;
  }

  if (state == STATE_PLAY)
  {
    bool moved = false;

    if (digitalRead(BUTTON_UP) == LOW)
      moved = moveCursor(0, -1);
    else if (digitalRead(BUTTON_DOWN) == LOW)
      moved = moveCursor(0, 1);
    else if (digitalRead(BUTTON_LEFT) == LOW)
      moved = moveCursor(-1, 0);
    else if (digitalRead(BUTTON_RIGHT) == LOW)
      moved = moveCursor(1, 0);

    drawPlay();

    if (cursorX == pochaccoX && cursorY == pochaccoY)
    {
      stageElapsedMs = millis() - stageStartMs;
      totalMs += stageElapsedMs;
      playClearSe();
      enterState(STATE_STAGE_CLEAR);
    }

    delay(moved ? 140 : 30);
    return;
  }

  if (state == STATE_STAGE_CLEAR)
  {
    drawStageClear();
    if (millis() - stateEnterMs >= 1200)
    {
      currentStageIndex++;
      if (currentStageIndex >= stageCount)
      {
        enterState(STATE_ALL_CLEAR);
      }
      else
      {
        enterState(STATE_STAGE_START);
      }
    }
    delay(30);
    return;
  }

  if (state == STATE_ALL_CLEAR)
  {
    drawAllClear();
    if (anyArrowPressed())
    {
      currentStageIndex = 0;
      totalMs = 0;
      enterState(STATE_TITLE);
    }
    delay(30);
  }
}
