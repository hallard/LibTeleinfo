// **********************************************************************************
// Driver definition for French Teleinfo
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// For any explanation about teleinfo ou use , see my blog
// http://hallard.me/category/tinfo
//
// Code based on following datasheet
// http://www.erdf.fr/sites/default/files/ERDF-NOI-CPT_02E.pdf
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#include "LibTeleinfo.h" 

/* ======================================================================
Class   : TInfo
Purpose : Constructor
Input   : -
Output  : -
Comments: -
====================================================================== */
TInfo::TInfo()
{
  // Init of our linked list
  _valueslist.name = NULL;
  _valueslist.value = NULL;
  _valueslist.checksum = '\0';
  _valueslist.flags = TINFO_FLAGS_NONE;

  // callback
  _fn_ADPS = NULL;
	_fn_data = NULL;   
	_fn_new_frame = NULL;   
	_fn_updated_frame = NULL;   
}

/* ======================================================================
Function: init
Purpose : configure ULPNode I/O ports 
Input   : -
Output  : -
Comments: - 
====================================================================== */
void TInfo::init()
{
  // free up linked list (in case on recall init())
  listDelete();

  // clear our receive buffer
	clearBuffer();

	// We're in INIT in term of receive data
	_state = TINFO_INIT;
}

/* ======================================================================
Function: attachADPS 
Purpose : attach a callback when we detected a ADPS on any phase
Input   : callback function
Output  : - 
Comments: -
====================================================================== */
void TInfo::attachADPS(void (*fn_ADPS)(uint8_t phase))
{
	// indicate the user callback
	_fn_ADPS = fn_ADPS;   
}

/* ======================================================================
Function: attachNewData 
Purpose : attach a callback when we detected a new/changed value 
Input   : callback function
Output  : - 
Comments: -
====================================================================== */
void TInfo::attachData(void (*fn_data)(ValueList * valueslist, uint8_t  state))
{
	// indicate the user callback
	_fn_data = fn_data;   
}

/* ======================================================================
Function: attachNewFrame 
Purpose : attach a callback when we received a full frame
Input   : callback function
Output  : - 
Comments: -
====================================================================== */
void TInfo::attachNewFrame(void (*fn_new_frame)(ValueList * valueslist))
{
	// indicate the user callback
	_fn_new_frame = fn_new_frame;   
}

/* ======================================================================
Function: attachChangedFrame 
Purpose : attach a callback when we received a full frame where data
					has changed since the last frame (cool to update data)
Input   : callback function
Output  : - 
Comments: -
====================================================================== */
void TInfo::attachUpdatedFrame(void (*fn_updated_frame)(ValueList * valueslist))
{
	// indicate the user callback
	_fn_updated_frame = fn_updated_frame;   
}

/* ======================================================================
Function: clearBuffer
Purpose : clear and init the buffer
Input   : -
Output  : -
Comments: - 
====================================================================== */
uint8_t TInfo::clearBuffer()
{
	// Clear our buffer, set index to 0
  memset(_recv_buff, 0, TINFO_BUFSIZE);
	_recv_idx = 0;
}

/* ======================================================================
Function  : valueAdd
Purpose   : Add element to the Linked List of values
Input     : Pointer to the label name
            pointer to the value
            checksum value
            state of the label (filled by function)
Output    : pointer to the new node (or founded one)
            state of the label changed by the function
====================================================================== */
ValueList * TInfo::valueAdd(char * name, char * value, uint8_t checksum, uint8_t * flags)
{
	// Get our linked list 
	ValueList * me = &_valueslist;

	uint8_t lgname = strlen(name);
	uint8_t lgvalue = strlen(value);

  // clear flags
  *flags = TINFO_FLAGS_NONE ;

  // Got one and all seems good ?
  if (me && lgname && lgvalue && checksum) {
    // Create pointer on the new node
    ValueList *newNode = NULL;
    ValueList *parNode = NULL ;

    // By default we done nothing
    *flags = TINFO_FLAGS_NOTHING;

    // Loop thru the node
    while (me->next) {
      // save parent node
      parNode = me ;

      // go to next node
      me = me->next;

      // Check if we already have this LABEL
      if (strncmp(me->name, name, lgname) == 0) {
        // Already got also this value, return US
        if (strncmp(me->value, value, lgvalue) == 0) {
          me->flags = TINFO_FLAGS_EXIST;
          *flags = me->flags;
          return ( me );
        } else {
          // We changed the value
          me->flags = TINFO_FLAGS_UPDATED;
          *flags = me->flags;
          // Do we have enought space to hold new value ?
          if (strlen(me->value) >= lgvalue ) {
            // Copy it
            strncpy(me->value, value , lgvalue );
            me->checksum = checksum ;

            // That's all
            return (me);
          } else {
            // indicate parent node next following node instead of me
            parNode->next = me->next;

            // Return to parent (that will now point on next node and not us)
            me = parNode;

            // free up this node
            free (me);
          }
        }
      }
    }

    // Our linked list structure sizeof(ValueList)
    // + Name  + '\0'
    // + Value + '\0'
		size_t size = sizeof(ValueList) + lgname + 1 + lgvalue + 1  ;
    // Create new node with size to store strings
    if ((newNode = (ValueList  *) malloc(size) ) == NULL) 
      return ( (ValueList *) NULL );
    else 
    	// get our buffer Safe
      memset(newNode, 0, size);
    
    // Put the new node on the list
    me->next = newNode;

    // First String located after last struct element
    // Second String located after the First + \0
    newNode->checksum = checksum;
    newNode->name = (char *)  newNode + sizeof(ValueList);
    newNode->value = (char *) newNode->name + lgname + 1;

    // Copy the string data
    memcpy(newNode->name , name  , lgname );
    memcpy(newNode->value, value , lgvalue );

    // So we just created this node but was it new
    // or was matter of text size ?
    if (*flags != TINFO_FLAGS_UPDATED) {
        // so we added this node !
        newNode->flags = TINFO_FLAGS_ADDED;
        *flags = newNode->flags  ;
    }

    // return pointer on the new node
    return (newNode);
  }

  // Error or Already Exists
  return ( (ValueList *) NULL);
}

/* ======================================================================
Function  : valueGet
Purpose   : get value of one element
Input     : Pointer to the label name
            pointer to the value where we fill data 
Output    : pointer to the value where we filled data NULL is not found
====================================================================== */
char * TInfo::valueGet(char * name, char * value)
{
  // Get our linked list 
  ValueList * me = &_valueslist;
  uint8_t lgname = strlen(name);

  // Got one and all seems good ?
  if (me && lgname) {

    // Loop thru the node
    while (me->next) {

      // go to next node
      me = me->next;

      // Check if we match this LABEL
      if (strncmp(me->name, name, lgname) == 0) {
        // this one has a value ?
        if (me->value) {
          // copy to dest buffer
          uint8_t lgvalue = strlen(me->value);
          strncpy(value, me->value , lgvalue );
          return ( value );
        }
      }
    }
  }
  // not found
  return ( NULL);
}

/* ======================================================================
Function  : getTopList
Purpose   : return a pointer on the top of the linked list
Input     : -
Output    : Pointer 
====================================================================== */
ValueList * TInfo::getList(void)
{
	// Get our linked list 
	return &_valueslist;
}

/* ======================================================================
Function  : valuesDump
Purpose   : dump linked list content
Input     : -
Output    : total number of values
====================================================================== */
uint8_t TInfo::valuesDump(void)
{
	// Get our linked list 
	ValueList * me = &_valueslist;
	uint8_t index = 0;

  // Got one ?
  if (me) {
    // Loop thru the node
    while (me->next) {
      // go to next node
      me = me->next;

      index++;
      TI_Debug(index) ;
      TI_Debug(F(") ")) ;

      if (me->name)
      	TI_Debug(me->name) ;
      else
	      TI_Debug(F("NULL")) ;

      TI_Debug(F("=")) ;

      if (me->value)
      	TI_Debug(me->value) ;
      else
	      TI_Debug(F("NULL")) ;

      TI_Debug(F(" '")) ;
      TI_Debug(me->checksum) ;
      TI_Debug(F("' ")); 

      // Flags management
      if ( me->flags) {
        TI_Debug(F("Flags : ")); 
        if ( me->flags & TINFO_FLAGS_EXIST)
          TI_Debug(F("Exist ")) ;
        if ( me->flags & TINFO_FLAGS_UPDATED)
          TI_Debug(F("Updated ")) ;
        if ( me->flags & TINFO_FLAGS_ADDED)
          TI_Debug(F("New ")) ;
      }

      TI_Debugln() ;
    }
  }

  return index;
}

/* ======================================================================
Function: labelCount
Purpose : Count the number of label in the list
Input   : -
Output  : element numbers
====================================================================== */
int TInfo::labelCount()
{
  int count = 0;

	// Get our linked list 
	ValueList * me = &_valueslist;

  if (me)
    while ((me = me->next))
      count++;

  return (count);
}

/* ======================================================================
Function: listDelete
Purpose : Delete the ENTIRE Linked List, not a value
Input   : -
Output  : True if Ok False Otherwise
====================================================================== */
boolean TInfo::listDelete()
{
	// Get our linked list 
	ValueList * me = &_valueslist;

  // Got a pointer
  if (me) {
    ValueList *current;

    // For each linked list
    while ((current = me->next)) {
      // Get the next
      me->next =  current->next;

      // Free the current
      free(current);
    }

    // Free the top element
    me->next = NULL ;

    // Ok
    return (true);
  }

  return (false);
}

/* ======================================================================
Function: checksum
Purpose : calculate the checksum based on data/value fields
Input   : label name 
					label value 
Output  : checksum
Comments: return '\0' in case of error
====================================================================== */
unsigned char TInfo::calcChecksum(char *etiquette, char *valeur) 
{
  uint8_t i ;
  uint8_t sum = ' ';  // Somme des codes ASCII du message + un espace

  // avoid dead loop, always check all is fine 
  if (etiquette && valeur) {
  	// this will not hurt and may save our life ;-)
  	if (strlen(etiquette) && strlen(valeur)) {
		  while (*etiquette)
		    sum += *etiquette++ ;
  
		  while(*valeur)
		    sum += *valeur++ ;
		    
		  return ( (sum & 63) + ' ' ) ;
  	}
  }
  return 0;
}

/* ======================================================================
Function: customLabel
Purpose : do action when received a correct label / value + checksum line
Input   : plabel : pointer to string containing the label
        : pvalue : pointer to string containing the associated value
Output  : 
Comments: 
====================================================================== */
void TInfo::customLabel( char * plabel, char * pvalue) 
{
  // Traitement de l'ADPS demandé
	if (_fn_ADPS) {
		// Monophasé
    if (strcmp(plabel, "ADPS")==0 )
			_fn_ADPS(0);

		// triphasé c'est ADIR + Num Phase
		if (plabel[0]=='A' && plabel[1]=='D' && plabel[2]=='I' && plabel[3]=='R' )
			if (plabel[4]>='1' && plabel[4]<='3')
				_fn_ADPS(plabel[4]-'0');
	}
}

/* ======================================================================
Function: checkLine
Purpose : check one line of teleinfo received
Input   : -
Output  : pointer to the data object in the linked list if OK else NULL
Comments: 
====================================================================== */
ValueList * TInfo::checkLine(char * pline) 
{
  char * p;
  char * ptok;
  char * pend;
  char * pvalue;
  char   checksum;
  char  buff[TINFO_BUFSIZE];
  uint8_t flags;
  boolean err = true ;	// Assume  error
  int len ; // Group len

  if (pline==NULL)
  	return NULL;

  len = strlen(pline); 

  // a line should be at least 7 Char
  // 2 Label + Space + 1 etiquette + space + checksum + \r
  if ( len < 7 )
  	return NULL;

  // Get our own working copy
  strncpy( buff, _recv_buff, len+1);

  p = &buff[0];
  ptok = p; 			// for sure we start with token name
  pend = p + len; // max size

  // Init values
  pvalue = NULL;
  checksum = 0;

  //TI_Debug("Got [");
  //TI_Debug(len);
  //TI_Debug("] ");

  
  // Loop in buffer 
  while ( p < pend ) {
    // start of token value
    if ( *p==' ' && ptok) {           
      // Isolate token name
      *p++ = '\0';

      // 1st space, it's the label value
      if (!pvalue)
        pvalue = p;
      else
	      // 2nd space, so it's the checksum
        checksum = *p;
    }           
    // new line ? ok we got all we need ?
    
    if ( *p=='\r' ) {           
      *p='\0';

      // Good format ?
      if ( ptok && pvalue && checksum ) {
      	// Always check to avoid bad behavior 
      	if(strlen(ptok) && strlen(pvalue)) {
	        // Is checksum is OK
	        if ( calcChecksum(ptok,pvalue) == checksum) {
	          // In case we need to do things on specific labels
	          customLabel(ptok, pvalue);

	          // Add value to linked lists of values
	          ValueList * me = valueAdd(ptok, pvalue, checksum, &flags);

	          // value correctly added/changed
	          if ( me ) {
				        // something to do with new datas
			        if (flags & (TINFO_FLAGS_UPDATED | TINFO_FLAGS_ADDED) ) {
			        	// this frame will for sure be updated
			        	_frame_updated = true;

			        	// Do we need to advertise user callback
			        	if (_fn_data)
			        		_fn_data(me, flags);
			        }
	          }
	        }
      	}
      }
    }           
    // Next char
    p++;

  } // While

  return NULL;
}

/* ======================================================================
Function: process
Purpose : teleinfo serial char received processing, should be called
          my main loop, this will take care of managing all the other
Input   : pointer to the serial used 
Output  : teleinfo global state
====================================================================== */
_State_e TInfo::process(char c)
{
   // be sure 7 bits only
   c &= 0x7F;

	// What we received ?
	switch (c) 	{
	  // start of transmission ???
	  case  TINFO_STX:
	    // Clear buffer, begin to store in it
	  	clearBuffer();

	  	// by default frame is not "updated"
	  	// if data change we'll set this flag
     	_frame_updated = false;

	  	// We were waiting fo this one ?
	  	if (_state == TINFO_INIT || _state == TINFO_WAIT_STX ) {
	        TI_Debugln(F("TINFO_WAIT_ETX"));
	  		 _state = TINFO_WAIT_ETX;
	  	}	
	  break;
	    
	  // End of transmission ?
	  case  TINFO_ETX:

	  	// Normal working mode ?
	  	if (_state == TINFO_READY) {
	  		// Get on top of our linked list 
				ValueList * me = &_valueslist;
				
        // Call user callback if any
		  	if (_frame_updated && _fn_updated_frame)
		  		_fn_updated_frame(me);
		  	else if (_fn_new_frame)
	  			_fn_new_frame(me);
	  	}

	  	// We were waiting fo this one ?
	  	if (_state == TINFO_WAIT_ETX) {
	    	TI_Debugln(F("TINFO_READY"));
	  		_state = TINFO_READY;
	  	}	
	  	else if ( _state == TINFO_INIT) {
	      TI_Debugln(F("TINFO_WAIT_STX"));
	  		_state = TINFO_WAIT_STX	;
	  	} 

	  break;

	  // Start of group \n ?
	  case  TINFO_SGR:
	    // Do nothing we'll work at end of group
	  	// we can safely ignore this char
	  break;

	  // End of group \r ?
	  case  TINFO_EGR:
	    // Are we ready to process ?
	    if (_state == TINFO_READY) {
        // Store data recceived (we'll need it)
	      if ( _recv_idx < TINFO_BUFSIZE)
	        _recv_buff[_recv_idx++]=c;

	      // clear the end of buffer (paranoia inside)
        memset(&_recv_buff[_recv_idx], 0, TINFO_BUFSIZE-_recv_idx);

        // check the group we've just received
	      checkLine(_recv_buff) ;

	      // Whatever error or not, we done
	      clearBuffer();
	    }
	  break;
	  
	  // other char ?
	  default:
	  {
	  	// Only in a ready state of course
	    if (_state == TINFO_READY) {
	      // If buffer is not full, Store data 
	      if ( _recv_idx < TINFO_BUFSIZE)
	        _recv_buff[_recv_idx++]=c;
	      else
	        clearBuffer();
		  }
	  }
	  break;
	}
}


