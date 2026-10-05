


void secScreen() {
  buildSecScreen();
  readSecBtns();
}

//##########################################################################################################################//
void buildSecScreen() {
  tft.fillRect(0, 52, 320, 188, TFT_BLACK);
  drawButtons(TFT_NAVY, TFT_BLUE, 3);
  printSecButtonTexts();
}


//##########################################################################################################################//

void printSecButtonTexts() {


  struct Button {
    int x;
    int y;
    const char* label;
  };




  tft.setTextFont(2);
  tft.setTextSize(1);
  Button buttons[] = {
    { 20, 70, "Touch" },
    { 20, 90, "Calibr." },
    { 100, 70, "WiFi" },
    { 95, 90, "Uploader" },
    { 176, 70, ".wav file" },
    { 170, 90, "Modulator" },
    { 265, 70, "4" },
    { 265, 90, "4" },
    { 15, 147, "5" },
    { 15, 127, "5" },
    { 95, 147, "6" },
    { 95, 127, "6" },
    { 175, 127, "7" },
    { 175, 147, "7" },
    { 260, 137, "8" },
    { 20, 193, "Return" },
    { 100, 184, "SSTV" },
    { 100, 204, "Martin1" },
    { 180, 184, "FT2" },
    { 180, 204, "COSTAS" },
    { 265, 194, "12" },


  };


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    tft.setCursor(buttons[i].x, buttons[i].y);
    tft.setTextColor(textColor);
    tft.print(buttons[i].label);
  }
}



//##########################################################################################################################//

void readSecBtns() {

  tRel();
  tPress();
  getButtonID();
  tft.setTextColor(textColor);  // general color for slider and return button text
  tRel();
  tx = 0;
  ty = 0;
  pressed = false;
  switch (buttonID) {
    case 21:
      touchCal();
      ESP.restart();
      break;
    case 22:
      startUploader();
      runUpLoader();
      break;
    case 23:
      startPlayISR();
      playWavFile();
     ESP.restart();
      break;
    case 24:
      break;
    case 31:
      break;
    case 32:
      break;
    case 33:
      break;
    case 34:
      break;
    case 41:
      buildMainScreen();
      return;
      break;
    case 42:
      SSTV();
      tRel();
      ESP.restart();
      break;
    case 43:
      playFT2Costas();
      break;
    case 44:
      break;
    default:
      return;
      tx = 0;
      ty = 0;
      pressed = false;
      buildMainScreen();
  }
}

//##########################################################################################################################//


// FT2 Costas signal test
// modulate generates the tone
// for the specified duration.

const float baseFreq = 1000.0f;
const float toneSpacing = 1000.0f / 24.0f;  // 41.667 Hz
const uint32_t symbolUs = 24000;

const uint8_t costas[4][4] = {
  { 0, 1, 3, 2 },
  { 1, 0, 2, 3 },
  { 2, 3, 1, 0 },
  { 3, 2, 0, 1 }
};

void playFT2Costas() {


  si5351.set_freq(14080000 * 100ULL, SI5351_CLK1);
  pllAStartFreq = si5351.plla_freq;
  mult = pllAStartFreq / FREQ;

  si5351.set_pll(pllAStartFreq, SI5351_PLLA);


  while (true) {
    uint32_t start = millis();


    for (int block = 0; block < 4; block++) {
      for (int i = 0; i < 4; i++) {
        float freq = baseFreq + costas[block][i] * toneSpacing;
        modulateFT2(freq, symbolUs);
      }

      if (block < 3) {
        modulateFT2(0, symbolUs);
        delayMicroseconds(29 * symbolUs);
      }
    }

  
    while (millis() - start < 2520) {
      // Wait for the nominal 2.52-second frame period.
      modulateFT2(0, symbolUs);
    }

    while (millis() - start < 3750) {
      // Wait for the full timeslot.
    }
  }
}

//##########################################################################################################################//

void modulateFT2(uint16_t freq, uint32_t duration) {

  //  setTone(freq);


  si5351.set_pll(pllAStartFreq + (long)freq * mult, SI5351_PLLA);

  delayMicroseconds(duration);
}
