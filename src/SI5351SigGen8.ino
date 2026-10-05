/*
Non-Commercial Source Code License Agreement

Grant of License
The licensor grants you a non-exclusive, worldwide, royalty-free license to:

Use, modify, and compile the source code for personal/non-commercial purposes

Distribute unmodified copies to others for non-commercial use


Commercialization Prohibited
You may not:

Sell, sublicense, or monetize the source code or derivatives

Integrate the code into commercial products/services

Use the code for revenue-generating activities

Attribution
Original copyright notices and license terms must remain intact in all copies.

No Warranty
The code is provided "AS IS" without warranties of any kind.

Termination
Violation of these terms automatically revokes all rights granted herein.


Copyright Holder: [Ulrich Schmidt]

*/


/*

Project: Basic signal generator with AM modulator
Hardware description:

ESP32+ILI9341+SI5351A module +2 mosfets, 2npn transistors and a signal relay. 

ESP32 and display are connected in a standard SPI configuration as per defined below. The SI5351 is connected to the I2C bus of the ESP(GPIO21 and 22). I2C Busspeed 2MHz seems to work fine.
GPIO27 drives via 1.5K the base of a npn resistor.  Emitter on ground and collector connected to coil of signal relay and protection diode. Other pin of the relay goes to +5V.

The modulator consists of 2 dual gate mosfets BF961. The 1st mosfet is the AM modulator and passes the signal to the 2nd mosfet which adjusts the output level. 
CLK0 and CLK1 of the SI5351 go via 100n and 470R each to G1 of the 1st Mosfet. G1 is also grounded via 100R. This attenuates the level by around 15dB and allows to use either
CLK0 or CLK1.
G2 goes to the center of a 100K pot (this adjusts the carrier to about 50% of the max. output), the other 2 pins of the pot go to ground and +3.3. 
G2 also gets switched by the relay btw external input and GPIO26. This allows to select which source modulates.
Source of the mosfet is grounded and Drain goes via 100R to +3.3V. Drain also goes via 100nf to G1 of the 2nd mosfet. This G1 is also grounded via 100R. 
G2 goes via 1.5K to GPIO25 (dac1).
This allows to adjust the output level by roughly 20dB. Drain goes via 100R to +3.3V and via 100nf to the output of the signal generator. 



*/
#include "driver/ledc.h"
#include "FS.h"
#include <si5351.h> 
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Preferences.h>  //writes, reads and stores data on the EEPROM
#include "DacESP32.h"    // sine wave generator library
#include <LittleFS.h>
#include <stdint.h>
#include <WebServer.h> //File system uploader
#include <WiFi.h>                 // Wi-Fi (ESP32 core)
#include <WiFiClient.h>           // TCP client (ESP32 core)
#include <WiFiClientSecure.h>     // Secure TCP client (ESP32 core)
#include <WiFiUdp.h>              // UDP streamer (ESP32core)
#include "SD.h"                   // SD card support (ESP32 core)
#define DISABLE_FS_H_WARNING    // Needed to disable warning caused by SdFat.h, "type File not defined".
#include <SdFat.h>              // Library Manager: "SdFat" by Bill Greiman

//##########################################################################################################################//

//colors
#define TFT_BLACK 0x0000 /*   0,   0,   0 */
#define TFT_NAVY 0x000F  /*   0,   0, 128 */
#define TFT_DARKDARKGREEN 0x1b41
#define TFT_DARKGREEN 0x03E0
#define TFT_MIDGREEN 0x0584
#define TFT_DARKCYAN 0x03EF  /*   0, 128, 128 */
#define TFT_MAROON 0x7800    /* 128,   0,   0 */
#define TFT_PURPLE 0x780F    /* 128,   0, 128 */
#define TFT_OLIVE 0x7BE0     /* 128, 128,   0 */
#define TFT_LIGHTGREY 0xD69A /* 211, 211, 211 */
#define TFT_DARKGREY 0x7BEF  /* 128, 128, 128 */
#define TFT_DARKDARKGREY 0x2945
#define TFT_SILVERBLUE 0x8D5F
#define TFT_BLUE 0x001F  /*   0,   0, 255 */
#define TFT_GREEN 0x07E0 /*   0, 255,   0 */
#define TFT_CYAN 0x07FF  /*   0, 255, 255 */
#define TFT_RED 0xF800
#define TFT_DARKRED 0x90C1 /* 255,   0,   0 */
#define TFT_MAGENTA 0xF81F /* 255,   0, 255 */
#define TFT_YELLOW 0xFFE0  /* 255, 255,   0 */
#define TFT_WHITE 0xFFFF   /* 255, 255, 255 */
#define TFT_ORANGE 0xFDA0  /* 255, 180,   0 */
#define TFT_DEEPORANGE 0xf401
#define TFT_GREENYELLOW 0xB7E0
#define TFT_PINK 0xFE19 /* 255, 192, 203 */
#define TFT_DARKBROWN 0x6a44
#define TFT_BROWN 0x9A60 /* 150,  75,   0 */
#define TFT_LIGHTBROWN 0x9347
#define TFT_GOLD 0xFEA0    /* 255, 215,   0 */
#define TFT_SILVER 0xC618  /* 192, 192, 192 */
#define TFT_SKYBLUE 0x867D /* 135, 206, 235 */
#define TFT_VIOLET 0x915C  /* 180,  46, 226 */
#define TFT_GREY 0x5AEB
#define TFT_FOREGROUND TFT_GOLD
#define TFT_GRID TFT_YELLOW

//plain button colors
#define TFT_BTNBDR TFT_NAVY
#define TFT_BTNCTR TFT_BLUE
#define TFT_MAINBTN_BDR TFT_NAVY
#define TFT_MAINBTN_CTR TFT_GREY

// button  sizes
#define TILE_WIDTH 65
#define TILE_HEIGHT 50

#define SLIDER_X 30    // Start position
#define SLIDER_Y 120   // Vertical position
#define SLIDER_WIDTH 255
#define SLIDER_HEIGHT 20

//display size
#define DISP_WIDTH 320
#define DISP_HEIGHT 240

#define ESP32_I2C_SDA 21  // I2C bus pin on ESP32
#define ESP32_I2C_SCL 22  // I2C bus pin on ESP32


//TFT

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)
#define TOUCH_CS 5     // Chip select pin (T_CS) of touch screen

//SD card
#define SCK 18    // definitions for SD card
#define MISO 19   //MISO=SDO
#define MOSI 23   //MOSI=SDI
#define SD_CS 25  // Connect CS to GPIO25
// Format SDcard as FAT32



//instances
TFT_eSPI tft = TFT_eSPI();
Si5351 si5351;        //Si5351 I2C Address 0x60
Preferences preferences;     // EEPROM save data
SdFat sd;  // SdFat library
SPIClass spiSD = SPIClass(VSPI);                // for SDcard


// Web

String password = "YourPW";
String ssid = "YourSSID";




// globals

long FREQ = 27500000;        // starting frequency in CB band to avoid issues
long FREQ_OLD = FREQ - 1;     // start with != FREQ value
long LOW_FREQ = 10;           // lowest frequency
long HI_FREQ = 225000000;      // highest frequency
const long correction = 155000;      // correction value of my SI5

uint16_t modfreq;      // variables to change the PLL frequency quickly in SSTV and RTTY
uint16_t mult;
uint64_t pllAStartFreq;

long I2C_BUSSPEED = 2000000;  // Adjust as needed
long freq = 0;                // for frequency display
uint16_t textColor = TFT_WHITE; // general letter and button color

bool pressed = false;  //
bool pin27State = false; // Output pin to switch relay for internal/external AM modulation
uint16_t tx = 0, ty = 0;
bool showTouchCoordinates = false; // for debugging
uint16_t buttonID = 0, row = 0, column = 0;
long keyVal = 0;       // global for values delivered by keypad (using return)
int sliderValue = 127;  // Default slider position  (0-255)

DacESP32 dac1(GPIO_NUM_25); // DAC for output level
DacESP32 dac2(GPIO_NUM_26);


// .wav player
const uint32_t aSR = 8000;  // audio sample rate
hw_timer_t* timer = NULL;   // ISR

uint8_t playBuffer[512]; // ISR audio player buffer
uint16_t playIndex = 0;
volatile bool bufferPlaying = false;



//##########################################################################################################################//

void setup() {

  tft.init();
  // Calibration code for touchscreen : for 2.8 inch & Rotation = 1
  //tft.setRotation(1);
  //tft.invertDisplay(true);

  tft.setRotation(3);


  
    uint16_t calData[5] = { // values for ILI9341
                          (uint16_t)preferences.getInt("cal0", 246),
                          (uint16_t)preferences.getInt("cal1", 3448),
                          (uint16_t)preferences.getInt("cal2", 381),
                          (uint16_t)preferences.getInt("cal3", 3264),
                          (uint16_t)preferences.getInt("cal4", 7)
  };

 
  
  tft.setTouch(calData);
  preferences.begin("data", false);  // preferences namespace is data.
  tft.setTextColor(textColor);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  Wire.begin(21, 22, I2C_BUSSPEED); 
  SI5351_Init();
  FREQ = preferences.getLong("lastFreq", 27500000);                 // load last Freq 
  dac1.outputVoltage((uint8_t)127); // medium RF ouptput level
  dac2.disable(); // sine tone generator off
  pinMode(27, OUTPUT);
  delay(500);
  tft.fillScreen(TFT_BLACK);
  buildMainScreen(); 
}

//##########################################################################################################################//
void loop() {

  if(FREQ_OLD != FREQ) {
    limitFREQ();
    displayFREQ(FREQ);
    settleSynth();
    si5351.set_freq(FREQ *100ULL, SI5351_CLK1); 
    FREQ_OLD = FREQ;
  }
  
  saveCurrentFreq();
  readTouchCoordinates();
  readMainBtns();
  delay(20);
}



//##########################################################################################################################//

void displayFREQ(long freq) {
  const uint16_t xPos = 0;
  const uint16_t yPos = 0;

  tft.setTextFont(7);
  tft.setTextSize(0);
  tft.fillRect(xPos, yPos, 320, 51, TFT_BLACK);
  tft.drawFastHLine(0, 51, 320, TFT_BLUE);
  tft.drawFastHLine(0, 52, 320, TFT_BLUE);


  
  long new_Mhz = freq / 1000000;
  long new_Khz = (freq - (new_Mhz * 1000000)) / 1000;
  long new_Hz = (freq - (new_Mhz * 1000000)) - new_Khz * 1000;


  tft.setTextColor(TFT_GREEN);

  if (new_Mhz >= 10) {  // position cursor
    tft.setCursor(xPos, yPos);
  } else if (new_Mhz >= 1) {
    tft.setCursor(xPos + 20, yPos);
  } else {
    tft.setCursor(xPos + 50, yPos);
  }
  tft.printf("%3.0ld", new_Mhz);
  tft.setCursor(xPos + 100, yPos);
  tft.printf(".%03ld", new_Khz);
  
  tft.setTextColor(TFT_DARKGREEN);
  tft.printf(".%03ld", new_Hz);

  tft.setTextFont(2);
  tft.setTextColor(textColor);
}


//##########################################################################################################################//
void limitFREQ(){

  if (FREQ < 1L)
      FREQ = 1L;

  if (FREQ > 225000000L)
      FREQ = 225000000L; 
}


//##########################################################################################################################//


void IRAM_ATTR processAudio() {  //audio  player via DAC2

  if (bufferPlaying) {
    dac1.outputVoltage(playBuffer[playIndex++]);  // this call is ISR save
    //dac2.outputVoltage(playBuffer[playIndex++]);  

    if (playIndex >= 512) {  // playbuffer size
      playIndex = 0;
      bufferPlaying = false;  // finished this buffer
    }
  }
}

//##########################################################################################################################//


void startPlayISR(){

  //set interrupt timer for sampling
  timer = timerBegin(aSR);  //sample rate fixed 8000
  timerAttachInterrupt(timer, &processAudio);

  // ticks per interrupt
  uint64_t baseFreq = timerGetFrequency(timer);
  uint64_t ticks = baseFreq / aSR;

  // enable alarm
  timerAlarm(timer, ticks, true, 0);
}

//##########################################################################################################################//
