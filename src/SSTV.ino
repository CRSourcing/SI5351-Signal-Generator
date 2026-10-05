#include <LittleFS.h>
#include <FS.h>

#define PORCH_FREQ 1500  // Hz
#define BLACK_FREQ 1500  // Hz
#define WHITE_FREQ 2300  // Hz
#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 256
#define VIS_CODE 44  // VIS code for Martin M1 SSTV


int mode = 0; // 0 = carrier frequency mdulated, 1 = AM modulation
File bmpFile;

// RGB buffers for one scanline
uint8_t r_values[IMAGE_WIDTH + 50];
uint8_t g_values[IMAGE_WIDTH + 50];
uint8_t b_values[IMAGE_WIDTH + 50];


int pixelDuration = 252;  // values for modulating the PLL. μs per pixel // theoretically 456, 249 is working, 252 too
int MartinHorSyncPulseDuration = 3800;  //4800 works,should be 6862
int MartinHorSyncPulseFreq = 1200;




// Hardware PWM Configuration
const int pwmPin = 26;        // GPIO25 (DAC1 also available)
const int pwmChannel = 0;     // Use channel 0-15
const int pwmResolution = 10;  // was8-bit resolution (0-255)


void toneSetup() {
  ledc_timer_config_t timer_conf = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 1200,  // Default frequency
    .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&timer_conf);


  ledc_channel_config_t channel_conf = {
    .gpio_num = pwmPin,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = static_cast<ledc_channel_t>(pwmChannel),
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_0,
    .duty = 128,  // 50% duty for clean tone
    .hpoint = 0
  };
  ledc_channel_config(&channel_conf);
}


void setTone(uint32_t freq) {
  // Set frequency (automatically calculats best divider)
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);
}

void SSTV() {

  pllAStartFreq = si5351.plla_freq;
  mult = pllAStartFreq / FREQ;
  tft.fillScreen(TFT_BLACK);
   tft.setCursor(0, 200);
  tft.print("Set frequency to 28.5MHz");
  tft.setCursor(0, 220);
  tft.printf("PLL: %lldMHz multiplier %d", pllAStartFreq / 100000000, mult);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    tft.println("LittleFS Mount Failed");
    return;
  }

  // Open BMP file
  bmpFile = LittleFS.open("/sstvtest.bmp", "r");
  if (!bmpFile) {
    tft.println("Failed to open BMP");
    return;
  }

  si5351.set_freq(28500000 * 100ULL, SI5351_CLK1);
  tft.setCursor(0, 0);
  tft.println("Freq: 28.5MHz, USB, mode: Martin1\n");
  tft.println("SSB modulator will modulate PLL frequency.");
  tft.println("AM modulator will modulate output stage.");
  tft.println("Select modulator. Disconnect to stop.");
  
 drawButton(0, 100, 60, 40, TFT_NAVY, TFT_BLUE);
 drawButton(260, 100, 60, 40, TFT_NAVY, TFT_BLUE); 
 tft.setCursor(20,115);
 tft.print("SSB"); 
 tft.setCursor(280,115);
 tft.print("AM"); 

drawButton(130, 100, 60, 40, TFT_NAVY, TFT_BLUE);

tft.setCursor(140,110);
tft.print("1200Hz AM");
tft.setCursor(140,125);
tft.print("Sync"); 

 tRel();
 tPress();
pressed = getFastTouchCoordinates();



if (tx > 130 && tx < 180) {

while (true) {


dac2.outputCW(1170); // 

delayMicroseconds(4862);

dac2.outputCW(random(2300));

delay(446);


}

}


if(tx < 100)
    mode = 0;
else 
   mode = 1;
   
if (mode) {
      dac2.disable();
      toneSetup();
}

  while (1) {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 225, 320, 15, TFT_BLACK);
  tft.setCursor(0, 225);
  tft.printf("Modulator: %s ", mode ? "AM (Output stage AM modulated)" : "SSB (PLL frequency modulated)");
  transmitSSTV();
  delay(2000);
  }
}

//##########################################################################################################################//

void transmitSSTV() {


si5351.set_pll(pllAStartFreq, SI5351_PLLA);


int gShift = 22; // to converge colors in buffer
int rShift = 11;
int bShift = 0;

if (mode == 0){
gShift = 20;
rShift = 10;
bShift = 2;

}


if (mode == 1) {

pixelDuration = 437;  // set parameters for AM modulator
MartinHorSyncPulseDuration = 6400; 
}


  long startTime = millis();

  modulate(1900, 300000);  // vertical sync
  modulate(1200, 10000);
  modulate(1900, 300000);
  sendVIS();  // VIS





  for (int y = IMAGE_HEIGHT - 1; y >= 0; y--) {  // Read lines bottom-to-top

    if (y < IMAGE_HEIGHT - 16) {

      bmpFile.seek(54 + y * IMAGE_WIDTH * 3);
      for (int x = 0; x < IMAGE_WIDTH; x++) {  // store a line
        r_values[x] = bmpFile.read();
        g_values[x] = bmpFile.read();
        b_values[x] = bmpFile.read();

        uint16_t color = tft.color565(b_values[x], g_values[x], r_values[x]);
        tft.drawPixel(x, IMAGE_HEIGHT - y - 16, color);
      }

    }

    else {  // image starts with 16 grey lines

      for (int x = 0; x < IMAGE_WIDTH; x++) {
        delayMicroseconds(50);
        r_values[x] = 127;
        g_values[x] = 127;
        b_values[x] = 127;
      }
    }

    for (int pass = 0; pass < 3; pass++) {


      if (pass == 0)
        modulate(MartinHorSyncPulseFreq, MartinHorSyncPulseDuration);  // insert sync pulse
      else
        modulate(1500, 572);  // Between each color component, there are short gaps of 1500 Hz lasting 0.572 ms.


      for (int x = 0; x < IMAGE_WIDTH; x++) {  // pixel

        if (pass == 0) {
          uint8_t g = g_values[x + gShift];
          modfreq = map(g, 0, 255, BLACK_FREQ, WHITE_FREQ);
          modulate(modfreq, pixelDuration);
        }

        if (pass == 1) {
          uint8_t r = r_values[x + rShift];
          modfreq = map(r, 0, 255, BLACK_FREQ, WHITE_FREQ);
          modulate(modfreq, pixelDuration);
        }

        if (pass == 2) {
          uint8_t b = b_values[x + bShift];
          modfreq = map(b, 0, 255, BLACK_FREQ, WHITE_FREQ);
          modulate(modfreq, pixelDuration);
        }
      }
    }
  }


  tft.fillRect(0, 225, 320, 15, TFT_BLACK);
  tft.setCursor(0, 225);
  tft.printf("Duration: %ld ms (should be 114) ", millis() - startTime);
}

//##########################################################################################################################//
void sendVIS() {
  // VIS Start bit (1200 Hz, 300 ms)
  modulate(1200, 300);

  // Send 8-bit VIS code (LSB first)
  for (int i = 0; i < 8; i++) {
    int bit = (VIS_CODE >> i) & 1;
    modulate(bit ? 1900 : 1200, 300);
  }

  // Parity bit (odd parity)
  int parity = __builtin_popcount(VIS_CODE) % 2;
  modulate(parity ? 1900 : 1200, 300);

  // VIS End bit (1200 Hz, 300 ms)
  modulate(1200, 300);
}


//##########################################################################################################################//

void modulate(uint16_t freq, uint32_t duration) {

  if (mode == 1 )
    setTone(freq);
  else 
    si5351.set_pll(pllAStartFreq + (long)freq * mult, SI5351_PLLA);
  
  delayMicroseconds(duration);
}



