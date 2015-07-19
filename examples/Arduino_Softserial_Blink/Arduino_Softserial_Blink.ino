// **********************************************************************************
// Arduino Teleinfo sample, display information on teleinfo values received + blink
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
// connect Teleinfo RXD pin To Arduin D3
// see schematic here https://hallard.me/demystifier-la-teleinfo/
// and dedicated article here 
//
// Written by Charles-Henri Hallard (https://hallard.me)
//
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#include <SoftwareSerial.h>
#include <LibTeleinfo.h>

// Arduino on board LED
// I use moteino, so it's D9
// classic Arduino is D13
#define LEDPIN 9
//#define LEDPIN 13

SoftwareSerial Serial1(3,4); // Teleinfo Serial
TInfo          tinfo; // Teleinfo object

// Pour clignotement LED asynchrone
unsigned long blinkLed  = 0;
uint8_t       blinkDelay= 0;

 /* ======================================================================
Function: printUptime 
Purpose : print pseudo uptime value
Input   : -
Output  : - 
Comments: compteur de secondes basique sans controle de dépassement
          En plus SoftwareSerial rend le compteur de millis() totalement
          A la rue, donc la precision de ce compteur de seconde n'est
          pas fiable du tout, dont acte !!!
====================================================================== */
void printUptime(void)
{
  Serial.print(millis()/1000);
  Serial.print(F("s\t"));
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
  printUptime();

  // Monophasé
  if (phase == 0 ) {
    Serial.println(F("ADPS"));
  }
  else {
    Serial.print(F("ADPS PHASE #"));
    Serial.println('0' + phase);
  }
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
  // Show our not accurate second counter
  printUptime();

  if (flags & TINFO_FLAGS_ADDED) 
    Serial.print(F("NEW -> "));

  if (flags & TINFO_FLAGS_UPDATED)
    Serial.print(F("MAJ -> "));

  // Display values
  Serial.print(me->name);
  Serial.print("=");
  Serial.println(me->value);
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
  digitalWrite(LEDPIN, HIGH);
  blinkLed = millis();
  blinkDelay = 50; // 50ms

  // Show our not accurate second counter
  printUptime();
  Serial.println(F("FRAME -> SAME AS PREVIOUS"));
}

/* ======================================================================
Function: NewFrame 
Purpose : callback when we received a complete teleinfo frame
Input   : linked list pointer on the concerned data
Output  : - 
Comments: it's called only if one data in the frame is different than
          the previous frame
====================================================================== */
void UpdatedFrame(ValueList * me)
{
  // Start long led blink
  digitalWrite(LEDPIN, HIGH);
  blinkLed = millis();
  blinkDelay = 100; // 100ms

  // Show our not accurate second counter
  printUptime();
  Serial.println(F("FRAME -> UPDATED"));
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
  pinMode(LEDPIN, OUTPUT);     
   
  digitalWrite(LEDPIN, LOW);

  // Serial, pour le debug
  Serial.begin(115200);

  Serial.println(F("========================================"));
  Serial.println(F(__FILE__));
  Serial.println(F(__DATE__ " " __TIME__));
  Serial.println();

  // Configure Teleinfo Soft serial 
  // La téléinfo est connectee sur D3
  // ceci permet d'eviter les conflits avec la 
  // vraie serial lors des uploads
  Serial1.begin(1200);

  // Init teleinfo
  tinfo.init();

  // Attacher les callback dont nous avons besoin
  // pour cette demo, toutes
  tinfo.attachADPS(ADPSCallback);
  tinfo.attachData(DataCallback);
  tinfo.attachNewFrame(NewFrame);
  tinfo.attachUpdatedFrame(UpdatedFrame);

  // Show our not accurate second counter
  printUptime();
  Serial.println(F("Teleinfo started"));
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
 
  // On a reçu un caractère ?
  if ( Serial1.available() )
  {
    c = Serial1.read() ;
    //Serial.print(c & 0x7F);

    // Gerer
    tinfo.process(c);
  }

  // Verifier si le clignotement LED doit s'arreter 
  if (blinkLed && ((millis()-blinkLed) >= blinkDelay))
  {
    digitalWrite(LEDPIN, LOW);
    blinkLed = 0;
  }
}


