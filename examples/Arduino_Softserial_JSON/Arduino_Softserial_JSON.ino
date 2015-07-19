// **********************************************************************************
// Arduino Teleinfo sample, return JSON data of modified teleinfo values received
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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Arduino.h"
#include <SoftwareSerial.h>
#include <LibTeleinfo.h>

// Arduino on board LED
// I use moteino, so it's D9
// classic Arduino is D13
#define LEDPIN 9
//#define LEDPIN 13

// Teleinfo Serial (on D3)
SoftwareSerial Serial1(3,4); 

TInfo tinfo; // Teleinfo object

// Pour clignotement LED asynchrone
unsigned long blinkLed  = 0;
uint8_t blinkDelay= 0;

// Uptime timer
volatile boolean tick1sec=0;// one for interrupt, don't mess with 
unsigned long uptime=0; // save value we can use in sketch even if we're interrupted

// Used to indicate if we need to send all date or just modified ones
boolean fulldata = true;
 
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
  digitalWrite(LEDPIN, HIGH);
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
  digitalWrite(LEDPIN, HIGH);
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
          uint8_t c;
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
Function: TIMER VECTOR
Purpose : Interrupt that gets called once a second
Input   : -
Output  : - 
Comments: -
====================================================================== */
ISR(TIMER1_COMPA_vect)
{
  // got our second ticker
  tick1sec = true;
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
  // disable global interrupts
  cli(); 

  // set timer1 to fire event every second
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B

  OCR1A = 15624; // set compare match register to desired timer count. 16 MHz with 1024 prescaler = 15624 counts/s
  TCCR1B |= (1 << WGM12); // turn on CTC mode. clear timer on compare match
  TCCR1B |= (1 << CS10); // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

  // enable global interrupts
  sei(); 

  // Arduino LED
  pinMode(LEDPIN, OUTPUT);     
  digitalWrite(LEDPIN, LOW);

  // Serial, pour le debug
  Serial.begin(115200);

  // Configure Teleinfo Soft serial 
  // La téléinfo est connectee sur D3
  // ceci permet d'eviter les conflits avec la 
  // vraie serial lors des uploads
  Serial1.begin(1200);

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


