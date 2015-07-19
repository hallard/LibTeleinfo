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
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <termios.h>
#include <getopt.h>
#include <sys/sysinfo.h>
#include "../../LibTeleinfo.h"

// ----------------
// Constants
// ----------------
#define true 1
#define false 0

#define PRG_NAME   "raspjson"
#define TELEINFO_DEVICE   ""
#define TELEINFO_BUFSIZE  512


// Some enum for serial
enum parity_e     {  P_NONE,  P_EVEN,    P_ODD };
enum flowcntrl_e  { FC_NONE,  FC_RTSCTS, FC_XONXOFF };
enum mode_e       { MODE_NONE, MODE_SEND,   MODE_RECEIVE, MODE_TEST };
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
  int verbose;
// Configuration structure defaults values
} opts ;


void sendJSON(ValueList * me, boolean all);


// ======================================================================
// Global vars 
// ======================================================================
int   g_fd_teleinfo;          // teleinfo serial handle
struct termios g_oldtermios ; // old serial config
int   g_exit_pgm;             // indicate en of the program
struct sysinfo g_info;
TInfo tinfo; // Teleinfo object

// Used to indicate if we need to send all date or just modified ones
boolean fulldata = true;
 
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
Function: sendJSON 
Purpose : dump teleinfo values on serial
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : - 
Comments: -
====================================================================== */
void sendJSON(ValueList * me, boolean all)
{
  bool firstdata = true;
  
  // Got at least one ?
  if (me) {
    // Json start
    printf("{");

    if (all) {
      printf("\"_UPTIME\":%ld", g_info.uptime);
      firstdata = false;
    }

    // Loop thru the node
    while (me->next) {
      // go to next node
      me = me->next;

      // uniquement sur les nouvelles valeurs ou celles modifiées 
      // sauf si explicitement demandé toutes
      if ( all || ( me->flags & (TINFO_FLAGS_UPDATED | TINFO_FLAGS_ADDED) ) )
      {
        // First elemement, no comma
        if (firstdata)
          firstdata = false;
        else
          printf(", ") ;

        printf("\"%s\":", me->name) ;

        // we have at least something ?
        if (me->value && strlen(me->value))
        {
          boolean isNumber = true;
          uint8_t c;
          char * p = me->value;

          // check if value is number
          while (*p && isNumber) {
            if ( *p < '0' || *p > '9' )
              isNumber = false;
            p++;
          }
  
          // this will add "" on not number values
          if (!isNumber) {
            printf("\"%s\"", me->value) ;
          }
          // this will remove leading zero on numbers
          else
            printf("%ld",atol(me->value));
        }
      }
    }
   // Json end
   printf("}\r\n") ;
   fflush(stdout);
  }
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
  if (stream && opts.verbose ) 
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
  
  // Set serial speed to 1200 bps
  if (cfsetospeed(&termios, B1200) < 0 || cfsetispeed(&termios, B1200) < 0 )
    log_syslog(stderr, "cannot set serial speed to 1200 bps: %s",  strerror(errno));
    
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
int usage( char * name)
{
  printf("%s\n", PRG_NAME);
  printf("Usage is: %s [options] -d device\n", PRG_NAME);
  printf("Options are:\n");
  printf("  --<d>evice dev : open serial device name\n");
  printf("  --<v>erbose    : speak more to user\n");
  printf("  --<h>elp\n");
  printf("<?> indicates the equivalent short option.\n");
  printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
  printf("Example :\n");
  printf( "%s -d /dev/ttyAMA0\n\tstart listeming on hardware serial port /dev/ttyAMA0\n\n", PRG_NAME);
  printf( "%s -d /dev/ttyUSB0\n\tstart listeming on USB microteleinfo dongle\n\n", PRG_NAME);
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
    {"port",    required_argument,0, 'p'},
    {"verbose", no_argument,      0, 'v'},
    {"help",    no_argument,      0, 'h'},
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
  opts.flow = FC_NONE;
  strcpy(opts.flow_str, "none");
  opts.parity = P_EVEN;
  strcpy(opts.parity_str, "even");
  opts.databits = 7;
  opts.verbose = false;

  
  // default options
  strcpy( str_opt, "hvd:");

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

      case 'd':
        strncpy(opts.port, optarg, sizeof(opts.port) - 1);
        opts.port[sizeof(opts.port) - 1] = '\0';
      break;

      // These ones exit direct
      case 'h':
      case '?':
        usage(argv[0]);
        exit(EXIT_SUCCESS);
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

  if (opts.verbose)
  {
    printf("%s\n", PRG_NAME);

    printf("-- Serial Stuff -- \n");
    printf("tty device     : %s\n", opts.port);
    printf("flowcontrol    : %s\n", opts.flow_str);
    printf("baudrate is    : %d\n", opts.baud);
    printf("parity is      : %s\n", opts.parity_str);
    printf("databits are   : %d\n", opts.databits);

    printf("-- Other Stuff -- \n");
    printf("verbose is     : %s\n", opts.verbose? "yes" : "no");
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

  // Open serial port
  g_fd_teleinfo = tlf_init_serial();

  // Init teleinfo
  tinfo.init();

  // Attacher les callback dont nous avons besoin
  // pour cette demo, ADPS et TRAME modifiée
  tinfo.attachADPS(ADPSCallback);
  tinfo.attachUpdatedFrame(UpdatedFrame);
  tinfo.attachNewFrame(NewFrame); 

  log_syslog(stdout, "Inits succeded, entering Main loop\n");
  
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
