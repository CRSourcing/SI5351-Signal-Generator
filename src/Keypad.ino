
//draws and reads onscreen numeric keyboard

void freqScreen() {  
  tft.fillRect(0, 52, 320, 188, TFT_BLACK);
  drawNumPad();
  drawKeypadButtons();
  readKeypadButtons();
}

//##########################################################################################################################//
void drawKeypadButtons() {

  tft.setTextFont(2);
  tft.setTextSize(1);

  tft.setTextColor(textColor);
  char labels[9] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

  int x_positions[] = { 28, 108, 188 };
  int y_positions[] = { 65, 114, 165 };

  int index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      tft.setCursor(x_positions[j], y_positions[i]);
      tft.print(labels[index++]);
    }
  }

  tft.setCursor(28, 210);
  tft.print("0");

  struct TextPos {
    int x, y;
    const char* text;
  };

  TextPos textButtons[] = {
    { 191, 210, "." },
    { 96, 210, "Reset" },
    { 260, 65, "KHz" },
    { 260, 114, "MHz" },
    { 260, 165, "<<" },
    { 250, 210, "Return" }
  };

  tft.setTextColor(textColor);
  for (int i = 0; i < sizeof(textButtons) / sizeof(textButtons[0]); ++i) {
    tft.setCursor(textButtons[i].x, textButtons[i].y);
    tft.print(textButtons[i].text);
  }
}

//##########################################################################################################################//

bool readKeypadButtons() {

  double f = 0;
  uint16_t xPos = 0;
  uint16_t yPos = 0;
  uint16_t index = 0;
  bool decimalPoint = false;
  int decimalPosition = 0;
  double val = 0;
  long freqSave = FREQ;

  tx = ty = pressed = 0;

  tRel();
  while (index < 11) {
    tPress();


    //hspacing 80, yspacing 57

    column = 1 + (tx / 80);  // get row and column
    row = 1 + ((ty - 25) / 60);
    buttonID = row * 10 + column;

    tft.setTextFont(7);
    tft.setTextSize(0);


    if (column < 4) {                        // must be a digit
      if (row == 1) val = column;            // 1-3
      if (row == 2) val = column + 3;        // 4-6
      if (row == 3) val = column + 6;        //7-9
      if (row == 4 && column == 1) val = 0;  // 0
    }
    if (row == 4 && column == 3) {  // Decimal point
      if (!decimalPoint) {
        decimalPoint = true;
      }
    }

    if (!decimalPoint) {
      f *= 10;   // multiply last value
      f += val;  // add button pressed
    }

    else {
      f += val * pow(10, -decimalPosition);  // add fraction
      decimalPosition++;
    }

    if (row == 1 && column == 4) {  // Enter Khz
      tft.fillRect(0, 0, 320, 51, TFT_BLACK);
      if (!decimalPoint)
        FREQ = (long)(f * 100);
      else
        FREQ = (long)(f * 1000);

      if (FREQ > HI_FREQ) {  // limit max input
        FREQ = HI_FREQ;
        keyPadErr();
      }
      tRel();
      settleSynth();
      return true;
    }


    if (row == 2 && column == 4) {  //enter Mhz
      tft.fillRect(0, 0, 320, 51, TFT_BLACK);
      if (f > HI_FREQ / 1000)
        f /= 1000;  // Must be KHz, probably pressed MHz by user error
      if (!decimalPoint)
        FREQ = (f * 100000);
      else
        FREQ = (f * 1000000);


      if (FREQ < LOW_FREQ) {  // limit max input
        FREQ = freqSave;
        keyPadErr();
      }
      tRel();
      settleSynth();
      return true;
    }

    if (row == 3 && column == 4) {  // <<

      if (!decimalPoint) {
        f = (long)(f / 100);  // divide by 100 because it will get multiplied by 10 agai
        index--;
      } else {
        String fString = String(f, 3);
        fString = fString.substring(0, fString.indexOf('.'));  //eliminates digits after decimal point
        f = fString.toFloat();
        decimalPoint = false;
        decimalPosition = 0;
        index -= 4;
      }
      

      tft.fillRect(xPos, 0, 320, 50, TFT_BLACK);
      tft.setCursor(xPos, yPos);
      tft.printf("%3.6f", f);
    }




    if (row == 4 && column == 2) {  // restart ESP if pressed

      ESP.restart();
    }


    if (row == 4 && column == 4) {  // Return
      keyVal = (long)f / 10;        // keyVal is used as input for other functions
      displayFREQ(0);
      tft.fillRect(0, 0, 320, 51, TFT_BLACK);  // overwrite frequency window
      FREQ = freqSave;
      FREQ_OLD = FREQ - 1;
      displayFREQ(FREQ);
      tRel();
      return false;
    }

    if (row > 4 || column > 4)
      return false;  // outside of keypad area

    tft.fillRect(0, 0, 320, 51, TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(xPos, yPos);
    if (!decimalPoint)
      tft.printf("%ld", (long)f);
    else
      tft.printf("%3.6f", f);
    index++;

    tRel();
    val = 0;
  }

  FREQ = freqSave;  // too many digits
  keyPadErr();
  return false;
}


//##########################################################################################################################//


void keyPadErr() {

  tx = ty = pressed = 0;
  FREQ_OLD -= 1;  // no valid result
  tft.fillRect(0, 0, 330, 50, TFT_BLACK);
  tft.setCursor(10, 3);
  tft.print("Invalid entry");
  delay(500);
  tft.fillRect(0, 0, 330, 50, TFT_BLACK);
}



//##########################################################################################################################//
void drawNumPad() {
  uint16_t yb = 50;
  int h = 8;
  tft.fillScreen(TFT_BLACK);

  for (int i = 1; i < 5; i++) {
    for (int j = 0; j < 4; j++) {
      drawButton(h + j * 78, i * yb, 54, 35, TFT_BTNBDR, TFT_BTNCTR);
    }
  }
}

//##########################################################################################################################//

// Touch functions

void tRel() {  // wait for touch release

  do {
    pressed = getFastTouchCoordinates();
    delay(50);
  } while (pressed);
}


//##########################################################################################################################//

void tPress() {  // wait for press
  pressed = false;
  do {
    pressed = getFastTouchCoordinates();
    delay(50);
  } while (!pressed);
}

//##########################################################################################################################//


void tDoublePress() {  // wait until pressed again
  do {
     pressed = getFastTouchCoordinates();
    delay(20);
  } while (pressed);
  delay(20);

  do {
     pressed = getFastTouchCoordinates();
    delay(20);
  } while (!pressed);
}

//##########################################################################################################################//


bool getFastTouchCoordinates() {

  /*
replaces tft.getTouch(&tx, &ty),much faster, but may cause spurious errors. Depends on the particular display
 */

  uint16_t x = 0, y = 0;
  uint16_t z = tft.getTouchRawZ();
  if (z > 200) { // z equals pressure
    pressed = true;
    tft.getTouchRaw(&x, &y);   // Read raw x and y
    tft.convertRawXY(&x, &y);  // Convert to screen coordinates
                            
    if (y >= DISP_HEIGHT || x >= DISP_WIDTH) { //error
      pressed = false;
      tx = 0;
      ty = 0;
      return pressed;
    }

    tx = x;
    ty = y;
    return pressed;
  }

  else {
    pressed = false;
    tx = 0;
    ty = 0;
    return pressed;
  }
}