#ifndef Rasjson_stand_h
#define Rasjson_stand_h

#include <stdint.h>
#define pgm_read_byte(x) (*(x))

// All contract type for legacy, standard mode has in clear text
enum TInfoContrat{
    CONTRAT_BAS = 1,  // BASE => Option Base.
    CONTRAT_HC,       // HC.. => Option Heures Creuses.
    CONTRAT_EJP,      // EJP. => Option EJP.
    CONTRAT_BBR,      // BBRx => Option Tempo
    CONTRAT_END
};

// contract displayed name for legacy, standard mode has in clear text
const char kContratName[] =
    "|Base|Heures Creuses|EJP|Bleu Blanc Rouge"
    ;

// Received current contract value for legacy, standard mode has in clear text
const char kContratValue[] =
    "|BASE|HC..|EJP|BBR"
    ;

// all tariff type for legacy, standard mode has in clear text
enum TInfoTarif{
    TARIF_TH = 1,
    TARIF_HC,  TARIF_HP,
    TARIF_HN,  TARIF_PM,
    TARIF_CB,  TARIF_CW, TARIF_CR,
    TARIF_PB,  TARIF_PW, TARIF_PR,
    TARIF_END
};

// Legacy mode Received current tariff values
const char kTarifValue[] =
    "|TH..|HC..|HP.."
    "|HN..|PM.."
    "|HCJB|HCJW|HCJR"
    "|HPJB|HPJW|HPJR"
    ;

// legacy mode tariff displayed name
const char kTarifName[] =
    "|Toutes|Creuses|Pleines"
    "|Normales|Pointe Mobile"
    "|Creuses Bleu|Creuses Blanc|Creuse Rouges"
    "|Pleines Bleu|Pleines Blanc|Pleines Rouges"
    ;

// contract name for standard mode LGTF 
#define TELEINFO_STD_CONTRACT_BASE  "BASE"
#define TELEINFO_STD_CONTRACT_HCHP  "HC"
#define TELEINFO_STD_CONTRACT_BBR   "BBR"
#define TELEINFO_STD_CONTRACT_EJP   "EJP"

// Label used to do some post processing and/or calculation
enum TInfoLabel{
    LABEL_BASE = 1,
    LABEL_ADCO, LABEL_ADSC,
    LABEL_HCHC, LABEL_HCHP, LABEL_EAST, LABEL_EASF01, LABEL_EASF02,
    LABEL_HCJB,LABEL_HPJB,LABEL_HCJW,LABEL_HPJW,LABEL_HCJR,LABEL_HPJR,
    LABEL_EASF03, LABEL_EASF04, LABEL_EASF05, LABEL_EASF06,
    LABEL_OPTARIF, LABEL_NGTF, LABEL_ISOUSC, LABEL_PREF, LABEL_PTEC, LABEL_LTARF, LABEL_NTARF,
    LABEL_PAPP, LABEL_SINSTS, LABEL_SINSTS1, LABEL_SINSTS2, LABEL_SINSTS3, LABEL_IINST, LABEL_IINST1, LABEL_IINST2, LABEL_IINST3, LABEL_IRMS1, LABEL_IRMS2, LABEL_IRMS3,
    LABEL_TENSION, LABEL_URMS1, LABEL_URMS2, LABEL_URMS3,
    LABEL_IMAX, LABEL_IMAX1, LABEL_IMAX2, LABEL_IMAX3, LABEL_PMAX, LABEL_SMAXSN,
    LABEL_DEMAIN,LABEL_MSG1,LABEL_MSG2,LABEL_STGE,
    LABEL_END
};

const char kLabel[] =
    "|BASE|ADCO|ADSC|VTIC|DATE"
    "|HCHC|HCHP|EAST|EASF01|EASF02"
    "|BBRHCJB|BBRHPJB|BBRHCJW|BBRHPJW|BBRHCJR|BBRHPJR"
    "|EASF03|EASF04|EASF05|EASF06|EASF07|EASF08|EASF09|EASF10"
    "|EASD01|EASD02|EASD03|EASD04|EAIT|ERQ1|ERQ2|ERQ3|ERQ4"
    "|IRMS1|IRMS2|IRMS3|URMS1|URMS2|URMS3"
    "|OPTARIF|NGTF|ISOUSC|PREF|PTEC|LTARF|NTARF"
    "|PREF|PCOUP|PAPP|SINSTS|SINSTS1|SINSTS2|SINSTS3|IINST|IINST1|IINST2|IINST3"
    "|SMAXSN|SMAXSN1|SMAXSN2|SMAXSN3|SMAXSN-1|SMAXSN1-1|SMAXSN2-1|SMAXSN3-1"
    "|SINSTI|SMAXIN|SMAXIN-1|CCASN|CCASN-1|CCAIN|CCAIN-1"
    "|TENSION|UMOY1|UMOY2|UMOY3"
    "|DPM1|FPM1|DPM2|FPM2|DPM3|FPM3"
    "|IMAX|IMAX1|IMAX2|IMAX3|PMAX|SMAXSN"
    "|DEMAIN|MSG1|MSG2|STGE|PRM|RELAIS|NJOURF|NJOURF+1|PJOURF+1|PPOINTE"
    ;

// Blacklisted label from telemetry
// Each label shoud be enclosed by pipe
const char kLabelBlacklist[] =
    "|PJOURF+1"
    "|MSG1"
    "|PPOINTE"
    "|NGTF"
    "|LTARF"
    "|STGE"
    "|"
    ;

char* Trim(char* p) {
  // Remove leading and trailing tab, \n, \v, \f, \r and space
  if (*p != '\0') {
    while ((*p != '\0') && isspace(*p)) { p++; }  // Trim leading spaces
    char* q = p + strlen(p) -1;
    while ((q >= p) && isspace(*q)) { q--; }   // Trim trailing spaces
    q++;
    *q = '\0';
  }
  return p;
}

// remove spaces at the beginning and end of the string (but not in the middle)
char* TrimSpace(char *p) {
  // Remove white-space character (' ','\t','\n','\v','\f','\r')
  char* write = p;
  char* read = p;
  char ch = '.';

  // skip all leading spaces
  while (isspace(*read)) {
    read++;
  }
  // copy the rest
  do {
    ch = *read++;
    *write++ = ch;
  } while (ch != '\0');
  // move to end
  read = p + strlen(p);
  // move backwards
  while (p != read) {
    read--;
    if (isspace(*read)) {
      *read = '\0';
    } else {
      break;
    }
  }
  return p;
}

char* GetTextIndexed(char* destination, size_t destination_size, uint16_t index, const char* haystack)
{
  // Returns empty string if not found
  // Returns text of found
  char* write = destination;
  const char* read = haystack;

  index++;
  while (index--) {
    size_t size = destination_size -1;
    write = destination;
    char ch = '.';
    while ((ch != '\0') && (ch != '|')) {
      ch = pgm_read_byte(read++);
      if (size && (ch != '|'))  {
        *write++ = ch;
        size--;
      }
    }
    if (0 == ch) {
      if (index) {
        write = destination;
      }
      break;
    }
  }
  *write = '\0';
  return destination;
}

#endif