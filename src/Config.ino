//##########################################################################################################################//



void touchCal() {

  tft.fillScreen(TFT_BLACK);               // Clear the screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Set text color to white with black background
  tft.setTextSize(1);

  uint16_t calData[5];

  tft.setCursor(20, 15);

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");


      tft.calibrateTouch(calData, TFT_WHITE, TFT_RED, 15);
  
  tft.printf("\nCal Data: %d, %d, %d, %d, %d\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
    
  

  //write to flash
  preferences.putInt("cal0", calData[0]);
  preferences.putInt("cal1", calData[1]);
  preferences.putInt("cal2", calData[2]);
  preferences.putInt("cal3", calData[3]);
  preferences.putInt("cal4", calData[4]);

  // Apply calibration data
  tft.setTouch(calData);

tft.println("Finished. ESP will reboot now.");

delay(500);

}


//##########################################################################################################################//
