// **********************************************************************************
// ESP8266 WifInfo WEB Server global Include file
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
#ifndef WIFINFO_H
#define WIFINFO_H

// Include Arduino header
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <Ticker.h>
//#include <WebSocketsServer.h>
//#include <Hash.h>
#include <NeoPixelBus.h>
#include <LibTeleinfo.h>
#include <FS.h>

extern "C" {
#include "user_interface.h"
}

#include "webserver.h"
#include "webclient.h"
#include "config.h"


#define DEBUG
#define DEBUG_SERIAL	Serial1
#define DEBUG_SERIAL1	

#define WIFINFO_VERSION "1.0.0"

// I prefix debug macro to be sure to use specific for THIS library
// debugging, this should not interfere with main sketch or other 
// libraries
#ifdef DEBUG
#define Debug(x)    DEBUG_SERIAL.print(x)
#define Debugln(x)  DEBUG_SERIAL.println(x)
#define DebugF(x)   DEBUG_SERIAL.print(F(x))
#define DebuglnF(x) DEBUG_SERIAL.println(F(x))
#define Debugf(...) DEBUG_SERIAL.printf(__VA_ARGS__)
#define Debugflush  DEBUG_SERIAL.flush
#else
#define Debug(x)    {}
#define Debugln(x)  {}
#define DebugF(x)   {}
#define DebuglnF(x) {}
#define Debugf(...) {}
#define Debugflush  {}
#endif

#define BLINK_LED_MS   50 // 50 ms blink
#define RGB_LED_PIN    14 
#define RED_LED_PIN    12

// value for HSL color
// see http://www.workwithcolor.com/blue-color-hue-range-01.htm
#define COLOR_RED             0
#define COLOR_ORANGE         30
#define COLOR_ORANGE_YELLOW  45
#define COLOR_YELLOW         60
#define COLOR_YELLOW_GREEN   90
#define COLOR_GREEN         120
#define COLOR_GREEN_CYAN    165
#define COLOR_CYAN          180
#define COLOR_CYAN_BLUE     210
#define COLOR_BLUE          240
#define COLOR_BLUE_MAGENTA  275
#define COLOR_MAGENTA	      300
#define COLOR_PINK		      350

// GPIO 1 TX on board blue led
#ifdef BLU_LED_PIN
#define LedBluON()  {digitalWrite(BLU_LED_PIN, 0);}
#define LedBluOFF() {digitalWrite(BLU_LED_PIN, 1);}
#else
#define LedBluON()  {}
#define LedBluOFF() {}
#endif
// GPIO 12 red led
#define LedRedON()  {digitalWrite(RED_LED_PIN, 1);}
#define LedRedOFF() {digitalWrite(RED_LED_PIN, 0);}

// Light off the RGB LED
#ifndef RGB_LED_PIN
#define LedRGBOFF() {}
#define LedRGBON(x) {}
#endif
// sysinfo informations
typedef struct 
{
  String sys_uptime;
} _sysinfo;

// Exported variables/object instancied in main sketch
// ===================================================
extern ESP8266WebServer server;
extern WiFiUDP OTA;
extern TInfo tinfo;
extern uint8_t rgb_brightness;
extern unsigned long seconds;
extern _sysinfo sysinfo;
extern Ticker Tick_emoncms;
extern Ticker Tick_jeedom;


// Exported function located in main sketch
// ===================================================
void ResetConfig(void);
void Task_emoncms();
void Task_jeedom();

#endif

