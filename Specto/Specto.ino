#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif



#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_I2C_ADDR 0x3C
#define INTRO_DELAY 3000
#define REFRESH_DELAY 25
#define SAMPLING_FREQ 38461.5385
#define SAMPLES 64


enum class SpectogramSTATES : uint8_t {
  WELCOME_PAGE,
  MAIN_PAGE
};

const char* DisplaySTRS[] = {"Home Audio", "Jacob and Kofi", "ECE 2804", "Bass", "Mid", "Treble"};
Adafruit_SSD1306 spectogramDisplay(SCREEN_WIDTH, SCREEN_HEIGHT);
SpectogramSTATES currentState;

const int AudioPIN     = A0;
//const int MidPIN      = A1;
//const int TreblePIN   = A2


const int CommonDisplayPOS[] = {(SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), (SCREEN_WIDTH / 6), (2 * SCREEN_HEIGHT) / 3};
void setup() {
  Serial.begin(115200);
  if (!spectogramDisplay.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println("Unable to use display");
    while (1);
  }
  // set prescale to 16
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0) ;
    
  spectogramDisplay.clearDisplay();
  spectogramDisplay.display();
  currentState = SpectogramSTATES::WELCOME_PAGE;
}

void loop() {
  double data[SAMPLES] = {0.0};
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
    //currentStamp = micros() / 1000000.0;
  }
  else if (currentState == SpectogramSTATES::MAIN_PAGE) {
    
    //Serial.print("Index: ");
    //Serial.println(index);

    for (int i = 0;i < SAMPLES; i++) {
      data[i] = (((double)analogRead(AudioPIN)) * (5.0 / 1023.0));
    }

    double rB = 0.0, iB = 0.0;
    double rM = 0.0, iM = 0.0;
    double rT = 0.0, iT = 0.0;

    calculateFourierSeries(data, rB, iB, SAMPLES, 1);
    calculateFourierSeries(data, rM, iM, SAMPLES, 4);
    calculateFourierSeries(data, rT, iT, SAMPLES, 18);


    double bassMag    = sqrt((rB * rB) + (iB * iB));
    double midMag    = sqrt((rM * rM) + (iM * iM));
    double trebleMag = sqrt((rT * rT) + (iT * iT));

    
    Serial.print("Bass: ");
    Serial.println((int)bassMag);
    Serial.print("Mid: ");
    Serial.println((int)midMag);
    Serial.print("Treble: ");
    Serial.println((int)trebleMag);

    drawBar(CommonDisplayPOS[2] - 6, CommonDisplayPOS[3] - 5, bassMag);
    drawBar(CommonDisplayPOS[0] - 4, CommonDisplayPOS[3] - 5, midMag);
    drawBar(((5 * SCREEN_WIDTH) / 6) - 9, CommonDisplayPOS[3] - 5, trebleMag);
    spectogramDisplay.fillRect(0, 0, SCREEN_WIDTH, CommonDisplayPOS[3], 0xFFFF);
    spectogramDisplay.display();
    delay(REFRESH_DELAY);
  }
  else {
    //unknown state, reset
    currentState = SpectogramSTATES::WELCOME_PAGE;
  }
  
}


double calculateFourierSeries(double data[], double& real, double& imag, int samples, int k)
{
  double m = 2.0 * PI * (1.0 / samples);
  
  double r = 0.0;
  double i = 0.0;
  double w = (2.0  * PI * (double)k) / (double)samples;
  for (int n = 0; n < samples; n++)
  {
    real = real + data[n] * cos(w * n);
    imag = imag - data[n] * sin(w * n);
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


void drawBar(int16_t x, int16_t y, double mag) {
  int y_pos = 0;
  int height = 0;
  int midHeight = ((y - (SCREEN_HEIGHT / 4)) / 2);
  int midPos = (SCREEN_HEIGHT / 4) + midHeight;
  if (0.0 > mag > 4.5)
  {
    //y_pos = midPos;
    //height = map((long)map, 0, 5, 0, midHeight);
  }
  else if (4.5 > mag > 8.5)
  {
    y_pos = midPos;
    height = 0;
  }
  else
  {
    y_pos = map((long)map, 9, 20, midPos, SCREEN_HEIGHT / 4);
    height = y_pos - y;
  }
  /*
  int height_pos = map(pinValue, 0, 1023, y, (SCREEN_HEIGHT / 4));
  int height = (y - height_pos);*/
  spectogramDisplay.fillRect(x, y_pos, (SCREEN_WIDTH / 10), height, 0);
}
