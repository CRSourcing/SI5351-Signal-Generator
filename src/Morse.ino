int dotDelay = 35;

// Morse code for letters
const char* const letters[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",   
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",  
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."          
};

// Morse code for numbers
const char* const numbers[] = {
  "-----", ".----", "..---", "...--", "....-", ".....",
  "-....", "--...", "---..", "----."
};

// Morse encoding function
void morse() {

    tft.fillScreen(TFT_BLACK);
    drawButton(0, 180, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
   tft.setCursor(10, 200);
    tft.print("RETURN");
    displayText(0, 30, 320, 20, "Move slider to change speed");
    sliderValue = 127;
     drawSlider(sliderValue);

    while (true) {
     
        

        // Encode "TEST123"
        flashSequence(letters[19]);  // T
        flashSequence(letters[4]);   // E
        flashSequence(letters[18]);  // S
        flashSequence(letters[19]);  // T
        flashSequence(numbers[1]);
        flashSequence(numbers[2]);
        flashSequence(numbers[3]);

        delay(300);

        // Encode A-Z
        for (int i = 0; i < 26; i++)  
            
            
           if (flashSequence(letters[i]))
              return;
         
        
        delay(300);
   
    }

}

//##########################################################################################################################//
// Send Morse code sequence
bool flashSequence(const char* sequence) {
    int i = 0;
    while (sequence[i] != '\0') {  //  end of tring
        
              if (!updateSlider())
          return 1;

        dotDelay = (255 - sliderValue) / 4 + 15; 
        
        flashDotOrDash(sequence[i]);
        i++;
    }
    delay(dotDelay * 3);

     return 0;
}


//##########################################################################################################################//
void flashDotOrDash(char dotOrDash) {
    si5351.output_enable(SI5351_CLK1, 1);
    
    if (dotOrDash == '.') {

        delay(dotDelay);  // Short pulse for dot
    
    } else {
        delay(dotDelay * 3);  // Long pulse for dash
      
    }
    
    si5351.output_enable(SI5351_CLK1, 0);
    delay(dotDelay);
}
