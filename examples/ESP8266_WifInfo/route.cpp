// **********************************************************************************
// ESP8266 Teleinfo WEB Server, route web function
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
#include "route.h"

/* ======================================================================
Function: handleRoot 
Purpose : handle main page /, display information
Input   : -
Output  : - 
Comments: -
====================================================================== */
void handleRoot(void) 
{
  String response="";

  // Just to debug where we are
  Debug(F("Serving / page..."));

  LedBluON();

  // start HTML Code
  response += F("<html><head>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<meta charset='UTF-8'>"
                "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css'>"
                "<link rel='stylesheet' href='//cdnjs.cloudflare.com/ajax/libs/bootstrap-table/1.8.1/bootstrap-table.min.css'>"
                // Our custom style
                "<style type='text/css'>"
                  ".nav-tabs {margin-top:4px;}"
                  ".fixed-table-body {height:auto;}"
                  ".progress {position:relative;}"
                  ".progress span {position:absolute;display:block;width:100%;color:black;}"
                "</style>"
                "<script src='http://code.jquery.com/jquery-2.1.3.min.js'></script>"
                "<script src='http://cdn.rawgit.com/Foliotek/AjaxQ/master/ajaxq.js'></script>"
                "<title>Wifinfo</title>"
                "</head><body>");

  response += F("<div class='container'>"
                  // Onglets 
                  "<ul class='nav nav-tabs' id='myTab'>"
                    "<li class='active'>"
                      "<a href='#tab_tinfo' data-toggle='tab'>"
                        "Téléinformation <span class='badge' id='scharge'></span>"
                      "</a>"
                    "</li>"
                    "<li><a href='#tab_sys' data-toggle='tab'>Système</a></li>"
                    "<li><a href='#tab_cfg' data-toggle='tab'>Configuration</a></li>"
                  "</ul>");
  
  // Contenu des onglets
  response += F("<div class='tab-content'>");
  
  // tab teleinfo
  response += F(  "<div class='tab-pane fade in active' id='tab_tinfo'>"
                    "<h4>Données de Téléinformation</h4>"
                    "<div><span style='float:left;width:18ex;text-align:center;'>Charge courante : </span>"
                    "<div class='progress'>"
                    "<div class='progress-bar progress-bar-success' style='width:0' id='pbar'>"
                    "<span class=show id='pcharge'>Attente des données</span>"
                    "</div></div></div>"
                    "<div id='toolbar'>"
                    "</div>"
                    "<table data-toggle='table' "
                           "data-toolbar='#toolbar'"
                           "data-url='/tinfojsontbl' "
                           "class='table table-striped' "
                           "data-show-refresh='true' "
                           "data-show-toggle='true' "
                           "data-show-columns='true' "
                           "data-search='true' "
                           "data-row-style='rowStyle' "
                           "id='tblinfo' "
                           "data-select-item-name='toolbar1'>"
                      "<thead>"
                        "<tr>"
                          "<th data-field='na' data-align='left' data-sortable='true' data-formatter='labelFormatter'>Etiquette</th>"
                          "<th data-field='va' data-align='left' data-sortable='true' data-formatter='valueFormatter'>Valeur</th>"
                          "<th data-field='ck' data-align='center'>Checksum</th>"
                          "<th data-field='fl' data-align='center' data-visible='false'>Flags</th>"
                        "</tr>"
                      "</thead>"
                    "</table>"
                  "</div>"); // tab pane
  // tab Systeme
  response += F(  "<div class='tab-pane fade' id='tab_sys'>"
                    "<h4>Données du système</h4>"
                    "<table data-toggle='table' "
                           "data-url='/sysjsontbl' "
                           "class='table table-striped' "
                           "data-show-refresh='true' "
                           "data-show-toggle='true' "
                           "data-search='true' "
                           "id='tblsys' "
                           "data-select-item-name='toolbar2'>"
                      "<thead>"
                        "<tr>"
                          "<th data-field='na' data-align='left'>Donnée</th>"
                          "<th data-field='va' data-align='left'>Valeur</th>"
                        "</tr>"
                      "</thead>"
                    "</table>"
                  "</div>"); // tab pane
  
  // tab Configuration
  response += F(  "<div class='tab-pane fade' id='tab_cfg'>"
                    "<h4>Configuration du module WifInfo</h4>"
                    "<p>Cette partie reste à faire, des volontaires motivés ?</p>"
                  "</div>"); // tab pane
  
  response += F("</div>"); // tab content 

/*
  response += F("<script>$( document ).ready(function() {"));
  response += F("$.getq('queue','/"));
  response += F("Uptime"));
  response += F("', function(data) { $('#"));
  response += F("Uptime"));
  response += F("').html(data."));
  response += F("Uptime"));
  response += F("); });")); 
  response += F("});</script>"));
*/

  response += F("</div>\r\n"); // Container
  response += F("<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js'></script>\r\n");
  response += F("<script src='//cdnjs.cloudflare.com/ajax/libs/bootstrap-table/1.8.1/bootstrap-table.min.js'></script>\r\n");
  response += F("<script src='//cdnjs.cloudflare.com/ajax/libs/bootstrap-table/1.8.1/locale/bootstrap-table-fr-FR.min.js'></script>\r\n");

  response += F("<script>" "\r\n"
                  "var counters={};"
                  "var isousc, iinst;"
                  "function rowStyle(row, index){" "\r\n"
                    "var classes=['active','success','info','warning','danger'];" "\r\n"
                    "var flags=parseInt(row.fl,10);" "\r\n"
                    "if (flags&0x80){return{classes:classes[4]};}""\r\n"
                    "if (flags&0x02){return{classes:classes[3]};}""\r\n"
                    "if (flags&0x08){return{classes:classes[1]};}""\r\n"
                    "return {};""\r\n"
                  "}""\r\n"
                  "function labelFormatter(value, row){" "\r\n"
                    "var flags=parseInt(row.fl,10);" "\r\n"
                    "if (typeof counters[value]==='undefined') counters[value]=1;" "\r\n"
                    "if (flags&0x88) counters[value]++;" "\r\n"
                    "return value + ' <span class=\"badge\">'+counters[value]+'</span>';"  "\r\n"
                  "}" "\r\n"
                  "function valueFormatter(value, row){" "\r\n"
                    "if (row.na==\"ISOUSC\")" "\r\n"
                      "isousc=parseInt(value);" "\r\n"
                    "else if (row.na==\"IINST\"){" "\r\n"
                      "var pb, pe, cl;" "\r\n"
                      "iinst=parseInt(value);" "\r\n"
                      "pe=parseInt(iinst*100/isousc);" "\r\n"
                      "if (isNaN(pe)) pe=0;" "\r\n"
                      "cl='success';" "\r\n"
                      "if (pe>70) cl ='info';" "\r\n"
                      "if (pe>80) cl ='warning';" "\r\n"
                      "if (pe>90) cl ='danger';" "\r\n"
                      "cl = 'progress-bar-' + cl;" "\r\n"
                      "if (pe>0) $('#scharge').text(pe+'%');" "\r\n"
                      "if (typeof isousc!='undefined')" "\r\n"
                      "  $('#pcharge').text(iinst+'A / '+isousc+'A');" "\r\n"
                      "$('#pbar').attr('class','progress-bar '+cl);" "\r\n"
                      "$('#pbar').css('width', pe+'%');"" \r\n"
                    "}"  "\r\n"
                    "return value;" "\r\n"       
                  "}" "\r\n"
                "</script>" "\r\n");

  response += F("<script>" "\r\n"
                  "var myTimer;" "\r\n"
                  "function myRefresh(){" "\r\n"    
                    "var id=$('.nav-tabs .active > a').attr('href');" "\r\n"
                    "if (id=='#tab_tinfo') id='#tblinfo';" "\r\n"
                    "if (id=='#tab_sys') id='#tblsys';" "\r\n"
                    #ifdef DEBUG
                    "console.log('Refreshing : '+id);" "\r\n"        
                    #endif
                    "clearInterval(myTimer);" "\r\n"
                    "$('#tblinfo').bootstrapTable('refresh',{silent: true});" "\r\n"
                    "if (id=='#tblsys')" "\r\n"
                      "$(id).bootstrapTable('refresh',{silent: true});" "\r\n"
                  "}""\r\n"

                  "$(function () {"  "\r\n"
                    "$('#tblinfo').on('load-success.bs.table', function (e, data) {" "\r\n"
                      #ifdef DEBUG
                      "console.log('Event: load-success.bs.table');" "\r\n"
                      #endif
                      "myTimer=setInterval(function(){myRefresh()},1000);" "\r\n"
                    "})" "\r\n"
                    ".on('load-error.bs.table', function (e, status) {" "\r\n"
                      #ifdef DEBUG
                      "console.log('Event: load-error.bs.table');" "\r\n"
                      #endif
                      "myTimer=setInterval(function(){myRefresh()},5000);" "\r\n"
                    "})" "\r\n"
                  "});" "\r\n"
                "</script>"); 

  response += F("</body></html>\r\n");

  // Just to debug buffer free size
  Debug(F("sending "));
  Debug(response.length());
  Debug(F(" bytes..."));

  // Send response to client
  server.send ( 200, "text/html", response );

  Debug(F("OK!"));

  LedBluOFF();
}

/* ======================================================================
Function: formatNumberJSON 
Purpose : check if data value is full number and send correct JSON format
Input   : String where to add response
          char * value to check 
Output  : - 
Comments: 00150 => 150
          ADCO  => "ADCO"
          1     => 1
====================================================================== */
void formatNumberJSON( String &response, char * value)
{
  // we have at least something ?
  if (value && strlen(value))
  {
    boolean isNumber = true;
    uint8_t c;
    char * p = value;

    // just to be sure
    if (strlen(p)<=16) {
      // check if value is number
      while (*p && isNumber) {
        if ( *p < '0' || *p > '9' )
          isNumber = false;
        p++;
      }

      // this will add "" on not number values
      if (!isNumber) {
        response += '\"' ;
        response += value ;
        response += F("\":") ;
      } else {
        // this will remove leading zero on numbers
        p = value;
        while (*p=='0' && *(p+1) )
          p++;
        response += p ;
      }
    } else {
      Debugln(F("formatNumberJSON error!"));
    }
  }
}


/* ======================================================================
Function: tinfoJSONTable 
Purpose : dump all teleinfo values in JSON table format for browser
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: -
====================================================================== */
void tinfoJSONTable(void)
{
  ValueList * me = tinfo.getList();
  String response = "";

  // Just to debug where we are
  Debug(F("Serving /tinfoJSONTable page...\r\n"));

  // Got at least one ?
  if (me) {
    uint8_t index=0;

    boolean first_item = true;
    // Json start
    response += F("[\r\n");

    // Loop thru the node
    while (me->next) {

      // we're there
      ESP.wdtFeed();

      // go to next node
      me = me->next;

      // First item do not add , separator
      if (first_item)
        first_item = false;
      else
        response += F(",\r\n");

      Debug(F("(")) ;
      Debug(++index) ;
      Debug(F(") ")) ;

      if (me->name) Debug(me->name) ;
      else Debug(F("NULL")) ;

      Debug(F("=")) ;

      if (me->value) Debug(me->value) ;
      else Debug(F("NULL")) ;

      Debug(F(" '")) ;
      Debug(me->checksum) ;
      Debug(F("' ")); 

      // Flags management
      if ( me->flags) {
        Debug(F("Flags:0x")); 
        Debugf("%02X => ", me->flags); 
        if ( me->flags & TINFO_FLAGS_EXIST)
          Debug(F("Exist ")) ;
        if ( me->flags & TINFO_FLAGS_UPDATED)
          Debug(F("Updated ")) ;
        if ( me->flags & TINFO_FLAGS_ADDED)
          Debug(F("New ")) ;
      }

      Debugln() ;

      response += F("{\"na\":\"");
      response +=  me->name ;
      response += F("\", \"va\":\"") ;
      response += me->value;
      response += F("\", \"ck\":\"") ;
      if (me->checksum == '"' || me->checksum == '\\' || me->checksum == '/')
        response += '\\';
      response += (char) me->checksum;
      response += F("\", \"fl\":");
      response += me->flags ;
      response += '}' ;

    }
   // Json end
   response += F("\r\n]");

  } else {
    Debugln(F("sending 404..."));
    server.send ( 404, "text/plain", "No data" );
  }
  Debug(F("sending..."));
  server.send ( 200, "text/json", response );
  Debugln(F("OK!"));
}

/* ======================================================================
Function: sysJSONTable 
Purpose : dump all sysinfo values in JSON table format for browser
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: -
====================================================================== */
void sysJSONTable()
{
  String response = "";

  // Just to debug where we are
  Debug(F("Serving /sysJSONTable page..."));

  // Json start
  response += F("[\r\n");

  response += "{\"na\":\"Uptime\",\"va\":\"";
  response += sysinfo.sys_uptime;
  response += "\"},\r\n";

  response += "{\"na\":\"Compile le\",\"va\":\"" __DATE__ " " __TIME__ "\"},\r\n";
  response += "{\"na\":\"Free Ram\",\"va\":\"";
  response += sysinfo.sys_free_ram;
  response += "\"},\r\n";

  response += "{\"na\":\"Flash Size\",\"va\":\"";
  response += sysinfo.sys_flash_size ;
  response += "\"},\r\n";

  response += "{\"na\":\"Firmware Size\",\"va\":\"";
  response += sysinfo.sys_firmware_size;
  response += "\"},\r\n";

  response += "{\"na\":\"Free Size\",\"va\":\"";
  response += sysinfo.sys_firmware_free;
  response += "\"},\r\n";

  response += "{\"na\":\"Wifi SSID\",\"va\":\"";
  response += config.ssid;
  response += "\"},\r\n";

  response += "{\"na\":\"OTA Network Port\",\"va\":";
  response += config.ota_port ;
  response += "},\r\n";

  response += "{\"na\":\"Wifi RSSI\",\"va\":\"";
  response += WiFi.RSSI() ;
  response += " dB\"}\r\n";

  // Json end
  response += F("]\r\n");

  Debug(F("sending..."));
  server.send ( 200, "text/json", response );
  Debugln(F("Ok!"));
}

/* ======================================================================
Function: sendJSON 
Purpose : dump all values in JSON
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: -
====================================================================== */
void sendJSON(void)
{
  ValueList * me = tinfo.getList();
  String response = "";
  
  // Got at least one ?
  if (me) {
    // Json start
    response += F("{\"_UPTIME\":");
    response += seconds;

    // Loop thru the node
    while (me->next) {

      // we're there
      ESP.wdtFeed();

      // go to next node
      me = me->next;

      response += F(",\"") ;
      response += me->name ;
      response += F("\":") ;
      formatNumberJSON(response, me->value);
    }
   // Json end
   response += F("}\r\n") ;

  } else {
    server.send ( 404, "text/plain", "No data" );
  }
  server.send ( 200, "text/json", response );
}

/* ======================================================================
Function: handleNotFound 
Purpose : default WEB routing when URI is not found
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: We search is we have a name that match to this URI, if one we
          return it's pair name/value in json
====================================================================== */
void handleNotFound(void) 
{
  String response = "";

  // We check for an known label
  ValueList * me = tinfo.getList();
  const char * uri;
  boolean found = false;

  // Led on
  LedBluON();
  
  // convert uri to char * for compare
  uri = server.uri().c_str();

  Debugf("handleNotFound(%s)\r\n", uri);

  // Got at least one and consistent URI ?
  if (me && uri && *uri=='/' && *++uri ) {
    
    // Loop thru the linked list of values
    while (me->next && !found) {

      // we're there
      ESP.wdtFeed();

      // go to next node
      me = me->next;

      //Debugf("compare to '%s' ", me->name);
      // Do we have this one ?
      if (stricmp (me->name, uri) == 0 )
      {
        // no need to continue
        found = true;

        // Add to respone
        response += F("{\"") ;
        response += me->name ;
        response += F("\":") ;
        formatNumberJSON(response, me->value);
        response += F("}\r\n");
      }
    }
  }

  // Got it, send json
  if (found) {
    server.send ( 200, "text/json", response );
  } else {    
    // send error message in plain text
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for ( uint8_t i = 0; i < server.args(); i++ ) {
      message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
    }

    server.send ( 404, "text/plain", message );
  }

  // Led off
  LedBluOFF();
}

