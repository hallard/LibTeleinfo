// **********************************************************************************
// ESP32 Teleinfo basic 
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// For any explanation about teleinfo ou use , see my blog
// http://hallard.me/category/tinfo
//
// This program works with the Wifinfo board
// see schematic here https://github.com/hallard/teleinfo/tree/master/Wifinfo
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2020-06-11 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#include <LibTeleinfo.h>

//#define BOARD_EZSBC
//#define BOARD_LOLIN32
#define BOARD_DENKY32

// https://www.tindie.com/products/ddebeer/esp32-dev-board-wifibluetooth-with-ftdi-/
#if defined (BOARD_EZSBC)
// Set up the rgb led names
#define LED_RED_PIN 16
#define LED_GRN_PIN 17
#define LED_BLU_PIN 18

#define PUSH_BUTTON 0

#define TIC_ENABLE_PIN 27
#define TIC_RX_PIN     33
#define TIC_TX_PIN     32

// Lolin32
#elif defined (BOARD_LOLIN32)

#define PUSH_BUTTON     15

#define TIC_ENABLE_PIN  32
#define TIC_RX_PIN      16
#define TIC_TX_PIN      17

#define RGB_LED_PIN     13

// Denky32
#elif defined (BOARD_DENKY32)

#define PUSH_BUTTON     0

#define TIC_ENABLE_PIN  4
#define TIC_RX_PIN      33
//#define TIC_TX_PIN      17

#define LORA_TX_PIN     26
#define LORA_RX_PIN     27
#define LORA_RESET      14

#define RGB_LED_PIN     25


#endif

#ifdef RGB_LED_PIN
#include <NeoPixelBus.h>

#define colorSaturation 128
// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(1, RGB_LED_PIN);

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

#endif


TInfo tinfo; // Teleinfo object

// Pour clignotement LED asynchrone
unsigned long blinkLed  = 0;
uint8_t blinkDelay= 0;

// Uptime timer
boolean tick1sec=0;// one for interrupt, don't mess with 
unsigned long uptime=0; // save value we can use in sketch even if we're interrupted

// Used to indicate if we need to send all date or just modified ones
boolean fulldata = true;

//HardwareSerial Serial1(2);  // UART1/Serial2 pins 16,17
//HardwareSerial Serial1(1);  // UART1/Serial1 pins 9,10
//HardwareSerial Serial1(1);  // UART1/Serial1 pins 9,10
 
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
  Serial.print(F("{\"ADPS\":"));
  Serial.print('0' + phase);
  Serial.println(F("}"));
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
  #ifdef LED_RED_PIN
  digitalWrite(LED_RED_PIN, LOW);
  #endif
  #ifdef RGB_LED_PIN
  strip.SetPixelColor(0, red);
  strip.Show();
  #endif
  blinkLed = millis();
  blinkDelay = 50; // 50ms

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
  #ifdef LED_BLU_PIN
  digitalWrite(LED_BLU_PIN, LOW);
  #endif
  #ifdef RGB_LED_PIN
  strip.SetPixelColor(0, blue);
  strip.Show();
  #endif

  blinkLed = millis();
  blinkDelay = 50; // 50ms

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
    Serial.print(F("{"));

    if (all) {
      Serial.print(F("\"_UPTIME\":"));
      Serial.print(uptime, DEC);
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
          Serial.print(F(", ")) ;

        Serial.print(F("\"")) ;
        Serial.print(me->name) ;
        Serial.print(F("\":")) ;

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
            Serial.print(F("\"")) ;
            Serial.print(me->value) ;
            Serial.print(F("\"")) ;
          }
          // this will remove leading zero on numbers
          else
            Serial.print(atol(me->value));
        }
      }
    }
   // Json end
   Serial.println(F("}")) ;
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
  // Arduino LED
  #ifdef BOARD_EZSBC
  pinMode(LED_RED_PIN, OUTPUT);     
  pinMode(LED_GRN_PIN, OUTPUT);     
  pinMode(LED_BLU_PIN, OUTPUT);     
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(LED_GRN_PIN, HIGH);
  digitalWrite(LED_BLU_PIN, HIGH);
  #endif

  // Serial, pour le debug
  Serial.begin(115200);
  Serial.println(F("\r\n\r\n=============="));
  Serial.println(F("Teleinfo"));


  // Teleinfo enable pin
  #ifdef TIC_ENABLE_PIN
  pinMode(TIC_ENABLE_PIN, OUTPUT);     
  digitalWrite(TIC_ENABLE_PIN, HIGH);
  Serial.printf_P(PSTR("Enable TIC on  GPIO%d\r\n"), TIC_ENABLE_PIN);
  #endif 

  // Button
  #ifdef PUSH_BUTTON
  pinMode(PUSH_BUTTON, INPUT_PULLUP);     
  Serial.printf_P(PSTR("Enable Button on GPIO=%d\r\n"), PUSH_BUTTON);
  #endif



  // this resets all the neopixels to an off state
  #ifdef RGB_LED_PIN
  Serial.printf_P(PSTR("Enable WS2812 RGB LED on GPIO=%d\r\n"), RGB_LED_PIN);
  strip.Begin();
  strip.SetPixelColor(0, green);
  strip.Show();
  blinkLed = millis();
  blinkDelay = 500; // 500ms
  #endif

  // Configure Teleinfo Soft serial 
  // La téléinfo est connectee sur D3
  // ceci permet d'eviter les conflits avec la 
  // vraie serial lors des uploads
  Serial.printf_P(PSTR("TIC RX = GPIO=%d\r\n"), TIC_RX_PIN);
  Serial1.begin(1200, SERIAL_7E1, TIC_RX_PIN);
  pinMode(TIC_RX_PIN, INPUT_PULLUP);

  // Init teleinfo
  tinfo.init();

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
  unsigned long currentMillis = millis();

  // Button to enable TIC
#if defined (PUSH_BUTTON) && defined (TIC_ENABLE_PIN)
  static uint8_t enableTIC = HIGH;
  static uint8_t buttonState = 0;
  static unsigned long lastDebounceTime = 0;  

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
    if ( enableTIC ) {
      digitalWrite(TIC_ENABLE_PIN, LOW);
      enableTIC = false; 
    } else  {
      digitalWrite(TIC_ENABLE_PIN, HIGH);
      enableTIC = true; 
    }

    Serial.printf_P(PSTR("\r\nEnable TIC=%d\r\n"), enableTIC);
    lastDebounceTime = millis(); 
    buttonState = 0;
  } 

#endif

  
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
  if ( Serial1.available() ) {
    // Le lire
    c = Serial1.read();

    // Gérer
    tinfo.process(c);

    // L'affcher dans la console
    if (c!=TINFO_STX && c!=TINFO_ETX) {
      Serial.print(c);
    }
  }

  // Verifier si le clignotement LED doit s'arreter 
  if (blinkLed && ((millis()-blinkLed) >= blinkDelay))
  {
    #ifdef BOARD_EZSBC
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_GRN_PIN, HIGH);
    digitalWrite(LED_BLU_PIN, HIGH);
    #endif

    #ifdef RGB_LED_PIN
    strip.SetPixelColor(0, black);
    strip.Show();
    #endif

    blinkLed = 0;
  }

  if (currentMillis - previousMillis > 1000 ) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    tick1sec = true;
  }
}


