
/* NOT SURE WHETHER THIS WORKS WELL. MAY NEED REWORK!???

Each RTTY character follows this format:

(Start)  Data Bits (LSB First)  Stop
   0       D0  D1  D2  D3  D4     1
Bit Type	Value	Duration (at 45.45 baud)


Start Bit	0 (SPACE)	22 ms
5 Data Bits	LSB first	110 ms (22 ms per bit)

Stop Bit(s)	1 (MARK)	1.5 or 2 bits (33-44 ms)

Total duration per character: ~165-176 ms (for 1.5 or 2 stop bits)

2. Common RTTY Frequencies
RTTY uses two tones for MARK (1) and SPACE (0).

Baud Rate	MARK (Hz)2125 Hz
SPACE (Hz) 2295 Hz	

There are approximately 46.75 rising edges for the mark frequency (2125 Hz).

There are approximately 50.49 rising edges for the space frequency (2295 Hz).

*/




// RTTY Frequencies
const int MARK_FREQUENCY = 2125;   // Hz
const int SPACE_FREQUENCY = 2295;  // Hz
//const int SPACE_FREQUENCY = 2575; 
// timing
const float BAUD_RATE = 45.45;
//const float BAUD_RATE = 50.0;
const long BIT_DURATION = 22000;  // 22000 µs per bit


/*

const int MARK_FREQUENCY = 2000;   // Hz
const int SPACE_FREQUENCY = 2450;  // Hz
const float BAUD_RATE = 50.0;
const long BIT_DURATION = 20000;

*/

// Define Baudot Code Structure
typedef struct {
    uint8_t baudot;  // 5-bit Baudot code
    char letters;    // ASCII character in Letters shift
    char figures;    // ASCII character in Figures shift
} BaudotCode;

// Baudot Code Lookup Table (Sorted by Frequency)
const BaudotCode baudot_table[32] = {
    {0b00001, 'E', '3'}, {0b00010, '\n','\n'}, {0b00011, 'A','-'}, {0b00100, ' ', ' '},
    {0b00101, 'S','\''}, {0b00110, 'I','8'}, {0b00111, 'U','7'}, {0b01000, '\r','\r'},
    {0b01001, 'D','$'}, {0b01010, 'R','4'}, {0b01011, 'J','\a'}, {0b01100, 'N',','},
    {0b01101, 'F','!'}, {0b01110, 'C',':'}, {0b01111, 'K','('}, {0b10000, 'T','5'},
    {0b10001, 'Z','+'}, {0b10010, 'L',')'}, {0b10011, 'W','2'}, {0b10100, 'H','#'},
    {0b10101, 'Y','6'}, {0b10110, 'P','0'}, {0b10111, 'Q','1'}, {0b11000, 'O','9'},
    {0b11001, 'B','?'}, {0b11010, 'G','&'}, {0b11011, 0, 0},   // LETTERS shift
    {0b11100, 'M','.'}, {0b11101, 'X','/'}, {0b11110, 'V','='}, {0b11111, 0, 0} // FIGURES shift
};


// Shift state (default: Letters mode)
bool isFigures = false;

//##########################################################################################################################//
// Function to get Baudot code for a given ASCII character
uint8_t getBaudotCode(char character) {


    for (int i = 0; i < 32; i++) {
        if (character == baudot_table[i].letters || character == baudot_table[i].figures) {
            return baudot_table[i].baudot;
        }
    }
    return 0xFF; // Error
}


//##########################################################################################################################//


void sendStartBit() {
  si5351.set_pll(pllAStartFreq + (long) SPACE_FREQUENCY* mult, SI5351_PLLA);
  delayMicroseconds(BIT_DURATION);
}

//##########################################################################################################################//
// Function to send stop bits (1.5 stop bits)
void sendStopBits() {

  si5351.set_pll(pllAStartFreq + (long) MARK_FREQUENCY* mult, SI5351_PLLA); // Mark
  delayMicroseconds(BIT_DURATION + (BIT_DURATION / 2));
}

//##########################################################################################################################//
// Function to send a single bit
void sendBit(bool bit) {
  if (bit) {
      si5351.set_pll(pllAStartFreq + (long) MARK_FREQUENCY* mult, SI5351_PLLA);// Mark
    delayMicroseconds(BIT_DURATION);

  }

  else {
       si5351.set_pll(pllAStartFreq + (long) SPACE_FREQUENCY* mult, SI5351_PLLA);
      delayMicroseconds(BIT_DURATION); 
  
  }
}

//##########################################################################################################################//

// Function to send a Baudot character
void sendBaudot(uint8_t baudotChar) {
  sendStartBit();  // Start bit

  for (int i = 0; i < 5; i++) {
    sendBit(baudotChar & 0x01);  //LSB first
    baudotChar >>= 1;
  }

  sendStopBits();  // 1.5 Stop bits
}


bool isLetterMode(char character) {
    for (int i = 0; i < 32; i++) {
        if (baudot_table[i].letters == character) return true;
    }
    return false;
}

bool isFiguresMode(char character) {
    for (int i = 0; i < 32; i++) {
        if (baudot_table[i].figures == character) return true;
    }
    return false;
}


//##########################################################################################################################//


// Function to send an RTTY message
void sendRTTY(const char* message) {
    while (*message) {
        char character = *message;
        uint8_t baudotCode = getBaudotCode(character);

        if (baudotCode == 0xFF) {  // Invalid character error
            message++;
            continue;
        }

        if (!isFigures && isFiguresMode(character)) {
            sendBaudot(27);  // Shift to FIGURES
            isFigures = true;
        } else if (isFigures && isLetterMode(character)) {
            sendBaudot(31);  // Shift to LETTERS
            isFigures = false;
        }

        sendBaudot(baudotCode);  // Send Baudot code
        message++;               // Next character
      
        if (baudotCode == 0x00) {  // Invalid character error
           sendBaudot(2); // CR
           sendBaudot(8); //LF
        }
    
    }

  

}





//##########################################################################################################################//
void RTTY() {

  pllAStartFreq = si5351.plla_freq;
  mult = pllAStartFreq / FREQ;
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 200);
  tft.print(" 45.45Bd, 1.5Stop, USB, FSK");

  while (true) {
 
    sendRTTY("ABCDEFGHIJKLMNOPQRSTUVWXYZ RTTY_TEST 123456789 ");  
    delay(30);

   if (tft.getTouchRawZ() > 300) return;

    for (int i = 0; i < 10; i++) {
     sendBaudot(0x1F); 
     delay(22); 
    }
      


     si5351.set_pll(pllAStartFreq + (long) MARK_FREQUENCY* mult, SI5351_PLLA);
  }
}

//##########################################################################################################################//