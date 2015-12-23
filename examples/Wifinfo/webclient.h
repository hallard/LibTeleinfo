// **********************************************************************************
// ESP8266 Teleinfo WEB Client routing Include file
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
// History : V1.00 2015-12-04 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#ifndef WEBCLIENT_H
#define WEBCLIENT_H

// Include main project include file
#include "Wifinfo.h"

// Exported variables/object instancied in main sketch
// ===================================================

// declared exported function from route.cpp
// ===================================================
boolean httpPost(char * host, uint16_t port, char * url);
boolean emoncmsPost(void);
boolean jeedomPost(void);

#endif
