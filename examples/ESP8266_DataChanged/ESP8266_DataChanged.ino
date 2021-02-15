// **********************************************************************************
// ESP8266 Teleinfo, display changed data received and blink RGB Led
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

#define RGB_LED_PIN 0 // GPIO0

// Output Debug on Serial1 since Hardware Serial is used to receive Teleinfo
#define SerialMon Serial1

#ifdef RGB_LED_PIN
#include <NeoPixelBus.h>
#define colorSaturation 128
// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> strip(1, RGB_LED_PIN);
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);
#endif

TInfo tinfo; // Teleinfo object

// Pour clignotement LED asynchrone
unsigned long blinkLed  = 0;
uint16_t blinkDelay= 0;

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
  // n = numero de la phase 1 à 3
  if (phase == 0)
    phase = 1;
  SerialMon.print(F("ADPS:"));
  SerialMon.println('0' + phase);

  #ifdef RGB_LED_PIN
  strip.SetPixelColor(0, red);
  // Keep it RED between all frame until it disapears
  blinkDelay = 2500; // 2.5s should be enough
  strip.Show();
  blinkLed = millis();
  #endif
}

/* ======================================================================
Function: DataCallback
Purpose : callback when we detected new or modified data received
Input   : linked list pointer on the concerned data
          current flags value
Output  : -
Comments: -
====================================================================== */
void DataCallback(ValueList * me, uint8_t  flags)
{
  RgbColor col(0, 0, colorSaturation);

  // Nouvelle etiquette ?
  if (flags & TINFO_FLAGS_ADDED) {
    SerialMon.print(F("NEW -> "));
    #ifdef RGB_LED_PIN
    strip.SetPixelColor(0, green);
    strip.Show();
    blinkDelay = 10; // 10ms
    #endif
  }

  // Valeur de l'étiquette qui a changée ?
  if (flags & TINFO_FLAGS_UPDATED) {
    SerialMon.print(F("MAJ -> "));
    #ifdef RGB_LED_PIN
    strip.SetPixelColor(0, blue);
    strip.Show();
    blinkDelay = 50; // 50ms
    #endif
  }

  // Display values
  SerialMon.print(me->name);
  SerialMon.print("=");
  SerialMon.println(me->value);

  blinkLed = millis();

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
  // Serial1, pour le debug la Serial est pour la téléinfo
  // Donc débrancher la téléinfo pour les update via USB
  SerialMon.begin(115200);
  SerialMon.println(F("\r\n\r\n=============="));
  SerialMon.println(F("Teleinfo"));


  // this resets all the neopixels to an off state
  #ifdef RGB_LED_PIN
  SerialMon.printf_P(PSTR("Enable WS2812 RGB LED on GPIO=%d\r\n"), RGB_LED_PIN);
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
  Serial.begin(1200, SERIAL_7E1);

  //If you have no pullup on Serial entry try to uncomment the following line
  //pinMode(TIC_RX_PIN, INPUT_PULLUP);

  // Init teleinfo
  tinfo.init();

  // Attacher les callback dont nous avons besoin
  // pour cette demo, ADPS et TRAME modifiée
  tinfo.attachADPS(ADPSCallback);
  tinfo.attachData(DataCallback);
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

  // On a reçu un caractère ?
  if ( Serial.available() ) {
    // Le lire
    c = Serial.read();

    // Gérer
    tinfo.process(c);

    // L'affcher dans la console
    //if (c!=TINFO_STX && c!=TINFO_ETX) {
      //SerialMon.print(c);
    //}
  }

  // Verifier si le clignotement LED doit s'arreter
  if (blinkLed && ((millis()-blinkLed) >= blinkDelay))  {

    #ifdef RGB_LED_PIN
    strip.SetPixelColor(0, black);
    strip.Show();
    #endif

    blinkLed = 0;
  }
}

