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
#include <ESP8266mDNS.h>
#include <WiFiUDP.h>
#include <EEPROM.h>
#include <Ticker.h>
//#include <WebSocketsServer.h>
//#include <Hash.h>
#include <NeoPixelBus.h>
#include <LibTeleinfo.h>

#include "route.h"
#include "config.h"


#define DEBUG

// I prefix debug macro to be sure to use specific for THIS library
// debugging, this should not interfere with main sketch or other 
// libraries
#ifdef DEBUG
#define Debug(x)    Serial1.print(x)
#define Debugln(x)  Serial1.println(x)
#define DebugF(x)   Serial1.print(F(x))
#define DebuglnF(x) Serial1.println(F(x))
#define Debugf(...) Serial1.printf(__VA_ARGS__)
#define Debugflush  Serial1.flush
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
#define BLU_LED_PIN     1
#define RED_LED_PIN    12
// value for RGB color
#define COLOR_RED     rgb_brightness, 0, 0
#define COLOR_ORANGE  rgb_brightness, rgb_brightness>>1, 0
#define COLOR_YELLOW  rgb_brightness, rgb_brightness, 0
#define COLOR_GREEN   0, rgb_brightness, 0
#define COLOR_CYAN    0, rgb_brightness, rgb_brightness
#define COLOR_BLUE    0, 0, rgb_brightness
#define COLOR_MAGENTA rgb_brightness, 0, rgb_brightness

// GPIO 1 TX on board blue led
#define LedBluON()  {digitalWrite(BLU_LED_PIN, 0);}
#define LedBluOFF() {digitalWrite(BLU_LED_PIN, 1);}
// GPIO 12 red led
#define LedRedON()  {digitalWrite(RED_LED_PIN, 1);}
#define LedRedOFF() {digitalWrite(RED_LED_PIN, 0);}

  // Light off the RGB LED
#define LedRGBOFF() { rgb_led.SetPixelColor(0,0,0,0); rgb_led.Show(); }
#define LedRGBON(x) { rgb_led.SetPixelColor(0,x); rgb_led.Show(); }

// sysinfo informations
typedef struct 
{
  String sys_uptime;
  String sys_free_ram;
  String sys_flash_size;
  String sys_flash_speed;
  String sys_firmware_size;
  String sys_firmware_free;
  String sys_vcc;
  String sys_eep_config;
} _sysinfo;

// Exported variables/object instancied in main sketch
// ===================================================
extern ESP8266WebServer server;
extern WiFiUDP OTA;
extern TInfo tinfo;
extern NeoPixelBus rgb_led ;
extern uint8_t rgb_brightness;
extern unsigned long seconds;
extern _sysinfo sysinfo;


#endif

