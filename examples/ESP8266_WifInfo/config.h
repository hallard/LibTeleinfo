// **********************************************************************************
// ESP8266 Teleinfo WEB Server configuration Include file
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
#ifndef __CONFIG_H__
#define __CONFIG_H__

// Include main project include file
#include "ESP8266_WifInfo.h"

#define CFG_MAX_SSID_SIZE 32
#define CFG_MAX_PASS_SIZE 32
#define CFG_MAX_HOSTNAME  16

// Mettez ici vos identifiant de connexion à
// votre réseau WIFI
#define DEFAULT_WIFI_SSID   "************"
#define DEFAULT_WIFI_PASS   "************"
#define DEFAULT_HOSTNAME    "WifInfo-esp01"

// En mode acces point 
#define DEFAULT_WIFI_AP_SSID  "WifInfo"
#define DEFAULT_WIFI_AP_PSK   "WifInfoPSK"

// Port pour l'OTA
#define DEFAULT_OTA_PORT     8266

// Bit definition for different configuration modes
#define CFG_LCD				  0x0001	// Enable display
#define CFG_DEBUG			  0x0002	// Enable serial debug
#define CFG_BAD_CRC     0x8000  // Bad CRC when reading configuration

// Config saved into eeprom
// 128 bytes total including CRC
typedef struct 
{
  char  ssid[CFG_MAX_SSID_SIZE]; /* SSID     */
  char  pass[CFG_MAX_PASS_SIZE]; /* Password */
  char  host[CFG_MAX_HOSTNAME];  /* Password */
  uint32_t config;               /* Bit field register */
  uint16_t ota_port;             /* OTA port */
  uint8_t  filler[40];           /* in case adding data in config avoiding loosing current conf by bad crc*/
  uint16_t crc;
} _Config;


// Exported variables/object instancied in main sketch
// ===================================================
extern _Config config;
 
// Declared exported function from route.cpp
// ===================================================
bool readConfig (void);
bool saveConfig (void);


#endif 

