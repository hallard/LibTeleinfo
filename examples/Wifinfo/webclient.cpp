// **********************************************************************************
// ESP8266 Teleinfo WEB Client, web server function
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// For any explanation about teleinfo ou use, see my blog
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

#include "webclient.h"

/* ======================================================================
Function: httpPost
Purpose : Do a http post
Input   : hostname
          port
          url
Output  : true if received 200 OK
Comments: -
====================================================================== */
boolean httpPost(char * host, uint16_t port, char * url)
{
  HTTPClient http;
  bool ret = false;

  unsigned long start = millis();

  // configure traged server and url
  http.begin(host, port, url); 
  //http.begin("http://emoncms.org/input/post.json?node=20&apikey=2f13e4608d411d20354485f72747de7b&json={PAPP:100}");
  //http.begin("emoncms.org", 80, "/input/post.json?node=20&apikey=2f13e4608d411d20354485f72747de7b&json={}"); //HTTP

  Debugf("http%s://%s:%d%s => ", port==443?"s":"", host, port, url);

  // start connection and send HTTP header
  int httpCode = http.GET();
  if(httpCode) {
      // HTTP header has been send and Server response header has been handled
      Debug(httpCode);
      Debug(" ");
      // file found at server
      if(httpCode == 200) {
        String payload = http.getString();
        Debug(payload);
        ret = true;
      }
  } else {
      DebugF("failed!");
  }
  Debugf(" in %d ms\r\n",millis()-start);
  return ret;
}

/* ======================================================================
Function: emoncmsPost
Purpose : Do a http post to emoncms
Input   : 
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean emoncmsPost(void)
{
  boolean ret = false;

  // Some basic checking
  if (*config.emoncms.host) {
    ValueList * me = tinfo.getList();
    // Got at least one ?
    if (me && me->next) {
      String url ; 
      boolean first_item;

      url = *config.emoncms.url ? config.emoncms.url : "/";
      url += "?";
      if (config.emoncms.node>0) {
        url+= F("node=");
        url+= String(config.emoncms.node);
        url+= "&";
      } 

      url += F("apikey=") ;
      url += config.emoncms.apikey;
      url += F("&json={") ;

      first_item = true;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;
        // First item do not add , separator
        if (first_item)
          first_item = false;
        else
          url += ",";

        url +=  me->name ;
        url += ":" ;

        // EMONCMS ne sais traiter que des valeurs numériques, donc ici il faut faire une 
        // table de mappage, tout à fait arbitraire, mais c"est celle-ci dont je me sers 
        // depuis mes débuts avec la téléinfo
        if (!strcmp(me->name, "OPTARIF")) {
          // L'option tarifaire choisie (Groupe "OPTARIF") est codée sur 4 caractères alphanumériques 
          /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
          je mets le 4eme char à 0, trop de possibilités
          BASE => Option Base. 
          HC.. => Option Heures Creuses. 
          EJP. => Option EJP. 
          BBRx => Option Tempo
          */
          char * p = me->value;
            
               if (*p=='B'&&*(p+1)=='A'&&*(p+2)=='S') url += "1";
          else if (*p=='H'&&*(p+1)=='C'&&*(p+2)=='.') url += "2";
          else if (*p=='E'&&*(p+1)=='J'&&*(p+2)=='P') url += "3";
          else if (*p=='B'&&*(p+1)=='B'&&*(p+2)=='R') url += "4";
          else url +="0";
        } else if (!strcmp(me->name, "HHPHC")) {
          // L'horaire heures pleines/heures creuses (Groupe "HHPHC") est codé par un caractère A à Y 
          // J'ai choisi de prendre son code ASCII
          int code = *me->value;
          url += String(code);
        } else if (!strcmp(me->name, "PTEC")) {
          // La période tarifaire en cours (Groupe "PTEC"), est codée sur 4 caractères 
          /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
          TH.. => Toutes les Heures. 
          HC.. => Heures Creuses. 
          HP.. => Heures Pleines. 
          HN.. => Heures Normales. 
          PM.. => Heures de Pointe Mobile. 
          HCJB => Heures Creuses Jours Bleus. 
          HCJW => Heures Creuses Jours Blancs (White). 
          HCJR => Heures Creuses Jours Rouges. 
          HPJB => Heures Pleines Jours Bleus. 
          HPJW => Heures Pleines Jours Blancs (White). 
          HPJR => Heures Pleines Jours Rouges. 
          */
               if (!strcmp(me->value, "TH..")) url += "1";
          else if (!strcmp(me->value, "HC..")) url += "2";
          else if (!strcmp(me->value, "HP..")) url += "3";
          else if (!strcmp(me->value, "HN..")) url += "4";
          else if (!strcmp(me->value, "PM..")) url += "5";
          else if (!strcmp(me->value, "HCJB")) url += "6";
          else if (!strcmp(me->value, "HCJW")) url += "7";
          else if (!strcmp(me->value, "HCJR")) url += "8";
          else if (!strcmp(me->value, "HPJB")) url += "9";
          else if (!strcmp(me->value, "HPJW")) url += "10";
          else if (!strcmp(me->value, "HPJR")) url += "11";
          else url +="0";
        } else {
          url += me->value;
        }
      } // While me

      // Json end
      url += "}";

      ret = httpPost( config.emoncms.host, config.emoncms.port, (char *) url.c_str()) ;
    } // if me
  } // if host
  return ret;
}

/* ======================================================================
Function: jeedomPost
Purpose : Do a http post to jeedom server
Input   : 
Output  : true if post returned 200 OK
Comments: -
====================================================================== */
boolean jeedomPost(void)
{
  boolean ret = false;

  // Some basic checking
  if (*config.jeedom.host) {
    ValueList * me = tinfo.getList();
    // Got at least one ?
    if (me && me->next) {
      String url ; 
      boolean skip_item;

      url = *config.jeedom.url ? config.jeedom.url : "/";
      url += "?";

      // Config identifiant forcée ?
      if (*config.jeedom.adco) {
        url+= F("ADCO=");
        url+= config.jeedom.adco;
        url+= "&";
      } 

      url += F("api=") ;
      url += config.jeedom.apikey;
      url += F("&") ;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;
        skip_item = false;

        // Si ADCO déjà renseigné, on le remet pas
        if (!strcmp(me->name, "ADCO")) {
          if (*config.jeedom.adco)
            skip_item = true;
        }

        // Si Item virtuel, on le met pas
        if (*me->name =='_')
          skip_item = true;

        // On doit ajouter l'item ?
        if (!skip_item) {
          url +=  me->name ;
          url += "=" ;
          url +=  me->value;
          url += "&" ;
        }
      } // While me

      ret = httpPost( config.jeedom.host, config.jeedom.port, (char *) url.c_str()) ;
    } // if me
  } // if host
  return ret;
}

