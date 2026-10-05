
void SI5351_Init() {


    bool i2c_found;
  for (int i = 0; i < 10; i++) {
     i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    if (!i2c_found) {
      tft.setCursor(10 + 10 * i, 60);
      tft.print(".");
      delay(100);
    }
  }

  if (!i2c_found) {
    tft.print(" SI5351 not found!\n");
    delay(1000);
    return;
  }

 else
  si5351.set_correction(correction, SI5351_PLL_INPUT_XO);  
  si5351.set_ms_source(SI5351_CLK1, SI5351_PLLA);                               
  si5351.set_ms_source(SI5351_CLK0, SI5351_PLLB);  
}


//##########################################################################################################################//

void settleSynth() {  
  //This may or may not be needed. My SI5351 can't switch FREQ from one extreme to the other reliably in one step, the pll falls out of lock so a short time at 110MHz is needed

    if (FREQ >= 110000000 && FREQ_OLD < 110000000)  {
      si5351.set_freq(110000000 * 100ULL, SI5351_CLK1);
      delay(20);   

    }

    if (FREQ <= 110000000 && FREQ_OLD > 110000000)  {
       si5351.set_freq(110000000 * 100ULL, SI5351_CLK1);
       delay(20);   

    }



}



//##########################################################################################################################//

void saveCurrentFreq() {  // saves current FREQ after 1 minute of inactivity

  static uint32_t start;
  static bool write = false, written = false;
  const uint32_t writeDelay = 60000;  // write freq after 1 minute to EEPROM

  if (FREQ == FREQ_OLD && write == false) {
    start = millis();
    write = true;
  }

  if (write && (millis() >= start + writeDelay) && (!written) && (FREQ == FREQ_OLD)) {
    written = true;
    preferences.putLong("lastFreq", FREQ);
  }

  if (FREQ != FREQ_OLD) {
    write = false;
    written = false;
  }
}
//##########################################################################################################################//
