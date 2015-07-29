// **********************************************************************************
// ESP8266 Teleinfo WEB Server routing Include file
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
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#ifndef ROUTE_H
#define ROUTE_H

// Include main project include file
#include "ESP8266_WifInfo.h"

// Web response max size
#define RESPONSE_BUFFER_SIZE 4096

// Exported variables/object instancied in main sketch
// ===================================================
extern char response[];
extern uint16_t response_idx;

// declared exported function from route.cpp
// ===================================================
void handleTest(void);
void handleRoot(void); 
void handleNotFound(void);
void tinfoJSONTable(void);
void sysJSONTable(void);
void sendJSON(void);


#endif
