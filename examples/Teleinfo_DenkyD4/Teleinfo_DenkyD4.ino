// **********************************************************************************
// Arduino Teleinfo sample, return JSON data of modified teleinfo values received
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
// History : V1.00 2015-06-14 - First release
//           V1.01 2021-04-18 - Refactored for Denky D4 standalone test only
//           V1.02 2022-10-25 - Use bew Denky D4 WS2812 LED
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#include <LibTeleinfo.h>
#include <NeoPixelBrightnessBus.h> 

#define SERIAL_DEBUG  Serial
#define SERIAL_TIC    Serial1

// RGB Led Pins
#define NEOPIXEL_DATA 14
#define NEOPIXEL_LEDS 1

#define PUSH_BUTTON 0

// Teleinfo RXD pin is connected to ESP32-PICO-V3-02 GPIO8
#define TIC_RX_PIN  8   

#define COLOR_BLACK     RgbColor(0x00, 0x00, 0x00)
#define COLOR_RED       RgbColor(0xFF, 0x00, 0x00)
#define COLOR_ORANGE    RgbColor(0xFF, 0x22, 0x00)
#define COLOR_YELLOW    RgbColor(0xFF, 0xAA, 0x00)
#define COLOR_GREEN     RgbColor(0x00, 0xFF, 0x00)
#define COLOR_CYAN      RgbColor(0x00, 0xFF, 0xFF)
#define COLOR_BLUE      RgbColor(0x00, 0x00, 0xFF)
#define COLOR_VIOLET    RgbColor(0x99, 0x00, 0xFF)
#define COLOR_MAGENTA   RgbColor(0xFF, 0x00, 0x33)
#define COLOR_PINK      RgbColor(0xFF, 0x33, 0x77)
#define COLOR_AQUA      RgbColor(0x55, 0x7D, 0xFF)
#define COLOR_WHITE     RgbColor(0xFF, 0xFF, 0xFF)

// Led is common anode so reversed, this means
// 0   = Full brightness
// 255 = Minimal brightness
// 256 = OFF
uint8_t rgb_brightness = 255;

NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip( NEOPIXEL_LEDS, NEOPIXEL_DATA);

// Default mode, can be switched back and forth 
// with Denky D4 push button
_Mode_e tinfo_mode = TINFO_MODE_HISTORIQUE; 

TInfo tinfo; // Teleinfo object

// Pour clignotement LED asynchrone
unsigned long blinkLed  = 0;
int blinkDelay= 0;

// Uptime timer
boolean tick1sec=0;// one for interrupt, don't mess with 
unsigned long uptime=0; // save value we can use in sketch even if we're interrupted

// Used to indicate if we need to send all date or just modified ones
boolean fulldata = true;

//HardwareSerial Serial1(2);  // UART1/Serial2 pins 16,17
//HardwareSerial Serial1(1);  // UART1/Serial1 pins 9,10
//HardwareSerial Serial1(1);  // UART1/Serial1 pins 9,10

/* ======================================================================
Function: ShowStats
Purpose : display teleinfo stats
Input   : -
Output  : - 
Comments: -
====================================================================== */
void ShowStats()
{
  SERIAL_DEBUG.println(F("\r\n======= Errors ======="));
  SERIAL_DEBUG.printf_P(PSTR("Checksum     : %d\r\n"), tinfo.getChecksumErrorCount());
  SERIAL_DEBUG.printf_P(PSTR("Frame Size   : %d\r\n"), tinfo.getFrameSizeErrorCount());
  SERIAL_DEBUG.printf_P(PSTR("Frame Format : %d\r\n"), tinfo.getFrameFormatErrorCount());
  SERIAL_DEBUG.printf_P(PSTR("Interrupts   : %d\r\n"), tinfo.getFrameInterruptedCount());
  SERIAL_DEBUG.println(    F("======================"));
}

 
/* ======================================================================
Function: ADPSCallback 
Purpose : called by library when we detected a ADPS on any phased
Input   : phase number 
            0 for ADPS (monophase)
            1 for ADIR1 triphase
            2 for ADIR2 triphase
            3 for ADIR3 triphase
Output  : - 
Comments: should have been initialised in the main sketch with a
          tinfo.attachADPSCallback(ADPSCallback())
====================================================================== */
void ADPSCallback(uint8_t phase)
{
  // Envoyer JSON { "ADPS"; n}
  // n = numero de la phase 1 à 3
  if (phase == 0)
    phase = 1;
  SERIAL_DEBUG.print(F("{\"ADPS\":"));
  SERIAL_DEBUG.print('0' + phase);
  SERIAL_DEBUG.println(F("}"));
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
  // Start short led blink
  strip.SetPixelColor(0, COLOR_RED);
  strip.Show();
  blinkLed = millis();
  blinkDelay = 50; // 50ms
  ShowStats();

  // Envoyer les valeurs uniquement si demandé
  if (fulldata) 
    sendJSON(me, true);

  fulldata = false;
}

/* ======================================================================
Function: UpdatedFrame 
Purpose : callback when we received a complete teleinfo frame
Input   : linked list pointer on the concerned data
Output  : - 
Comments: it's called only if one data in the frame is different than
          the previous frame
====================================================================== */
void UpdatedFrame(ValueList * me)
{
  // Start long led blink
  strip.SetPixelColor(0, COLOR_BLUE);
  strip.Show();

  blinkLed = millis();
  blinkDelay = 50; // 50ms
  ShowStats();

  // Envoyer les valeurs 
  sendJSON(me, fulldata);
  fulldata = false;
}

/* ======================================================================
Function: sendJSON 
Purpose : dump teleinfo values on serial
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: -
====================================================================== */
void sendJSON(ValueList * me, boolean all)
{
  bool firstdata = true;

  // Got at least one ?
  if (me) {
    // Json start
    SERIAL_DEBUG.println();
    SERIAL_DEBUG.print(F("{"));

    if (all) {
      SERIAL_DEBUG.print(F("\"_UPTIME\":"));
      SERIAL_DEBUG.print(uptime, DEC);
      firstdata = false;
    }

    // Loop thru the node
    while (me->next) {
      // go to next node
      me = me->next;

      // uniquement sur les nouvelles valeurs ou celles modifiées 
      // sauf si explicitement demandé toutes
      if ( all || ( me->flags & (TINFO_FLAGS_UPDATED | TINFO_FLAGS_ADDED) ) )
      {
        // First elemement, no comma
        if (firstdata)
          firstdata = false;
        else
          SERIAL_DEBUG.print(F(", ")) ;

        SERIAL_DEBUG.print(F("\"")) ;
        SERIAL_DEBUG.print(me->name) ;
        SERIAL_DEBUG.print(F("\":")) ;

        // we have at least something ?
        if (me->value && strlen(me->value))
        {
          boolean isNumber = true;
          char * p = me->value;

          // check if value is number
          while (*p && isNumber) {
            if ( *p < '0' || *p > '9' )
              isNumber = false;
            p++;
          }
  
          // this will add "" on not number values
          if (!isNumber) {
            SERIAL_DEBUG.print(F("\"")) ;
            SERIAL_DEBUG.print(me->value) ;
            SERIAL_DEBUG.print(F("\"")) ;
          }
          // this will remove leading zero on numbers
          else
            SERIAL_DEBUG.print(atol(me->value));
        }
      }
    }
   // Json end
   SERIAL_DEBUG.println(F("}")) ;
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
Function: ledOff
Purpose : Setup I/O for RGB Led to be OFF
Input   : -
Output  : - 
Comments: -
====================================================================== */
void ledOff() {
  // Can be one of the 2 option
  strip.SetPixelColor(0, COLOR_BLACK);
  strip.Show();
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
  strip.SetBrightness(rgb_brightness);
  strip.Begin();
  strip.SetPixelColor(0, COLOR_RED);
  strip.Show();
  
  // Arduino LED Off
  ledOff();

  // Serial, pour le debug
  SERIAL_DEBUG.begin(115200);
  SERIAL_DEBUG.println(F("\r\n\r\n=============="));
  SERIAL_DEBUG.println(F("Teleinfo Denky D4"));

  // Button
  pinMode(PUSH_BUTTON, INPUT_PULLUP);     
  SERIAL_DEBUG.printf_P(PSTR("Enable Button on GPIO=%d\r\n"), PUSH_BUTTON);

  // Init Serial Port
  initSerial();

  // Init teleinfo
  tinfo.init(tinfo_mode);

  // Attacher les callback dont nous avons besoin
  // pour cette demo, ADPS et TRAME modifiée
  tinfo.attachADPS(ADPSCallback);
  tinfo.attachUpdatedFrame(UpdatedFrame);
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

    // Start long led blink
    strip.SetPixelColor(0, COLOR_GREEN);
    strip.Show();
    blinkLed = millis();
    blinkDelay = 100; // 100ms

    // Init Serial Port
    initSerial();

    // Init teleinfo
    tinfo.init(tinfo_mode);

    lastDebounceTime = millis(); 
    buttonState = 0;
  } 

  
  // Avons nous recu un ticker de seconde?
  if (tick1sec)
  {
    tick1sec = false;
    uptime++;

    // Forcer un envoi de trame complète toutes les minutes
    // fulldata sera remis à 0 après l'envoi
    if (uptime % 60 == 0)
      fulldata = true;
  }
  
  // On a reçu un caractère ?
  if ( SERIAL_TIC.available() ) {
    // Le lire
    c = SERIAL_TIC.read();

    // Gérer
    tinfo.process(c);

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
      SERIAL_DEBUG.print(c);
    }
  }

  // Verifier si le clignotement LED doit s'arreter 
  if (blinkLed && ((millis()-blinkLed) >= blinkDelay))
  {
      ledOff();
      blinkLed = 0;
  }

  if (currentMillis - previousMillis > 1000 ) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    tick1sec = true;
  }
}


