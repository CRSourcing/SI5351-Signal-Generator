

void buildUpDownScreen() {
  tft.fillRect(0, 52, 320, 188, TFT_BLACK);
  drawButtons(TFT_NAVY, TFT_BLUE, 3);
  tft.fillRect(75, 115, 252, 58, TFT_BLACK);
  printUpDownText();
  readUpDownBtns();
  tRel();
  tx = ty = pressed = 0;
  buildMainScreen(); 
  }

//##########################################################################################################################//



void printUpDownText() {

  
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
    
    { 20, 137, "Return" },
    { 20, 76, "+1MHz" },
    { 95, 76, "+100KHz" },
    { 180, 76, "+10KHz" },
    { 265, 80, "+1KHz" },
    { 25, 190, "-1MHz" },
    { 95, 194, "-100KHz" },
    { 185, 194, "-10KHz" },
    { 265, 194, "-1KHz" },


  };


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    tft.setCursor(buttons[i].x, buttons[i].y);
    tft.setTextColor(textColor);
    tft.print(buttons[i].label);
  }



}


//##########################################################################################################################//



void readUpDownBtns() {
   buttonID = 0;  
   
  
  while (buttonID != 31) {
  getButtonID();
  tft.setTextColor(textColor); // general color for slider and return button text
  switch (buttonID) {
    case 21:
      FREQ += 1000000;
      break;
    case 22:
     FREQ += 100000;
      break;
    case 23:
    FREQ += 10000;
      break;
    case 24:
   FREQ += 1000;
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
       FREQ -= 1000000;
      break;
    case 42:
     FREQ -= 100000;
      break;
    case 43:
        FREQ -= 10000;
      break;
    case 44:
      FREQ -= 1000;
      break;
  }

   delay(100);
   if(FREQ_OLD != FREQ){
  limitFREQ();
  displayFREQ(FREQ);
  settleSynth();
  si5351.set_freq(FREQ *100ULL, SI5351_CLK1); 
   FREQ_OLD = FREQ;
   }
  }
}
