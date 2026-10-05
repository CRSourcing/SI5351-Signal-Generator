
bool FMModulator() {
    // Primitive FM modulator, using si5351.set_pll to generate a test tone.

    static int oldSliderValue = 0;
    static int i = 0;
    static int j = 0;
    static bool toggle = false;
    uint64_t modulationDepth = 0;

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 60);
    drawButton(0, 180, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    tft.setCursor(10, 200);
    tft.print("RETURN");

    displayText(0, 20, 320, 20, "250Hz sine test tone only!");
    displayText(0, 40, 320, 20, "Move slider to change mod. depth (max +-50KHz)");
    tft.drawRect(SLIDER_X - 2, SLIDER_Y - 2, SLIDER_WIDTH + 4, SLIDER_HEIGHT + 4, TFT_WHITE);
    sliderValue = 10;
    drawSlider(sliderValue);

  
    const uint64_t deviationScaler = 80000ULL / (FREQ/1000000); // need to scale deviation, otherwise would depend on CLK value

    // Sine half-wave lookup table
    static const uint8_t sine_halfwave_10[10] = {
        0, 31, 59, 81, 95, 100, 95, 81, 59, 31
    };


    si5351.update_status();
    const uint64_t pllAStartFreq = si5351.plla_freq;
    
    tft.setCursor(0, 0);
    tft.printf("PLL: %lld MHz", pllAStartFreq/100000000ULL);
    


    while (true) {
        // Modulation calculation
        uint64_t modulation = modulationDepth * sine_halfwave_10[i];
        uint64_t freq = toggle ? (pllAStartFreq + modulation) : (pllAStartFreq - modulation);
        si5351.set_pll(freq, SI5351_PLLA);
     
  
        i = (i + 1) % 10;
        if (i == 0) toggle = !toggle;

        // Slider update check (every 1000 iterations)
        if (++j >= 1000) {
            j = 0;
            i = 0;
            modulationDepth = deviationScaler * sliderValue; 
             if (!updateSlider()){
              si5351.set_pll(pllAStartFreq,  SI5351_PLLA);   
              return true;
            }
            if (oldSliderValue != sliderValue || sliderValue == 0) {
                si5351.set_pll(pllAStartFreq, SI5351_PLLA);
                oldSliderValue = sliderValue;
            }
        }
    }
}


//##########################################################################################################################//
bool noise() {
    // Basic noise generator. Depending on the initial value of si5351.plla_freq, the noise may not be evenly distributed. 
    static int j = 0;
     bool toggle = false;

    uint64_t rand =0;

    tft.fillScreen(TFT_BLACK);
    displayText(0, 0, 320, 20, "Noise generator sets PLL to ~750MHz for even"); 
    displayText(0, 20, 320, 20, "noise distribution.");
   tft.drawRect(SLIDER_X - 2, SLIDER_Y - 2, SLIDER_WIDTH + 4, SLIDER_HEIGHT + 4, TFT_WHITE);


    if (FREQ < 100000000) // Fractional divider may fail >100MHz
      setOptimizedFrequency((uint64_t)FREQ);  
    
    si5351.update_status();
    delay(50);

    const uint64_t pllAStartFreq = si5351.plla_freq;
    tft.setCursor(0, 40);
    tft.printf("PLL: %lldKHz", pllAStartFreq/100000ULL);

    drawButton(0, 180, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    tft.setCursor(10, 200);
    tft.print("RETURN");
    sliderValue = 10;
    drawSlider(sliderValue);


    while (true) {

        rand = random(50000000); // 
       
        if (toggle)
        si5351.set_pll(pllAStartFreq + rand * sliderValue, SI5351_PLLA); 
        else 
        si5351.set_pll(pllAStartFreq - rand *  sliderValue, SI5351_PLLA); 
        
        toggle = !toggle;
        
        
        // Slider update check (every 1000 iterations)
        if (++j >= 1000) {
            j = 0;
              //si5351.set_pll(pllAStartFreq,  SI5351_PLLA);
            
            if (!updateSlider()){
              si5351.set_pll(pllAStartFreq,  SI5351_PLLA);   
              return true;
            }
        

        }
    }


}

//##########################################################################################################################//


#define PLL_TARGET  75000000000ULL  // 750 MHz target
#define PLL_RANGE    5000000000ULL   // ±50 MHz range
#define MIN_MS_DIV  8.0           // Minimum multisynth divider
#define MAX_MS_DIV  900.0         // Maximum multisynth divider

// Set SI5351 to closest valid frequency allowed by PLL and multisynth constraints
void setOptimizedFrequency(uint64_t freq_hz) {
    uint64_t best_pll_freq = 0;
    uint32_t best_div = 0;
    double best_error = INFINITY;

    // Valid PLL range (750MHz ±50MHz)
    const uint64_t min_pll = PLL_TARGET - PLL_RANGE;
    const uint64_t max_pll = PLL_TARGET + PLL_RANGE;

    // Find optimal PLL and divider combination
    for (uint32_t div = 4; div <= 900; div++) {
        // Skip invalid integer dividers
        if (div == 5 || div == 7 || (div > 8 && div < 900)) continue;

        uint64_t pll_freq = freq_hz * div;
        
        // Check if PLL frequency is within valid range
        if (pll_freq >= min_pll && pll_freq <= max_pll) {
            double error = fabs((double)pll_freq - (double)PLL_TARGET);
            
            // Track the configuration with smallest error
            if (error < best_error) {
                best_error = error;
                best_pll_freq = pll_freq;
                best_div = div;
            }
        }
    }

    // If no exact match found use closest valid configuration
    if (best_div == 0) {
        // Try fractional dividers between 8 and 2048
        double best_frac_error = INFINITY;
        double best_frac_div = 0.0;
        
        // Sample fractional dividers 
        for (double frac_div = 8.0; frac_div <= 2048.0; frac_div += 0.01) {
            uint64_t pll_freq = (uint64_t)(freq_hz * frac_div);
            
            if (pll_freq >= min_pll && pll_freq <= max_pll) {
                double error = fabs((double)pll_freq - (double)PLL_TARGET);
                if (error < best_frac_error) {
                    best_frac_error = error;
                    best_pll_freq = pll_freq;
                    best_frac_div = frac_div;
                }
            }
        }
        
        if (best_frac_div != 0.0) {
              tft.setCursor(0,60);
              tft.printf("Fractional diver set to: %.2f", best_frac_div);
              //tft.setCursor(0,70);
              //tft.printf("freq_hz%lld, best_pll_freq%lld", freq_hz * 100ULL, best_pll_freq);
            si5351.set_freq_manual(freq_hz * 100ULL, best_pll_freq, SI5351_CLK1);
            return;
        }
    }

    // Set the best found configuration
    if (best_div != 0) {
        si5351.set_freq_manual(freq_hz * 100ULL, best_pll_freq, SI5351_CLK1);
         tft.setCursor(0,60);
        tft.printf("Integer division set to: %ld", best_div);
    
    } 
    
    
    else {
        // Worst case, nutng found, use si5351.set_freq()
        si5351.set_freq(freq_hz * 100ULL, SI5351_CLK1);
        tft.setCursor(0,60);
        tft.print("setOptimizedFrequency(): No match");
    }
}

