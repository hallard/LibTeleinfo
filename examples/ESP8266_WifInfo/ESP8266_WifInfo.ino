// **********************************************************************************
// ESP8266 Teleinfo WEB Server
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

// Global project file
#include "ESP8266_WifInfo.h"


// To be able to read VCC from ESP SDK
ADC_MODE(ADC_VCC);

//WiFiManager wifi(0);
ESP8266WebServer server(80);
// Udp listener and telnet server
WiFiUDP OTA;
// Teleinfo
TInfo tinfo;
// RGB Loed
NeoPixelBus rgb_led = NeoPixelBus(1, RGB_LED_PIN, NEO_RGB | NEO_KHZ800);

// define whole brigtness level for RGBLED
uint8_t rgb_brightness = 127;
// LED Blink timers
Ticker rgb_ticker;
Ticker blu_ticker;
Ticker red_ticker;
Ticker Every_1_Sec;

volatile boolean task_1_sec = false;
unsigned long seconds = 0;

// sysinfo data
_sysinfo sysinfo;

/* ======================================================================
Function: UpdateSysinfo 
Purpose : update sysinfo variables
Input   : true if first call
          true if needed to print on serial debug
Output  : - 
Comments: -
====================================================================== */
void UpdateSysinfo(boolean first_call, boolean show_debug)
{
  char buff[64];
  int sec = seconds;
  int min = sec / 60;
  int hr = min / 60;

  sprintf( buff, "%02d:%02d:%02d", hr, min % 60, sec % 60);
  sysinfo.sys_uptime = buff;

  sprintf( buff, "%d KB", ESP.getFreeHeap()/1024 );
  sysinfo.sys_free_ram = buff;

  sprintf( buff, "%d mV", ESP.getVcc());
  sysinfo.sys_vcc = buff;
  
  // Values not subject to change during running sketch
  if (first_call) {
    sprintf( buff, "%d KB", ESP.getFlashChipRealSize()/1024 );
    sysinfo.sys_flash_real_size = buff;
    sprintf( buff, "%d KB", ESP.getSketchSize()/1024 );
    sysinfo.sys_firmware_size = buff;
    sprintf( buff, "%d KB", ESP.getFreeSketchSpace()/1024 );
    sysinfo.sys_firmware_free = buff;
    sprintf( buff, "%d MHZ", ESP.getFlashChipSpeed()/1000 );
    sysinfo.sys_flash_speed = buff;
  }

  if (show_debug) {
    Debug(F("Firmware     : ")); Debugln(__DATE__ " " __TIME__);
    Debug(F("Flash real id: ")); Serial1.printf("0x%08X\r\n", ESP.getFlashChipId());
    Debug(F("Flash RSize  : ")); Debugln(sysinfo.sys_flash_real_size);  
    Debug(F("CPU Speed    : ")); Debugln(sysinfo.sys_flash_speed); 
    Debug(F("Sketch size  : ")); Debugln(sysinfo.sys_firmware_size);     
    Debug(F("Free size    : ")); Debugln(sysinfo.sys_firmware_free);
    Debug(F("Free RAM     : ")); Debugln(sysinfo.sys_free_ram);       
    Debug(F("OTA port     : ")); Debugln(config.ota_port);          
    Debug(F("VCC          : ")); Debugln(sysinfo.sys_vcc);          
    Debug(F("Saved Config : ")); Debugln(sysinfo.sys_eep_config);          
  }
}

/* ======================================================================
Function: Task_1_Sec 
Purpose : update our second ticker
Input   : -
Output  : - 
Comments: -
====================================================================== */
void Task_1_Sec()
{
  task_1_sec = true;
  seconds++;
}

/* ======================================================================
Function: LedOff 
Purpose : callback called after led blink delay
Input   : led (defined in term of PIN)
Output  : - 
Comments: -
====================================================================== */
void LedOff(int led)
{
  if (led==BLU_LED_PIN)
    LedBluOFF();
  if (led==RED_LED_PIN)
    LedRedOFF();
  if (led==RGB_LED_PIN)
    LedRGBOFF();
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
  // Monophasé
  if (phase == 0 ) {
    Debugln(F("ADPS"));
  } else {
    Debug(F("ADPS Phase "));
    Debugln('0' + phase);
  }
}

/* ======================================================================
Function: DataCallback 
Purpose : callback when we detected new or modified data received
Input   : linked list pointer on the concerned data
          value current state being TINFO_VALUE_ADDED/TINFO_VALUE_UPDATED
Output  : - 
Comments: -
====================================================================== */
void DataCallback(ValueList * me, uint8_t flags)
{

  // This is for simulating ADPS during my tests
  // ===========================================
  /*
  static uint8_t test = 0;
  // Each new/updated values
  if (++test >= 20) {
    test=0;
    uint8_t anotherflag = TINFO_FLAGS_NONE;
    ValueList * anotherme = tinfo.addCustomValue("ADPS", "46", &anotherflag);

    // Do our job (mainly debug)
    DataCallback(anotherme, anotherflag);
  }
  Debugf("%02d:",test);
  */
  // ===========================================
  

  // Do whatever you want there
  Debug(me->name);
  Debug('=');
  Debug(me->value);
  
  if ( flags & TINFO_FLAGS_NOTHING ) Debug(F(" Nothing"));
  if ( flags & TINFO_FLAGS_ADDED )   Debug(F(" Added"));
  if ( flags & TINFO_FLAGS_UPDATED ) Debug(F(" Updated"));
  if ( flags & TINFO_FLAGS_EXIST )   Debug(F(" Exist"));
  if ( flags & TINFO_FLAGS_ALERT )   Debug(F(" Alert"));

  Debugln();
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
  char buff[32];

  // Light the RGB LED 
  LedRGBON(COLOR_GREEN);

  sprintf( buff, "New Frame (%ld Bytes free)", ESP.getFreeHeap() );
  Debugln(buff);

  // led off after delay
  rgb_ticker.once_ms( (uint32_t) BLINK_LED_MS, LedOff, (int) RGB_LED_PIN);
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
  char buff[32];

  // Light the RGB LED (purple)
  LedRGBON(COLOR_MAGENTA);

  sprintf( buff, "Updated Frame (%ld Bytes free)", ESP.getFreeHeap() );
  Debugln(buff);

  // led off after delay
  rgb_ticker.once_ms(BLINK_LED_MS, LedOff, RGB_LED_PIN);
}

/* ======================================================================
Function: CheckOTAUpdate 
Purpose : Check if we need to update the firmware over the Air
Input   : -
Output  : - 
Comments: If upgraded, no return, perform update and reboot ESP
====================================================================== */
void CheckOTAUpdate(void)
{
  bool spiffs = false;

  //OTA detection
  if (OTA.parsePacket()) {
    IPAddress remote = OTA.remoteIP();
    int cmd  = OTA.parseInt();
    int port = OTA.parseInt();
    int size = OTA.parseInt();

    LedRGBON(COLOR_MAGENTA);

    DebugF("OTA received command ");
    Debugln(cmd);
    if (cmd == U_SPIFFS) {
      spiffs = true;
      DebugF("Get SPIFFS image");
    }


    DebugF("Update Start: ip:");
    Debug(remote);
    Debugf(", port:%d, size:%dKB\n", port, size/1024);
    uint32_t startTime = millis();

    WiFiUDP::stopAll();

    if(!Update.begin(size, cmd)) {
      DebugF("Update Begin Error");
      return;
    }

    WiFiClient ota_client;

    if (ota_client.connect(remote, port)) {

      uint32_t written;
      while(!Update.isFinished()) {
        written = Update.write(ota_client);
        if(written > 0) 
        {
          LedRGBOFF();
          ota_client.print(written, DEC);
          LedRGBON(COLOR_MAGENTA);
        }
      }
      LedRGBOFF();
      Serial.setDebugOutput(false);

      if(Update.end()) {
        ota_client.println("OK");
        Debugf("Update Success: %u\nRebooting...\n", millis() - startTime);
        ESP.restart();
      } else {
        Update.printError(ota_client);
        Update.printError(Serial);
      }
    } else {
      Debugf("Connect Failed: %u\n", millis() - startTime);
    }
 
    // Be sure to re enable it
    OTA.begin(DEFAULT_OTA_PORT);
  }
}

/* ======================================================================
Function: WifiSoftAP
Purpose : Change Wifi mode to Soft AP
Input   : -
Output  : state of the wifi status
Comments: -
====================================================================== */
int WifiSoftAP() 
{
}

/* ======================================================================
Function: WifiHandleConn
Purpose : Handle Wifi connection / reconnection and OTA updates
Input   : setup true if we're called 1st Time from setup
Output  : state of the wifi status
Comments: -
====================================================================== */
int WifiHandleConn(boolean setup = false) 
{
  int ret ;

  if (setup)
  {
    // Check WiFi connection mode, at startup
    // try to connect to AP
    if (WiFi.getMode()!=WIFI_STA) {
      WiFi.mode(WIFI_STA);
      delay(10);
    }

    // Get Wifi Status
    ret = WiFi.status();

    // Try to get 1st connexion
    if ( ret != WL_CONNECTED ) {

      // Orange we're not connected anymore
      LedRGBON(COLOR_ORANGE);

      DebugF("Connecting to: "); 
      Debug(DEFAULT_WIFI_SSID);
      Debug(F("..."));

      WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);

      ret = WiFi.waitForConnectResult();
      if ( ret != WL_CONNECTED) {
        LedRGBON(COLOR_RED);
        DebuglnF("Connection failed!");
      } else {
        LedRGBON(COLOR_GREEN);
        DebuglnF("Connected");
        DebugF("IP address   : "); Debugln(WiFi.localIP());
        DebugF("MAC address  : "); Debugln(WiFi.macAddress());

        // just in case your sketch sucks, keep update OTA Available
        // Trust me, when coding and testing it happens, this could save
        // the need to connect FTDI to reflash
        // Usefull just after 1st connexion when called from setup() before
        // launching potentially bugging main()
        for (uint8_t i=0; i<= 10; i++) {
          LedRGBON(COLOR_MAGENTA);
          delay(100);
          LedRGBOFF();
          delay(200);
          CheckOTAUpdate();
        }
      }
    }

    // We did not succeded to connect ?
    if ( ret != WL_CONNECTED ) {
      uint8_t mac[WL_MAC_ADDR_LENGTH];
      char ssid[32];

      // start Soft AP
      DebuglnF("Starting Soft AP mode");
      WiFi.mode(WIFI_AP);

      // Add the last two bytes of the MAC address to AP name
      WiFi.softAPmacAddress(mac);
      sprintf_P(ssid, PSTR("%s_%02X%02X"), DEFAULT_WIFI_AP_SSID, mac[4], mac[5] );

      DebugF("SSID : "); Debugln(ssid);
      //WiFi.softAP(ssid, DEFAULT_WIFI_AP_PSK);
      //DebuglnF("PSK  : " DEFAULT_WIFI_AP_PSK);
      // No password
      WiFi.softAP(ssid);
      DebugF("IP   : ");
      Debugln(WiFi.softAPIP());
    }

    // Advertise US for Arduino IDE
    // not very usefull on Windows (IDE does not always sees us)
    MDNS.begin(DEFAULT_HOSTNAME);
    MDNS.addService("arduino", "tcp", DEFAULT_OTA_PORT);

    // Setup OTA feature
    OTA.begin(DEFAULT_OTA_PORT);
  }

  // Handle OTA 
  CheckOTAUpdate();

  return ret;
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
  boolean reset_config = true;

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  //WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  //delay(100);

  // Init the RGB Led, and set it off
  rgb_led.Begin();
  LedRGBOFF();

  // Init the serial 1
  // Teleinfo is connected to RXD2 (GPIO13) to 
  // avoid conflict when flashing, this is why
  // we swap RXD1/RXD1 to RXD2/TXD2 
  // Note that TXD2 is not used teleinfo is receive only
  Serial.begin(1200, SERIAL_7E1);
  Serial.swap();

  // Our Debug Serial TXD0
  // note this serial can only transmit, just 
  // enough for debugging purpose
  Serial1.begin(115200);

  Debugln(F("=============="));
  Debugln(F("Wifinfo V1.0.1"));
  Debugln();
  Debugflush();

  // Clear our global flags
  config.config = 0;

  // Our configuration is stored into EEPROM
  EEPROM.begin(sizeof(_Config));

  // Read Configuration from EEP
  if (readConfig()) {
    // in case of saved default config it's not good
    if (config.ssid[0]!='*' && config.pass[0]!='*' ) {
      sysinfo.sys_eep_config = PSTR("Good CRC, config OK");
      reset_config = false;
    } else {
      sysinfo.sys_eep_config = PSTR("Good CRC, not set!");
    }
  } 

  if (reset_config ) {
    // Error, enable default configuration
    strcpy_P(config.ssid, PSTR(DEFAULT_WIFI_SSID));
    strcpy_P(config.pass, PSTR(DEFAULT_WIFI_PASS));
    strcpy_P(config.host, PSTR(DEFAULT_HOSTNAME));
    config.ota_port = DEFAULT_OTA_PORT ;

    // Indicate the error in global flags
    config.config |= CFG_BAD_CRC;
    sysinfo.sys_eep_config = PSTR("Reset to default");

    // save back
    saveConfig();
  }

  Debugln(sysinfo.sys_eep_config);
  
  // Init teleinfo
  tinfo.init();

  // Attach the callback we need
  // set all as an example
  tinfo.attachADPS(ADPSCallback);
  tinfo.attachData(DataCallback);
  tinfo.attachNewFrame(NewFrame);
  tinfo.attachUpdatedFrame(UpdatedFrame);

  // We'll drive the on board LED (TXD1) and our on GPIO1
  // old TXD1, not used anymore, has been swapped
  pinMode(BLU_LED_PIN, OUTPUT); 
  pinMode(RED_LED_PIN, OUTPUT); 
  LedRedOFF();
  LedBluOFF();

  // start Wifi connect or soft AP
  WifiHandleConn(true);

  // Update sysinfor variable and print them
  UpdateSysinfo(true, true);

  server.on("/", handleRoot);
  server.on("/json", sendJSON);
  server.on("/tinfojsontbl", tinfoJSONTable);
  server.on("/sysjsontbl", sysJSONTable);
  server.onNotFound(handleNotFound);
  server.begin();

  Debugln(F("HTTP server started"));

  //webSocket.begin();
  //webSocket.onEvent(webSocketEvent);

  // Light off the RGB LED
  LedRGBOFF();

  // control watchdog
  //ESP.wdtEnable();
  //ESP.wdtDisable()
  //ESP.wdtFeed(); 

  // Update sysinfo every second
  Every_1_Sec.attach(1, Task_1_Sec);
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
  char c;

  // Do all related network stuff
  server.handleClient();

  //webSocket.loop();

  // 1 second task job ?
  if (task_1_sec)  {
    UpdateSysinfo(false, false);
    task_1_sec = false;
  }

  // Handle teleinfo serial
  if ( Serial.available() ) {
    // Read Serial and process to tinfo
    c = Serial.read();
    //Serial1.print(c);
    tinfo.process(c);
  }
}

