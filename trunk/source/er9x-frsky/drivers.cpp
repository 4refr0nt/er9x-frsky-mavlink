/*
 * Author - Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include "er9x.h"

#ifndef SIMU
#include "avr/interrupt.h"

///opt/cross/avr/include/avr/eeprom.h
static inline void __attribute__ ((always_inline))
eeprom_write_byte_cmp (uint8_t dat, uint16_t pointer_eeprom)
{
  //see /home/thus/work/avr/avrsdk4/avr-libc-1.4.4/libc/misc/eeprom.S:98 143
#ifdef CPUM2561
  while(EECR & (1<<EEPE)) /* make sure EEPROM is ready */
#else
  while(EECR & (1<<EEWE)) /* make sure EEPROM is ready */
#endif
  {
    if (Ee_lock & EE_TRIM_LOCK)    // Only if writing trim changes
    {
      mainSequence() ;      // Keep the controls running while waiting		
    }
  } ;
  EEAR  = pointer_eeprom;

  EECR |= 1<<EERE;
  if(dat == EEDR) return;

  EEDR  = dat;
  uint8_t flags=SREG;
  cli();
#ifdef CPUM2561
  EECR |= 1<<EEMPE;
  EECR |= 1<<EEPE;
#else
  EECR |= 1<<EEMWE;
  EECR |= 1<<EEWE;
#endif
  SREG = flags;
}

void eeWriteBlockCmp(const void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size)
{
  const char* pointer_ram = (const char*)i_pointer_ram;
  uint16_t    pointer_eeprom = i_pointer_eeprom;
  while(size){
    eeprom_write_byte_cmp(*pointer_ram++,pointer_eeprom++);
    size--;
  }
}
#endif

//inline uint16_t anaIn(uint8_t chan)
//{
//  //                     ana-in:   3 1 2 0 4 5 6 7
//  static prog_char APM crossAna[]={4,2,3,1,5,6,7,0}; // wenn schon Tabelle, dann muss sich auch lohnen
//  return s_ana[pgm_read_byte(crossAna+chan)] / 4;
//}



uint8_t s_evt;

uint8_t getEvent()
{
  uint8_t evt = s_evt;
  s_evt=0;
  return evt;
}

class Key
{
#define FILTERBITS      4
#define FFVAL          ((1<<FILTERBITS)-1)
#define KSTATE_OFF      0
#define KSTATE_RPTDELAY 95 // gruvin: longer dely before key repeating starts
  //#define KSTATE_SHORT   96
#define KSTATE_START   97
#define KSTATE_PAUSE   98
#define KSTATE_KILLED  99
  uint8_t m_vals:FILTERBITS;   // key debounce?  4 = 40ms
  uint8_t unused_m_dblcnt:2;
  uint8_t m_cnt;
  uint8_t m_state;
public:
  void input(bool val, EnumKeys enuk);
  bool state()       { return m_vals==FFVAL;                }
  void pauseEvents() { m_state = KSTATE_PAUSE;  m_cnt   = 0;}
  void killEvents()  { m_state = KSTATE_KILLED; /*m_dblcnt=0;*/ }
//  uint8_t getDbl()   { return m_dblcnt;                     }
};


Key keys[NUM_KEYS];
void Key::input(bool val, EnumKeys enuk)
{
  //  uint8_t old=m_vals;
	uint8_t t_vals ;
//  m_vals <<= 1;  if(val) m_vals |= 1; //portbit einschieben
	t_vals = m_vals ;
	t_vals <<= 1 ;
  if(val) t_vals |= 1; //portbit einschieben
	m_vals = t_vals ;
  m_cnt++;

  if(m_state && m_vals==0){  //gerade eben sprung auf 0
    if(m_state!=KSTATE_KILLED) {
      putEvent(EVT_KEY_BREAK(enuk));
//      if(!( m_state == 16 && m_cnt<16)){
//        m_dblcnt=0;
//      }
        //      }
    }
    m_cnt   = 0;
    m_state = KSTATE_OFF;
  }
  switch(m_state){
    case KSTATE_OFF:
      if(m_vals==FFVAL){ //gerade eben sprung auf ff
        m_state = KSTATE_START;
//        if(m_cnt>16) m_dblcnt=0; //pause zu lang fuer double
        m_cnt   = 0;
      }
      break;
      //fallthrough
    case KSTATE_START:
      putEvent(EVT_KEY_FIRST(enuk));
      Inactivity.inacCounter = 0;
//      m_dblcnt++;
#ifdef KSTATE_RPTDELAY
      m_state   = KSTATE_RPTDELAY;
#else
      m_state   = 16;
#endif
      m_cnt     = 0;
      break;
#ifdef KSTATE_RPTDELAY
    case KSTATE_RPTDELAY: // gruvin: longer delay before first key repeat
      if(m_cnt == 32) putEvent(EVT_KEY_LONG(enuk)); // need to catch this inside RPTDELAY time
      if (m_cnt == 40) {
        m_state = 16;
        m_cnt = 0;
      }
      break;
#endif
    case 16:
#ifndef KSTATE_RPTDELAY
      if(m_cnt == 32) putEvent(EVT_KEY_LONG(enuk));
      //fallthrough
#endif
    case 8:
    case 4:
    case 2:
      if(m_cnt >= 48)  { //3 6 12 24 48 pulses in every 480ms
        m_state >>= 1;
        m_cnt     = 0;
      }
      //fallthrough
    case 1:
      if( (m_cnt & (m_state-1)) == 0)  putEvent(EVT_KEY_REPT(enuk));
      break;

    case KSTATE_PAUSE: //pause
      if(m_cnt >= 64)      {
        m_state = 8;
        m_cnt   = 0;
      }
      break;

    case KSTATE_KILLED: //killed
      break;
  }
}

bool keyState(EnumKeys enuk)
{
  uint8_t xxx = 0 ;
	uint8_t ping = PING ;
  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0;

  switch((uint8_t)enuk){
    case SW_ElevDR : xxx = PINE & (1<<INP_E_ElevDR);
    break ;

#if defined(CPUM128) || defined(CPUM2561)
    case SW_AileDR :
			if ( g_eeGeneral.FrskyPins )
			{
				xxx = PINC & (1<<INP_C_AileDR) ;
			}
			else
			{
				xxx = PINE & (1<<INP_E_AileDR) ;
			}
#else
    //case SW_AileDR : return PINE & (1<<INP_E_AileDR);
 #if (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
    case SW_AileDR : xxx = PINE & (1<<INP_E_AileDR);
 #else
    case SW_AileDR : xxx = PINC & (1<<INP_C_AileDR); //shad974: rerouted inputs to free up UART0
			// Test bit 0 of PINC as well, temp fix for reacher10 broken pin on PINC bit 7
//    								if ( ( PINC & 0x01 ) == 0 )
//										{
//											xxx = 0 ;											
//										}
 #endif
#endif
    break ;


    case SW_RuddDR : xxx = ping & (1<<INP_G_RuddDR);
    break ;
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0    : xxx = ~ping & (1<<INP_G_ID1);
    break ;
    case SW_ID1    : xxx = (ping & (1<<INP_G_ID1)) ; if ( xxx ) xxx = (PINE & (1<<INP_E_ID2));
    break ;
    case SW_ID2    : xxx = ~PINE & (1<<INP_E_ID2);
    break ;
    case SW_Gear   : xxx = PINE & (1<<INP_E_Gear);
    break ;
    //case SW_ThrCt  : return PINE & (1<<INP_E_ThrCt);

#if defined(CPUM128) || defined(CPUM2561)
    case SW_ThrCt :
			if ( g_eeGeneral.FrskyPins )
			{
				xxx = PINC & (1<<INP_C_ThrCt) ;
			}
			else
			{
				xxx = PINE & (1<<INP_E_ThrCt) ;
			}
#else
 #if (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
     case SW_ThrCt  : xxx = PINE & (1<<INP_E_ThrCt);
 #else
    case SW_ThrCt  : xxx = PINC & (1<<INP_C_ThrCt); //shad974: rerouted inputs to free up UART0
 #endif
#endif
		break ;

    case SW_Trainer: xxx = PINE & (1<<INP_E_Trainer);
    break ;
    default:;
  }
  if ( xxx )
  {
    return 1 ;
  }
  return 0;
}

void pauseEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].pauseEvents();
}
void killEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].killEvents();
}

//uint8_t getEventDbl(uint8_t event)
//{
//  event=event & EVT_KEY_MASK;
//  if(event < (int)DIM(keys))  return keys[event].getDbl();
//  return 0;
//}

//uint16_t g_anaIns[8];
volatile uint16_t g_tmr10ms;
//volatile uint8_t g8_tmr10ms ;
volatile uint8_t  g_blinkTmr10ms;
extern uint8_t StickScrollTimer ;


void per10ms()
{
	uint16_t tmr ;
//  g_tmr10ms++;				// 16 bit sized
//	g8_tmr10ms += 1 ;		// byte sized
//  g_blinkTmr10ms++;
  tmr = g_tmr10ms + 1 ;
	g_tmr10ms = tmr ;
	g_blinkTmr10ms = tmr ;
  uint8_t enuk = KEY_MENU;
  uint8_t    in = ~PINB;
	
	static uint8_t current ;
	uint8_t dir_keys ;
	uint8_t lcurrent ;

	dir_keys = in & 0x78 ;		// Mask to direction keys
	if ( ( lcurrent = current ) )
	{ // Something already pressed
		if ( ( lcurrent & dir_keys ) == 0 )
		{
			lcurrent = 0 ;	// No longer pressed
		}
		else
		{
			in &= lcurrent | 0x06 ;	// current or MENU or EXIT allowed
		}
	}
	if ( lcurrent == 0 )
	{ // look for a key
		if ( dir_keys & 0x20 )	// right
		{
			lcurrent = 0x60 ;		// Allow L and R for 9X
		}
		else if ( dir_keys & 0x40 )	// left
		{
			lcurrent = 0x60 ;		// Allow L and R for 9X
		}
		else if ( dir_keys & 0x08 )	// down
		{
			lcurrent = 0x08 ;
		}
		else if ( dir_keys & 0x10 )	// up
		{
			lcurrent = 0x10 ;
		}
		in &= lcurrent | 0x06 ;	// current or MENU or EXIT allowed
	}
	current = lcurrent ;

  for(uint8_t i=1; i<7; i++)
  {
    //INP_B_KEY_MEN 1  .. INP_B_KEY_LFT 6
    keys[enuk].input(in & 2,(EnumKeys)enuk);
    ++enuk;
		in >>= 1 ;
  }

  const static  prog_uchar  APM crossTrim[]={
    1<<INP_D_TRM_LH_DWN,
    1<<INP_D_TRM_LH_UP,
    1<<INP_D_TRM_LV_DWN,
    1<<INP_D_TRM_LV_UP,
    1<<INP_D_TRM_RV_DWN,
    1<<INP_D_TRM_RV_UP,
    1<<INP_D_TRM_RH_DWN,
    1<<INP_D_TRM_RH_UP
  };
  in = ~PIND;

	for(int i=0; i<8; i++)
  {
    // INP_D_TRM_RH_UP   0 .. INP_D_TRM_LH_UP   7
    keys[enuk].input(in & pgm_read_byte(crossTrim+i),(EnumKeys)enuk);
    ++enuk;
  }
	
	uint8_t value = Rotary.RotEncoder & 0x20 ;
	keys[enuk].input( value,(EnumKeys)enuk); // Rotary Enc. Switch
	
	value |= ~PINB & 0x7E ;
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;
	}
}


