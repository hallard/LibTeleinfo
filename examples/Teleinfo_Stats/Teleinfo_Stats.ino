// **********************************************************************************
// Arduino Teleinfo sample, Check and return Teleinfo Stats
// This sketch works as is only for Denky D4 board, need to be adapted for other
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// for detailled explanation of this library see dedicated article
// https://hallard.me/libteleinfo/
//
// For any explanation about teleinfo or use, see my blog
// https://hallard.me/category/tinfo
// 
// see dedicated article here https://hallard.me/demystifier-la-teleinfo/
//
// Written by Charles-Henri Hallard (https://hallard.me)
//
// History : V1.00 2023-01-21 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#include <LibTeleinfo.h>

#define SERIAL_DEBUG  Serial
#define SERIAL_TIC    Serial1

#define PUSH_BUTTON 0

// Teleinfo RXD pin is connected to ESP32-PICO-V3-02 GPIO8
#define TIC_RX_PIN  8   

// Default mode, can be switched back and forth 
// with Denky D4 push button
_Mode_e tinfo_mode = TINFO_MODE_HISTORIQUE; 

TInfo tinfo; // Teleinfo object

// Uptime timer
boolean tick1sec=0;// one for interrupt, don't mess with 
unsigned long uptime=0; // save value we can use in sketch even if we're interrupted

// Count total frames
uint32_t total_frames = 0;

/* ======================================================================
Function: ShowStats
Purpose : display teleinfo stats
Input   : -
Output  : - 
Comments: -
====================================================================== */
void ShowStats()
{
  SERIAL_DEBUG.println(F("\r\n======= Stats ======="));
  SERIAL_DEBUG.printf_P(PSTR("Total Frames : %d\r\n"), total_frames);
  SERIAL_DEBUG.printf_P(PSTR("Interrupts   : %d\r\n"), tinfo.getFrameInterruptedCount());
  SERIAL_DEBUG.printf_P(PSTR("Checksum Err : %d\r\n"), tinfo.getChecksumErrorCount());
  SERIAL_DEBUG.printf_P(PSTR("Size Err     : %d\r\n"), tinfo.getFrameSizeErrorCount());
  SERIAL_DEBUG.printf_P(PSTR("Format Err   : %d\r\n"), tinfo.getFrameFormatErrorCount());
  SERIAL_DEBUG.println(    F("====================="));
}


/* ======================================================================
Function: NewFrame 
Purpose : callback when we received a complete teleinfo frame
Input   : linked list pointer on the concerned data
Output  : - 
Comments: -
====================================================================== */
void NewFrame(ValueList * me)
{
  total_frames++;
  // Display stat info on each 10 frames
  if (total_frames % 10 == 0) {
    ShowStats();
  }
}

/* ======================================================================
Function: initSerial
Purpose : Configure (or reconfigure Serial Port)
Input   : -
Output  : - 
Comments: -
====================================================================== */
void initSerial()
{
  // Cleanup
  SERIAL_TIC.flush();
  SERIAL_TIC.end();

  // Configure Teleinfo 
  SERIAL_DEBUG.printf_P(PSTR("TIC RX=GPIO%d  Mode:"), TIC_RX_PIN);
  SERIAL_TIC.begin(tinfo_mode == TINFO_MODE_HISTORIQUE ? 1200 : 9600, SERIAL_7E1, TIC_RX_PIN);

  if ( tinfo_mode == TINFO_MODE_HISTORIQUE ) {
    SERIAL_DEBUG.println(F("Historique"));
  } else {
    SERIAL_DEBUG.println(F("Standard"));
  }
}

/* ======================================================================
Function: setup
Purpose : Setup I/O and other one time startup stuff
Input   : -
Output  : - 
Comments: -
====================================================================== */
void setup()
{
  // Serial, pour le debug
  SERIAL_DEBUG.begin(115200);
  SERIAL_DEBUG.println(F("\r\n\r\n"));
  SERIAL_DEBUG.println(F("====================================="));
  SERIAL_DEBUG.println(F("          Teleinfo stats "));
  SERIAL_DEBUG.println(F("====================================="));
  SERIAL_DEBUG.println(F("  You can change teleinfo mode from"));
  SERIAL_DEBUG.println(F("Historique to Standard with B0 button"));

  // Button
  pinMode(PUSH_BUTTON, INPUT_PULLUP);     

  // Init Serial Port
  initSerial();

  // Init teleinfo
  tinfo.init(tinfo_mode);

  // Attacher les callback dont nous avons besoin
  tinfo.attachNewFrame(NewFrame); 
}

/* ======================================================================
Function: loop
Purpose : infinite loop main code
Input   : -
Output  : - 
Comments: -
====================================================================== */
void loop()
{
  static char c;
  static unsigned long previousMillis = 0;
  static uint8_t buttonState = 0;
  static unsigned long lastDebounceTime = 0;  
  _State_e state;

  unsigned long currentMillis = millis();

  // Button to switch mode
  uint8_t button = digitalRead(PUSH_BUTTON);

  // New Press 
  if ( button==LOW && buttonState==0) {
    buttonState = 1; 
    lastDebounceTime = millis(); 

  // Pressed enought (debounced)
  } else if ( buttonState==1 && button==LOW && (millis()-lastDebounceTime)>50 ) {
    buttonState = 2; 
 
  // Release (no need debouce here)
  } else if ( buttonState==2 && button==HIGH ) {

    // Invert mode
    if ( tinfo_mode == TINFO_MODE_HISTORIQUE ) {
      tinfo_mode = TINFO_MODE_STANDARD;
    } else  {
      tinfo_mode = TINFO_MODE_HISTORIQUE;
    }

    // Init Serial Port
    initSerial();

    // Init teleinfo
    tinfo.init(tinfo_mode);

    lastDebounceTime = millis(); 
    buttonState = 0;
  } 

  
  // Avons nous recu un ticker de seconde?
  if (tick1sec) {
    tick1sec = false;
    uptime++;
  }
  
  // On a reçu un caractère ?
  if ( SERIAL_TIC.available() ) {
    // Le lire
    c = SERIAL_TIC.read();

    // Gérer
    state = tinfo.process(c);

    // L'affcher dans la console
    if (c==TINFO_STX) {
      SERIAL_DEBUG.print("<STX>");
    } else if (c==TINFO_ETX) {
      SERIAL_DEBUG.print("<ETX>");
    } else if (c==TINFO_HT) {
      SERIAL_DEBUG.print("<TAB>");
    } else if (c==TINFO_EOT) {
      SERIAL_DEBUG.print("<INTERRUPT>");
    } else {
      // Display only when receiving state OK
      if (state == TINFO_READY) {
        SERIAL_DEBUG.print(c);
      }
    }
  }

  if (currentMillis - previousMillis > 1000 ) {
    // save the last time 
    previousMillis = currentMillis;   
    tick1sec = true;
  }
}


