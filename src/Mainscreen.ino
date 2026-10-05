
void buildMainScreen() {
  tft.fillRect(0, 52, 320, 188, TFT_BLACK);
  drawButtons(TFT_NAVY, TFT_BLUE, 3);
  printMainButtonTexts();
  tft.fillCircle(195, 160, 4, TFT_RED);  //AM external modulation
}

//##########################################################################################################################//
void drawButtons(uint16_t color1, uint16_t color2, int rows) {  // draws 3 or 4 rows with 4 buttons each

  int hStart = 8;
  int ySpace = 57;  //57
  int yStart = 60;  //121

  for (int j = 0; j < 4; j++) {

    drawButton(hStart + j * 80, yStart + 2 * ySpace, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    drawButton(hStart + j * 80, yStart + ySpace, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    drawButton(hStart + j * 80, yStart, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);

    if (rows == 4)
      drawButton(hStart + j * 80, yStart, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
  }
}
//##########################################################################################################################//


void drawButton(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color1, uint32_t color2) {  // draws buttons as plain rectangles


  tft.fillRectVGradient(x, y + 4, w, h / 2, color1, color2);
  tft.fillRectVGradient(x, y + (h / 2) + 4, w, h / 2, color2, color1);
  tft.drawRoundRect(x, y + 4, w, h, 3, TFT_LIGHTGREY);
}


//##########################################################################################################################//


void printMainButtonTexts() {


  struct Button {
    int x;
    int y;
    const char* label;
  };



  // button and sprite button sizes
  //TILE_WIDTH 65
  // TILE_HEIGHT 50

  tft.setTextFont(2);
  tft.setTextSize(1);
  Button buttons[] = {
    { 20, 70, "Set" },
    { 20, 90, "Freq" },
    { 105, 70, "Up" },
    { 105, 90, "Down" },
    { 180, 66, "+100Hz" },
    { 180, 94, "-100Hz" },
    { 265, 70, "Output" },
    { 265, 90, "Level" },
    { 15, 147, "Testtone" },
    { 30, 127, "FM" },
    { 95, 147, "Testtone" },
    { 110, 127, "AM" },
    { 175, 140, "AM Mod." },
    { 175, 120, "External" },
    { 260, 137, "Noise" },
    { 20, 193, "MORE" },
    { 105, 184, "Morse" },
    { 105, 204, "String" },
    { 185, 184, "RTTY" },
    { 185, 204, "String" },
    { 265, 194, "Sweep" },


  };


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    tft.setCursor(buttons[i].x, buttons[i].y);

    if (buttons[i].x == 20 && buttons[i].y == 193)
     tft.setTextColor(TFT_GREEN);
    else
      tft.setTextColor(textColor);

    tft.print(buttons[i].label);
  }
}


//##########################################################################################################################//

void readTouchCoordinates() {

  pressed = getFastTouchCoordinates();
  if (showTouchCoordinates) {  // for debug
    tft.fillRect(210, 1, 110, 15, TFT_BLACK);
    tft.setTextColor(textColor);
    tft.setCursor(210, 0);
    tft.printf("x:%d y:%d, %d", tx, ty, buttonID);
  }
}

//##########################################################################################################################//


void getButtonID() {

  //hspacing 80, yspacing 57

  pressed = getFastTouchCoordinates();
  column = 1 + (tx / 80);  // get row and column
  row = 1 + (ty / 60);
  buttonID = row * 10 + column;
}

//##########################################################################################################################//

void readMainBtns() {

  if (!pressed)
    return;
  getButtonID();
  tft.setTextColor(textColor);  // general color for slider and return button text
  switch (buttonID) {
    case 21:
      freqScreen();
      buildMainScreen();
      break;
    case 22:
      buildUpDownScreen();
      break;
    case 23:
      if (ty > 75)
        FREQ -= 100;
      if (ty < 75)
        FREQ += 100;
      delay(200);
      break;
    case 24:
      sliderValue = 127;
      setOutputLevel();
      buildMainScreen();
      tRel();

      break;
    case 31:
      FMModulator();
      tRel();
      si5351.set_freq(FREQ * 100ULL, SI5351_CLK1);
      FREQ_OLD = FREQ - 1;  // retrigger frequency display
      buildMainScreen();
      break;
    case 32:
      AMTone();
      tRel();
      FREQ_OLD = FREQ - 1;  // retrigger frequency display
      buildMainScreen();
      break;
    case 33:
      pin27State = !pin27State;  //external modulation on/off
      if (pin27State) {
        digitalWrite(27, HIGH);
        tft.fillCircle(195, 160, 4, TFT_GREEN);
      }

      else {
        digitalWrite(27, LOW);
        tft.fillCircle(195, 160, 4, TFT_RED);
      }
      tRel();
      break;
    case 34:
      noise();
      tRel();
      FREQ_OLD = FREQ - 1;  // retrigger frequency display
      buildMainScreen();
      break;
    case 41:
      secScreen();
      break;
    case 42:
      morse();
      tRel();
      ESP.restart();
      break;
    case 43:
      playFT2Costas();
      RTTY();
      tRel();
      FREQ_OLD = FREQ - 1;  // retrigger frequency display
      buildMainScreen();
      break;
    case 44:
      sweepGenerator();  // 1 for sweepmode
      tRel();
      FREQ_OLD = FREQ - 1;
      buildMainScreen();
      break;
    default:
      return;
  }
}

//##########################################################################################################################//


void setOutputLevel() {

  tft.fillRect(0, 53, 320, 267, TFT_BLACK);
  drawButton(0, 180, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
  tft.setCursor(10, 200);
  tft.print("RETURN");

  tft.drawRect(SLIDER_X - 2, SLIDER_Y - 2, SLIDER_WIDTH + 4, SLIDER_HEIGHT + 4, TFT_WHITE);
  drawSlider(sliderValue);

  while (true) {
    if (!updateSlider())
      return;
    dac1.outputVoltage((uint8_t)(int)(sliderValue * 0.66));  // reduce output voltage since mosfet is already full open at 2.5V
  }
}

//##########################################################################################################################//


static int oldValue = -1;

void drawSlider(int value) {


  if (value == oldValue) return;

  int oldWidth = map(oldValue, 0, 255, 0, SLIDER_WIDTH);
  int newWidth = map(value, 0, 255, 0, SLIDER_WIDTH);

  if (value > oldValue) {
    tft.fillRect(SLIDER_X + oldWidth, SLIDER_Y, newWidth - oldWidth, SLIDER_HEIGHT, TFT_BLUE);
  } else {

    tft.fillRect(SLIDER_X + newWidth, SLIDER_Y, oldWidth - newWidth, SLIDER_HEIGHT, TFT_BLACK);
  }

  oldValue = value;
}

//##########################################################################################################################//

bool updateSlider() {
  static uint16_t oldtx = 0;

  pressed = getFastTouchCoordinates();


  if (pressed && (tx != oldtx)) {
    if (pressed && ty > 185 && tx < 60) {  // Return pressed
      oldValue = -1;
      return false;
    }

    // Get touch coordinates
    if (ty > SLIDER_Y && ty < SLIDER_Y + SLIDER_HEIGHT) {  // Touch inside slider
      sliderValue = map(tx, SLIDER_X, SLIDER_X + SLIDER_WIDTH, 0, 255);
      sliderValue = constrain(sliderValue, 0, 255);
      drawSlider(sliderValue);
      tft.fillRect(SLIDER_X + 200, SLIDER_Y - 20, 60, 14, TFT_BLACK);
      tft.setCursor(SLIDER_X + 200, SLIDER_Y - 20);
      tft.printf("%d", (int)(sliderValue / 2.55));
      oldtx = tx;
    }
  }
  return true;
}



//##########################################################################################################################//
void sweepGenerator() {

  long lowLimit = 0, highLimit = 0, temp = 0, diff = 0, stepSize = 1000;
  int ctr = 0, xOffset = 0;
  long fSave = FREQ;
  drawNumPad();
  drawKeypadButtons();

  displayText(0, 30, 320, 20, "Enter low limit:");
  readKeypadButtons();
  lowLimit = FREQ;
  tft.fillRect(0, 0, 320, 20, TFT_BLACK);
  drawNumPad();
  drawKeypadButtons();
  displayText(0, 30, 320, 20, "Enter high limit:");
  readKeypadButtons();
  highLimit = FREQ;
  tft.fillRect(0, 0, 320, 20, TFT_BLACK);
  drawNumPad();
  drawKeypadButtons();
  displayText(0, 30, 320, 20, "Enter Step Size:");
  readKeypadButtons();
  stepSize = FREQ;


  if (lowLimit > highLimit) {  // invert order if needed
    long temp = lowLimit;
    lowLimit = highLimit;
    highLimit = temp;
  }
  long sweepFreq = lowLimit;
  diff = highLimit - lowLimit;
  long seg = diff / 255;

  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setCursor(SLIDER_X + 200, SLIDER_Y - 20);
  tft.printf("%d", (int)(sliderValue / 2.55));
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.print("Running... touch Return to stop");
  tft.setCursor(SLIDER_X, SLIDER_Y - 20);
  tft.print("Delay:");
  drawButton(0, 180, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
  tft.setCursor(10, 200);
  tft.print("RETURN");
  tft.drawRect(SLIDER_X - 2, SLIDER_Y - 2, SLIDER_WIDTH + 4, SLIDER_HEIGHT + 4, TFT_WHITE);
  sliderValue = 0;
  drawSlider(sliderValue);
  si5351.set_freq((lowLimit + diff / 2) * 100ULL, SI5351_CLK1);

  tft.setCursor(0, 12);
  tft.printf("Sweep step = %ld KHz", stepSize / 1000);
  while (true) {

    settleSynth();
    si5351.set_freq((sweepFreq)*100ULL, SI5351_CLK1);
    if (sweepFreq >= lowLimit + temp) {
      temp += seg;
      xOffset++;
      tft.fillRect(SLIDER_X + xOffset, SLIDER_Y - 50, 1, 10, TFT_BLUE);
    }

    ctr++;
    if (ctr == 10) {  // call slow slider function only 1/10 times is enough
      ctr = 0;

      if (!updateSlider()) {
        FREQ = fSave;
        return;
      }
    }
    delayMicroseconds(sliderValue * 100);  // adjustable delay

    sweepFreq += stepSize;

    if (sweepFreq >= highLimit) {
      temp = 0;
      xOffset = 0;
      tft.fillRect(SLIDER_X, SLIDER_Y - 50, SLIDER_WIDTH + 1, 10, TFT_BLACK);
      sweepFreq = lowLimit;
    }
  }
}

//##########################################################################################################################//
void displayText(int x, int y, int length, int height, const char* text) {  // helper to display text
  tft.fillRect(x, y, length, height, TFT_BLACK);
  tft.setCursor(x, y);
  tft.print(text);
}
//##########################################################################################################################//

bool AMTone() {  // AM sine tone generator, roughly btw 0 and 4KHz
  static int j = 0;
  static int oldSliderValue = 0;
  sliderValue = 127;
  tft.fillScreen(TFT_BLACK);
  pin27State = false;
  digitalWrite(27, LOW);
  tft.setCursor(0, 60);
  drawButton(0, 180, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
  tft.setCursor(10, 200);
  tft.print("RETURN");
  displayText(0, 0, 320, 20, "AM Sine Tone Generator");
  tft.drawRect(SLIDER_X - 2, SLIDER_Y - 2, SLIDER_WIDTH + 4, SLIDER_HEIGHT + 4, TFT_WHITE);
  drawSlider(sliderValue);
  dac2.enable();

  while (true) {


    if (oldSliderValue != sliderValue) {
      dac2.outputCW(sliderValue * 15);  // max 4kHz sinus signal on pin 25
      tft.fillRect(30, 80, 100, 20, TFT_BLACK);
      tft.setCursor(30, 80);
      tft.printf("Freq: %dHz", sliderValue * 16);
      oldSliderValue = sliderValue;
    }

    // Slider update check (every 1000 iterations)
    if (++j >= 1000) {
      j = 0;
      if (!updateSlider()) {
        dac2.disable();
        sliderValue = 127;
        return true;
      }
    }
  }
  /*

while (true) {
uint32_t start = micros();

dac2.outputCW(1500);
delay(20);
dac2.outputCW(2300);

while(micros() - start < 500000);
}
*/
}

//##########################################################################################################################//
