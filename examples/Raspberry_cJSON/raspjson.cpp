// **********************************************************************************
// Raspberry PI LibTeleinfo sample, display JSON data of modified teleinfo values received
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
// Written by Charles-Henri Hallard (https://hallard.me)
//
// History : V1.00 2015-07-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <termios.h>
#include <getopt.h>
#include <sys/sysinfo.h>
#include "../../src/LibTeleinfo.h"

#include "cJSON.h"
#include <curl/curl.h>

// ----------------
// Constants
// ----------------
#define true 1
#define false 0

#define PRG_DIR    "/usr/local/bin" 
#define PRG_NAME   "raspjson"
#define TELEINFO_DEVICE   ""
#define TELEINFO_BUFSIZE  512

#define HTTP_APIKEY_SIZE  64    // Max size of apikey
#define HTTP_NODE_SIZE    64    // Max size of node
#define HTTP_URL_SIZE     128   // Max size of url (url only, not containing posted data)
#define HTTP_BUFFER_SIZE  1024  // Where http returned data will be filled

// Some enum for serial
enum parity_e     {  P_NONE,  P_EVEN,    P_ODD };
enum flowcntrl_e  { FC_NONE,  FC_RTSCTS, FC_XONXOFF };
enum value_e      { VALUE_NOTHING, VALUE_ADDED, VALUE_EXIST, VALUE_CHANGED};

// Configuration structure
static struct 
{
  char port[128];
  int baud;
  enum flowcntrl_e flow;
  char flow_str[32];
  enum parity_e parity;
  char parity_str[32];
  int databits;
  int mode;
  int verbose;
  int emoncms;
  char node[HTTP_NODE_SIZE];
  char url[HTTP_URL_SIZE];
  char apikey[HTTP_APIKEY_SIZE];
  int daemon;
// Configuration structure defaults values
} opts;

void log_syslog( FILE * stream, const char *format, ...);
void sendJSON(ValueList * me, bool all);


// ======================================================================
// Global vars 
// ======================================================================
int   g_fd_teleinfo;          // teleinfo serial handle
struct termios g_oldtermios ; // old serial config
int   g_exit_pgm;             // indicate en of the program
struct sysinfo g_info;
TInfo tinfo;                  // Teleinfo object

CURL *g_pcurl;
char  http_buffer[HTTP_BUFFER_SIZE];  // Where http returned data will be filled

// Used to indicate if we need to send all date or just modified ones
bool fulldata = true;
 
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
  printf( "{\"ADPS\":%c}\r\n",'0' + phase);
  fflush(stdout);
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
  // Envoyer les valeurs 
  sendJSON(me, fulldata);
  fulldata = false;
}

/* ======================================================================
Function: tlf_treat_label
Purpose : do action when received a correct label / value + checksum line
Input   : plabel : pointer to string containing the label
        : pvalue : pointer to string containing the associated value
Output  : 
Comments: 
====================================================================== */
void tlf_treat_label( char * plabel, char * pvalue) 
{
  // emoncms need only numeric values
  if (opts.emoncms)
  {
    if (strcmp(plabel, "OPTARIF")==0 )
    {
      // L'option tarifaire choisie (Groupe "OPTARIF") est codée sur 4 caractères alphanumériques 
      /* J'ai pris un nombre arbitraire codé dans l'ordre ci-dessous
      je mets le 4eme char à 0, trop de possibilités
      BASE => Option Base. 
      HC.. => Option Heures Creuses. 
      EJP. => Option EJP. 
      BBRx => Option Tempo
      */
      pvalue[3] = '\0';
        
           if (strcmp(pvalue, "BAS")==0 ) strcpy (pvalue, "1");
      else if (strcmp(pvalue, "HC.")==0 ) strcpy (pvalue, "2");
      else if (strcmp(pvalue, "EJP")==0 ) strcpy (pvalue, "3");
      else if (strcmp(pvalue, "BBR")==0 ) strcpy (pvalue, "4");
      else strcpy (pvalue, "0");
    }
    else if (strcmp(plabel, "HHPHC")==0 )
    {
      // L'horaire heures pleines/heures creuses (Groupe "HHPHC") est codé par un caractère A à Y 
      // J'ai choisi de prendre son code ASCII
      int code = *pvalue;
      sprintf(pvalue, "%d", code);
    }
    else if (strcmp(plabel, "DEMAIN")==0 )
    {
      if (strcmp(pvalue, "----")==0 ) strcpy (pvalue, "0");
    }
    else if (strcmp(plabel, "PTEC")==0 )
    {
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
           if (strcmp(pvalue, "TH..")==0 ) strcpy (pvalue, "1");
      else if (strcmp(pvalue, "HC..")==0 ) strcpy (pvalue, "2");
      else if (strcmp(pvalue, "HP..")==0 ) strcpy (pvalue, "3");
      else if (strcmp(pvalue, "HN..")==0 ) strcpy (pvalue, "4");
      else if (strcmp(pvalue, "PM..")==0 ) strcpy (pvalue, "5");
      else if (strcmp(pvalue, "HCJB")==0 ) strcpy (pvalue, "6");
      else if (strcmp(pvalue, "HCJW")==0 ) strcpy (pvalue, "7");
      else if (strcmp(pvalue, "HCJR")==0 ) strcpy (pvalue, "8");
      else if (strcmp(pvalue, "HPJB")==0 ) strcpy (pvalue, "9");
      else if (strcmp(pvalue, "HPJW")==0 ) strcpy (pvalue, "10");
      else if (strcmp(pvalue, "HPJR")==0 ) strcpy (pvalue, "11");
      else strcpy (pvalue, "0");
      
    }
  }
}

/* ======================================================================
Function: http_write
Purpose : callback function when curl write return data
Input   : see curl API
Output  : -
Comments: -
===================================================================== */
size_t http_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  // clean up our own receive buffer
  bzero(&http_buffer, HTTP_BUFFER_SIZE); 

  /* Copy curl's received data into our own buffer */
  if (size*nmemb < HTTP_BUFFER_SIZE - 1 )
  {
    memcpy(&http_buffer, ptr, size*nmemb);
    return (size*nmemb);
  }
  else
  {
    memcpy(&http_buffer, ptr, HTTP_BUFFER_SIZE - 1);
    return (HTTP_BUFFER_SIZE);
  }
}

/* ======================================================================
Function: http_post
Purpose : post data to emoncms
Input   : full url to post data
Output  : true if emoncms returned ok, false otherwise
Comments: we don't exit if not working, neither mind, take it next time
===================================================================== */
int http_post( char * str_url )
{
  CURLcode res;
  int retcode = false;

  // Set curl URL 
  if ( curl_easy_setopt(g_pcurl, CURLOPT_URL, str_url) != CURLE_OK )
    log_syslog(stderr, "Error while setting curl url %s : %s", str_url, curl_easy_strerror(res));
  else
  {
    // Perform the request, res will get the return code 
    if( (res = curl_easy_perform(g_pcurl)) != CURLE_OK)
    {
      log_syslog(stderr, "Error on http request %s : %s", str_url, curl_easy_strerror(res));
    }
    else
    { 
      // return data received 
      if (opts.verbose)
        log_syslog(stdout, "http_post %s ==> '%s'\n", str_url, http_buffer);  
        
      // emoncms returned string "ok", all went fine
      if (strcmp(http_buffer, "ok") == 0 )
      {
        retcode = true;
      }
    }
  }
  return retcode;
}

/* ======================================================================
Function: sendJSON 
Purpose : dump teleinfo values on serial
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: -
====================================================================== */
void sendJSON(ValueList * me, bool all)
{
    static char emoncms_url[1024];
    char *string = NULL;

    if (me) {
        cJSON *uptime = NULL;
        cJSON *me_value = NULL;

        cJSON *trame_json = cJSON_CreateObject();
        if (trame_json == NULL)
        {
            goto end;
        }
    
        if (all) {
            uptime = cJSON_CreateNumber(g_info.uptime);
            if (uptime == NULL)
            {
              goto end;
            }
            cJSON_AddItemToObject(trame_json, "_UPTIME", uptime);
        }

        // Loop thru the node
        while (me->next) {
            // go to next node
            me = me->next;

            tlf_treat_label(me->name, me->value);
            // uniquement sur les nouvelles valeurs ou celles modifiées 
            // sauf si explicitement demandé toutes
            if ( all || ( me->flags & (TINFO_FLAGS_UPDATED | TINFO_FLAGS_ADDED) ) )
            {
                // we have at least something ?
                if (me->value && strlen(me->value))
                {
                    bool isNumber = true;
                    uint8_t c;
                    char * p = me->value;
          
                    // Format N° Serial 16 caractères
                    if ( strcmp(me->value, "ADCO") || strcmp(me->value, "ADSC") || strcmp(me->value, "PRM") ) {
                    isNumber = false;
                    }

                    // check if value is number
                    while (*p && isNumber) {
                        if ( *p < '0' || *p > '9' )
                            isNumber = false;
                        p++;
                    }
 
                    if (!isNumber) {
                        me_value = cJSON_CreateString(me->value);
                    }
                    else {
                        std::size_t pos{};
                        me_value = cJSON_CreateNumber(std::stol(me->value, &pos));
                    }
                    if (me_value == NULL)
                    {
                      goto end;
                    }
                    cJSON_AddItemToObject(trame_json, me->name, me_value);
                }
            }
        }
        string = cJSON_PrintUnformatted(trame_json);
        if (string == NULL)
        {
          fprintf(stderr, "Failed to print trame_json.\n");
        }
        end:
            cJSON_Delete(trame_json);
    }
    if (opts.emoncms)
    {
      sprintf(emoncms_url, "%s?node=%s&data=%s&apikey=%s", opts.url, opts.node, string, opts.apikey);
      if (!opts.daemon) 
      {
        fprintf(stdout, "%s\n", emoncms_url);
      }
      // Send data to emoncms
      if (!http_post(emoncms_url))
      {
        log_syslog(stderr, "emoncms post error\n");
      }
    }
    else
      fprintf(stdout, "%s\n", string);
}

// ======================================================================
// some func declaration
// ======================================================================
void tlf_close_serial(int);

/* ======================================================================
Function: log_syslog
Purpose : write event to syslog
Input   : stream to write if needed
          string to write in printf format
          printf other arguments
Output  : -
Comments: 
====================================================================== */
void log_syslog( FILE * stream, const char *format, ...)
{
  static char tmpbuff[512]="";
  va_list args;
  int len;

  // do a style printf style in ou buffer
  va_start (args, format);
  len = vsnprintf (tmpbuff, sizeof(tmpbuff), format, args);
  tmpbuff[sizeof(tmpbuff) - 1] = '\0';
  va_end (args);

  // Write to logfile
  openlog( PRG_NAME, LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "%s", tmpbuff);
  closelog();
  
  // stream passed ? write also to it
  if (stream && opts.verbose && !opts.daemon) 
  {
    fprintf(stream, "%s", tmpbuff);
    //fprintf(stream, "\n");
    fflush(stream);
  }
}

/* ======================================================================
Function: clean_exit
Purpose : exit program 
Input   : exit code
Output  : -
Comments: 
====================================================================== */
void clean_exit (int exit_code)
{
  int r;

  // free up linked list
  tinfo.listDelete();

  // close serials
  if (g_fd_teleinfo)
  {
    // Restore Old parameters.
    if (  (r = tcsetattr(g_fd_teleinfo, TCSAFLUSH, &g_oldtermios)) < 0 )
      log_syslog(stderr, "cannot restore old parameters %s: %s", opts.port, strerror(errno));

    // then close
    tlf_close_serial(g_fd_teleinfo);
  }

  exit(exit_code);
}

/* ======================================================================
Function: fatal
Purpose : exit program due to a fatal error
Input   : string to write in printf format
          printf other arguments
Output  : -
Comments: 
====================================================================== */
void fatal (const char *format, ...)
{
  char tmpbuff[512] = "";
  va_list args;
  int len;

  va_start(args, format);
  len = vsnprintf(tmpbuff, sizeof(tmpbuff), format, args);
  tmpbuff[sizeof(tmpbuff) - 1] = '\0';
  va_end(args);

  // Write to logfile
  openlog( PRG_NAME, LOG_PID | LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "%s", tmpbuff);
  closelog();

  fprintf(stderr,"\r\nFATAL: %s \r\n", tmpbuff );
  fflush(stderr);

  clean_exit(EXIT_FAILURE);
}

/* ======================================================================
Function: daemonize
Purpose : daemonize the pocess
Input   : -
Output  : -
Comments: 
====================================================================== */
static void daemonize(void)
{
  pid_t pid, sid;

  // already a daemon
  if ( getppid() == 1 )
    return;

  // Fork off the parent process
  pid = fork();
  if (pid < 0)
    fatal( "fork() : %s", strerror(errno));

  // If we got a good PID, then we can exit the parent process.
  if (pid > 0)
    exit(EXIT_SUCCESS);


  // At this point we are executing as the child process
  // ---------------------------------------------------

  // Change the file mode mask
  umask(0);

  // Create a new SID for the child process
  sid = setsid();
  if (sid < 0)
    fatal( "setsid() : %s", strerror(errno));

  // Change the current working directory.  This prevents the current
  // directory from being locked; hence not being able to remove it.
  if ((chdir(PRG_DIR)) < 0)
    fatal( "chdir('%s') : %s", PRG_DIR, strerror(errno));

  // Close standard files
  close(STDIN_FILENO);
  
  // if verbose mode, allow display on stdout
  if (!opts.verbose)
    close(STDOUT_FILENO);
    
  // Always display errors on stderr
  //close(STDERR_FILENO);
}

/* ======================================================================
Function: signal_handler
Purpose : Interrupt routine Code for signal
Input   : signal received
Output  : -
Comments: 
====================================================================== */
void signal_handler (int signum)
{
  // Does we received CTRL-C ?
  if ( signum==SIGINT )
  {
    // Indicate we want to quit
    g_exit_pgm = true;
    log_syslog(stdout, "\nReceived SIGINT\n");
  }
  else if ( signum==SIGTERM )
  {
    // Indicate we want to quit
    g_exit_pgm = true;
    log_syslog(stdout, "\nReceived SIGTERM\n");
  }
}

/* ======================================================================
Function: tlf_init_serial
Purpose : initialize serial port for receiving teleinfo
Input   : -
Output  : Serial Port Handle
Comments: -
====================================================================== */
int tlf_init_serial(void)
{
  int tty_fd, r ;
  struct termios  termios ;

  // Open serial device
  if ( (tty_fd = open(opts.port, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) < 0 ) 
    fatal( "tlf_init_serial %s: %s", opts.port, strerror(errno));
  else
    log_syslog( stdout, "'%s' opened.\n",opts.port);
    
  // Set descriptor status flags
  fcntl (tty_fd, F_SETFL, O_RDWR ) ;

  // Get current parameters for saving
  if (  (r = tcgetattr(tty_fd, &g_oldtermios)) < 0 )
    log_syslog(stderr, "cannot get current parameters %s: %s",  opts.port, strerror(errno));
    
  // copy current parameters and change for our own
  memcpy( &termios, &g_oldtermios, sizeof(termios)); 
  
  // raw mode
  cfmakeraw(&termios);
  
  if(opts.mode == TINFO_MODE_HISTORIQUE) {
    // Set serial speed to 1200 bps
    if (cfsetospeed(&termios, B1200) < 0 || cfsetispeed(&termios, B1200) < 0 )
      log_syslog(stderr, "cannot set serial speed to 1200 bps (mode historique): %s",  strerror(errno));
  } else {
    // Set serial speed to 9600 bps
    if (cfsetospeed(&termios, B9600) < 0 || cfsetispeed(&termios, B9600) < 0 )
      log_syslog(stderr, "cannot set serial speed to 9600 bps (mode standard): %s",  strerror(errno));
  }

  // Parity Even
  termios.c_cflag &= ~PARODD;
  termios.c_cflag |= PARENB;    
  
  // 7 databits
  termios.c_cflag = (termios.c_cflag & ~CSIZE) | CS7;
  
  // No Flow Control
  termios.c_cflag &= ~(CRTSCTS);
  termios.c_iflag &= ~(IXON | IXOFF | IXANY);
  
  // Local
  termios.c_cflag |= CLOCAL;

  // No minimal char but 5 sec timeout
  termios.c_cc [VMIN]  =  0 ;
  termios.c_cc [VTIME] = 50 ; 

  // now setup the whole parameters
  if ( tcsetattr (tty_fd, TCSANOW | TCSAFLUSH, &termios) <0) 
    log_syslog(stderr, "cannot set current parameters %s: %s",  opts.port, strerror(errno));
    
  // Sleep 50ms
  // trust me don't forget this one, it will remove you some
  // headache to find why serial is not working
  usleep(50000);

  return tty_fd ;
}

/* ======================================================================
Function: tlf_close_serial
Purpose : close serial port for receiving teleinfo
Input   : Serial Port Handle
Output  : -
Comments: 
====================================================================== */
void tlf_close_serial(int device)
{
  if (device)
  { 
    // flush and restore old settings
    tcsetattr(device, TCSANOW | TCSAFLUSH, &g_oldtermios);
      
    close(device) ;
  }
}

/* ======================================================================
Function: usage
Purpose : display usage
Input   : program name
Output  : -
Comments: 
====================================================================== */
void usage( char * name)
{
  printf("%s\n", PRG_NAME);
  printf("Usage is: %s [options] -y device\n", PRG_NAME);
  printf("Options are:\n");
  printf("  --<m>ode            : h (=historique) | s (=standard)\n");
  printf("  --tt<y> device dev  : open serial device name\n");
  printf("  --<v>erbose         : speak more to user\n");
  printf("  --<e>moncms         : send data to emoncms\n");
  printf("  --u<r>l             : emoncms url\n");
  printf("  --api<k>ey          : emoncms apikey\n");
  printf("  --<n>ode            : emoncms node\n");
  printf("  --<d>aemon          : daemonize the process\n");
  printf("  --<h>elp\n");
  printf("<?> indicates the equivalent short option.\n");
  printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
  printf("Example :\n");
  printf( "%s -y /dev/ttyAMA0\n\tstart listeming on hardware serial port /dev/ttyAMA0\n\n", PRG_NAME);
  printf( "%s -y /dev/ttyACM0\n\tstart listeming on USB microteleinfo dongle\n\n", PRG_NAME);
}

/* ======================================================================
Function: read_config
Purpose : read configuration from config file and/or command line
Input   : -
Output  : -
Comments: Config is read from config file then from command line params
          this means that command line parameters always override config
          file parameters 
====================================================================== */
void read_config(int argc, char *argv[])
{
  static struct option longOptions[] =
  {
    {"daemon",  no_argument,      0, 'd'},
    {"mode",    required_argument,0, 'm'},
    {"tty",     required_argument,0, 'y'},
    {"port",    required_argument,0, 'p'},
    {"verbose", no_argument,      0, 'v'},
    {"help",    no_argument,      0, 'h'},
    {"emoncms", no_argument,      0, 'e'},
    {"url",     required_argument,0, 'r'},
    {"apikey",  required_argument,0, 'k'},
    {"node",    required_argument,0, 'n'},
    {0, 0, 0, 0}
  };

  int optionIndex = 0;
  int c;
  char str_opt[64];
   
  char buffer[512];

  char* bufp = NULL;
  char* opt = NULL;
  char* optdata = NULL; 
  
  // default values
  *opts.port = '\0';
  opts.baud = 1200;
  opts.mode = TINFO_MODE_HISTORIQUE;
  opts.flow = FC_NONE;
  strcpy(opts.flow_str, "none");
  opts.parity = P_EVEN;
  strcpy(opts.parity_str, "even");
  opts.databits = 7;
  opts.verbose = false;

  
  // default options
  strcpy(str_opt, "hvm:y:");
  strcat(str_opt,  "der:k:n:");

  // We will scan all options given on command line.
  while (1) 
  {
    // no default error messages printed.
    opterr = 0;
    
    // Get option 
    c = getopt_long(argc, argv, str_opt,  longOptions, &optionIndex);

    // Last one ?
    if (c < 0)
      break;
    
    switch (c) 
    {
      case 'v':
        opts.verbose = true;
      break;

      case 'y':
        strncpy(opts.port, optarg, sizeof(opts.port) - 1);
        opts.port[sizeof(opts.port) - 1] = '\0';
      break;

      // These ones exit direct
      case 'h':
      case '?':
        usage(argv[0]);
        exit(EXIT_SUCCESS);
      break;

      case 'd':
        opts.daemon = true;
      break;

      case 'm':
      switch (optarg[0]) 
      {
        case 'h':
        case 'H':
          opts.mode = TINFO_MODE_HISTORIQUE;
          opts.baud = 1200;
        break;
        case 's':
        case 'S':
          opts.mode = TINFO_MODE_STANDARD;
          opts.baud = 9600;
        break;
        
        default:
          fprintf(stderr, "--mode '%c' ignored.\n", optarg[0]);
          fprintf(stderr, "please select at least mode historique or standard\n");
          fprintf(stderr, "--mode can be one off: 'h' or 's'\n");
        break;

      }
    break;

      case 'e':
        opts.emoncms = true;
      break;

      case 'r':
        strcpy(opts.url, optarg );
      break;

      case 'k':
        strcpy(opts.apikey, optarg );
      break;

      case 'n':
        strcpy(opts.node, optarg );
      break;

      default:
        fprintf(stderr, "Unrecognized option.\n");
        fprintf(stderr, "Run %s with '--help'.\n", PRG_NAME);
        exit(EXIT_FAILURE);
      break;
    }
  } 
  
  if ( !*opts.port)
  { 
    fprintf(stderr, "No tty device given\n");
    fprintf(stderr, "please select at least tty device such as /dev/ttyS0\n");
    fprintf(stderr, "Run %s with '--help'.\n", PRG_NAME);
    exit(EXIT_FAILURE);
  }

  if (opts.daemon && !opts.emoncms)
  {
      fprintf(stderr, "--daemon ignored.\n");
      fprintf(stderr, "--daemon must be used only in mode emoncms\n");
      opts.daemon = false;
  }
  
  if (opts.verbose)
  {
    printf("%s process ID: %d\n", PRG_NAME, getpid());
    printf("%s parent's ID: %d\n", PRG_NAME, getppid());

    printf("-- Serial Stuff -- \n");
    printf("tty device     : %s\n", opts.port);
    printf("flowcontrol    : %s\n", opts.flow_str);
    printf("baudrate is    : %d\n", opts.baud);
    printf("parity is      : %s\n", opts.parity_str);
    printf("databits are   : %d\n", opts.databits);
    printf("mode is        : %s\n", opts.mode ? "standard" : "historique");

    printf("-- Other Stuff -- \n");
    printf("verbose is     : %s\n", opts.verbose? "yes" : "no");

    if (opts.emoncms)
    {
      printf("-- Emoncms    -- \n");
      printf("Emoncms post   : %s\n", opts.emoncms ? "Enabled" : "Disabled");
      printf("Server url is  : %s\n", opts.url);
      printf("APIKEY is      : %s\n", opts.apikey);
      printf("Node is        : %s\n", opts.node);
    }
    printf("\n");
  } 
}

/* ======================================================================
Function: main
Purpose : Main entry Point
Input   : -
Output  : -
Comments: 
====================================================================== */
int main(int argc, char **argv)
{
  struct sigaction sa;
  fd_set rdset, wrset;
  unsigned char c;
  char  rcv_buff[TELEINFO_BUFSIZE];
  int   rcv_idx;
  char time_str[200];
  time_t t;
  struct tm *tmp;
  int n;
  struct sysinfo info;
  CURLcode res;
  
  g_fd_teleinfo = 0; 
  g_exit_pgm = false;
  
  // get configuration
  read_config(argc, argv);

  // Set up the structure to specify the exit action.
  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);
  sigaction (SIGTERM, &sa, NULL);
  sigaction (SIGINT, &sa, NULL); 

  // Initialize curl library
  if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK ) 
    fatal("Error initializing Global Curl");
  else
  {
    // Basic curl init
    g_pcurl = curl_easy_init();

    // If init was not OK
    if(!g_pcurl)
      fatal("Error initializing easy Curl");
    else
    {
      // Set curl write callback (to receive http stream response)
      if (  (res = curl_easy_setopt(g_pcurl, CURLOPT_WRITEFUNCTION, &http_write)) != CURLE_OK )
        fatal("Error initializing Curl CURLOPT_WRITEFUNCTION options : %s", curl_easy_strerror(res));
        
      // set Curl transfer timeout to 5 seconds
      else if ( (res = curl_easy_setopt(g_pcurl, CURLOPT_TIMEOUT, 5L)) != CURLE_OK )
        fatal("Error initializing Curl CURLOPT_TIMEOUT options : %s", curl_easy_strerror(res));
        
      // set Curl server connection  to 2 seconds
      else if ( (res = curl_easy_setopt(g_pcurl, CURLOPT_CONNECTTIMEOUT, 2L)) != CURLE_OK )
        fatal("Error initializing Curl CURLOPT_CONNECTTIMEOUT options : %s", curl_easy_strerror(res));
      else
      {
        if (opts.verbose)
          log_syslog(stderr, "Curl Initialized\n");
      }
    }
  }

  // Open serial port
  g_fd_teleinfo = tlf_init_serial();

  // Init teleinfo
  if (opts.mode == TINFO_MODE_STANDARD) {
    tinfo.init(TINFO_MODE_STANDARD);
  } else {
    tinfo.init(TINFO_MODE_HISTORIQUE);
  }

  // Attacher les callback dont nous avons besoin
  // pour cette demo, ADPS et TRAME modifiée
  tinfo.attachADPS(ADPSCallback);
  tinfo.attachUpdatedFrame(UpdatedFrame);
  tinfo.attachNewFrame(NewFrame); 

  log_syslog(stdout, "Inits succeded, entering Main loop\n");
  
  if (opts.daemon)
  {
    log_syslog(stdout, "Starting as a daemon\n");
    daemonize();
  }
  
  // Do while not end
  while ( ! g_exit_pgm ) {
    // Read from serial port
    n = read(g_fd_teleinfo, &c, 1);
    
    if (n >= 0)
      tinfo.process(c);
    
    // Check full frame every 60 sec
    sysinfo(&info);
    if (info.uptime >= g_info.uptime + 60) {
      g_info.uptime = info.uptime;
      fulldata = true;
    }
    
    // Sleep 10ms; let time to others process
    usleep(10000);
  } 
  
  log_syslog(stderr, "Program terminated\n");
  
  clean_exit(EXIT_SUCCESS);
  
  // avoid compiler warning
  return (0);
}
