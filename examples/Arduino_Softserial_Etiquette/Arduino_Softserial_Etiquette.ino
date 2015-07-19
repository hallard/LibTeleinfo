// **********************************************************************************
// Arduino Teleinfo sample, display information on teleinfo values received
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

SoftwareSerial Serial1(3,4); // Teleinfo Serial
TInfo          tinfo; // Teleinfo object

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
Function: setup
Purpose : Setup I/O and other one time startup stuff
Input   : -
Output  : - 
Comments: -
====================================================================== */
void setup()
{
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
  // pour cette demo, ici attach data
  tinfo.attachData(DataCallback);

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
  // Teleinformation processing
  if ( Serial1.available() ) {
    tinfo.process(Serial1.read());
  }
}


