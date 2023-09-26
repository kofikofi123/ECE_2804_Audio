#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_I2C_ADDR 0x3C
#define INTRO_DELAY 3000
#define REFRESH_DELAY 25


#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8

enum class SpectogramSTATES {
  WELCOME_PAGE,
  MAIN_PAGE
};

const char* DisplaySTRS[] = {"Home Audio", "Jacob and Kofi", "ECE 2804", "Bass", "Mid", "Treble"};
Adafruit_SSD1306 spectogramDisplay(SCREEN_WIDTH, SCREEN_HEIGHT);
SpectogramSTATES currentState;

const int BasePIN   = A0;
const int MidPIN    = A1;
const int TreblePIN = A2;

const int CommonDisplayPOS[] = {(SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), (SCREEN_WIDTH / 6), (2 * SCREEN_HEIGHT) / 3};
void setup() {
  Serial.begin(9600);
  if (!spectogramDisplay.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println("Unable to use display");
    while (1);
  }
  

  spectogramDisplay.clearDisplay();
  spectogramDisplay.display();
  currentState = SpectogramSTATES::WELCOME_PAGE;
}

void loop() {
  if (currentState == SpectogramSTATES::WELCOME_PAGE) {
    spectogramDisplay.clearDisplay();
    spectogramDisplay.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0xFFFF);
    drawString(CommonDisplayPOS[0] - 30, CommonDisplayPOS[1] - 8, DisplaySTRS[0], 0, 0);
    drawString(CommonDisplayPOS[0] - 42, CommonDisplayPOS[1], DisplaySTRS[1], 0, 0);
    drawString(CommonDisplayPOS[0] - 24, CommonDisplayPOS[1] + 8, DisplaySTRS[2], 0, 0);
    spectogramDisplay.display();
    delay(INTRO_DELAY);
    spectogramDisplay.clearDisplay();
    spectogramDisplay.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0xFFFF);
    drawString(CommonDisplayPOS[2] - 12, CommonDisplayPOS[3], DisplaySTRS[3], 0, 0);
    drawString(CommonDisplayPOS[0] - 9, CommonDisplayPOS[3], DisplaySTRS[4], 0, 0);
    drawString(((5 * SCREEN_WIDTH) / 6) - 18, CommonDisplayPOS[3], DisplaySTRS[5], 0, 0);
    currentState = SpectogramSTATES::MAIN_PAGE;
  }
  else if (currentState == SpectogramSTATES::MAIN_PAGE) {
    spectogramDisplay.fillRect(0, 0, SCREEN_WIDTH, CommonDisplayPOS[3], 0xFFFF);

    drawBar(CommonDisplayPOS[2] - 6, CommonDisplayPOS[3] - 5, BasePIN);
    drawBar(CommonDisplayPOS[0] - 4, CommonDisplayPOS[3] - 5, MidPIN);
    drawBar(((5 * SCREEN_WIDTH) / 6) - 9, CommonDisplayPOS[3] - 5, TreblePIN);
    spectogramDisplay.display();
    delay(REFRESH_DELAY);
  }
  else {
    //unknown state, reset
    currentState = SpectogramSTATES::WELCOME_PAGE;
  }
}

void drawString(int16_t x, int16_t y, const char* string, uint16_t col, uint16_t bg) {
  unsigned char c = *string;
  unsigned int counter = 0;
  while (c != 0) {
    spectogramDisplay.drawChar(x + counter, y, c, col, bg, 1);
    c = *(++string);
    counter = counter + 6;
  }
}

void drawBar(int16_t x, int16_t y, const int pin) {
  int pinValue = analogRead(pin);
  int height_pos = map(pinValue, 0, 1023, y, (SCREEN_HEIGHT / 4));
  int height = (y - height_pos);
  spectogramDisplay.fillRect(x, height_pos, (SCREEN_WIDTH / 10), height, 0);
}
