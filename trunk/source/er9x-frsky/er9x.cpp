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
#include <stdlib.h>
#include "language.h"
#include "pulses.h"
#include "lcd.h"
#include "menus.h"

// Next two lines swapped as new complier/linker reverses them in memory!
const
#include "s9xsplash.lbm"
#include "splashmarker.h"

/*
mode1 rud ele thr ail
mode2 rud thr ele ail
mode3 ail ele thr rud
mode4 ail thr ele rud
*/

// Various debug defines
#define	SERIALVOICE	0
// #define BLIGHT_DEBUG 1

#define ROTARY	1

extern int16_t AltOffset ;

#if defined(CPUM128) || defined(CPUM2561)
uint8_t Last_switch[NUM_CSW+EXTRA_CSW] ;
#else
uint8_t Last_switch[NUM_CSW] ;
#endif

static void checkMem( void );
static void checkTHR( void );
///   Pr�ft beim Einschalten ob alle Switches 'off' sind.
static void checkSwitches( void );

#ifndef SIMU
static void checkQuickSelect( void ); // Quick model select on startup
void getADC_osmp( void ) ;
#endif

EEGeneral  g_eeGeneral;
ModelData  g_model;

extern uint8_t scroll_disabled ;
const prog_char *AlertMessage ;
uint8_t Main_running ;
uint8_t SlaveMode ;
#if defined(CPUM128) || defined(CPUM2561)
uint8_t Vs_state[NUM_CHNOUT+EXTRA_VOICE_SW] ;
#else
uint8_t Vs_state[NUM_CHNOUT] ;
#endif
uint8_t CurrentVolume ;

uint8_t ppmInValid = 0 ;

struct t_rotary Rotary ;

uint8_t Tevent ;

//const prog_uint8_t APM chout_ar[] = { //First number is 0..23 -> template setup,  Second is relevant channel out
//                                      1,2,3,4 , 1,2,4,3 , 1,3,2,4 , 1,3,4,2 , 1,4,2,3 , 1,4,3,2,
//                                      2,1,3,4 , 2,1,4,3 , 2,3,1,4 , 2,3,4,1 , 2,4,1,3 , 2,4,3,1,
//                                      3,1,2,4 , 3,1,4,2 , 3,2,1,4 , 3,2,4,1 , 3,4,1,2 , 3,4,2,1,
//                                      4,1,2,3 , 4,1,3,2 , 4,2,1,3 , 4,2,3,1 , 4,3,1,2 , 4,3,2,1    };
const prog_uint8_t APM bchout_ar[] = {
																			0x1B, 0x1E, 0x27, 0x2D, 0x36, 0x39,
																			0x4B, 0x4E, 0x63, 0x6C, 0x72, 0x78,
                                      0x87, 0x8D, 0x93, 0x9C, 0xB1, 0xB4,
                                      0xC6, 0xC9, 0xD2, 0xD8, 0xE1, 0xE4		} ;

//new audio object
audioQueue  audio;

uint8_t sysFlags = 0;

struct t_alarmControl AlarmControl = { 100, 0, 10, 2 } ;

#if defined(CPUM128) || defined(CPUM2561)
int16_t  CsTimer[NUM_CSW+EXTRA_CSW] ;
#else
int16_t  CsTimer[NUM_CSW] ;
#endif

const prog_char APM Str_Alert[] = STR_ALERT ;
const prog_char APM Str_Switches[] = SWITCHES_STR ;

#define BACKLIGHT_ON    (Voice.Backlight = 1)
#define BACKLIGHT_OFF   (Voice.Backlight = 0)
//#define BACKLIGHT_ON    {Backlight = 1 ; if ( (g_eeGeneral.speakerMode & 2) == 0 ) PORTB |=  (1<<OUT_B_LIGHT);}
//#define BACKLIGHT_OFF   {Backlight = 0 ; if ( (g_eeGeneral.speakerMode & 2) == 0 ) PORTB &= ~(1<<OUT_B_LIGHT);}

const prog_char APM Str_OFF[] =  STR_OFF ;
const prog_char APM Str_ON[] = STR_ON ;

#ifdef FIX_MODE
const prog_char APM modi12x3[]= "\004"STR_STICK_NAMES ;
const prog_uint8_t APM stickScramble[]= {
    0, 1, 2, 3,
    0, 2, 1, 3,
    3, 1, 2, 0,
    3, 2, 1, 0 };

//const prog_uint8_t APM modeFix[] =
//{
//    1, 2, 3, 4,		// mode 1
//    1, 3, 2, 4,		// mode 2
//    4, 2, 3, 1,		// mode 3
//    4, 3, 2, 1		// mode 4
//} ;

#else
const prog_char APM modi12x3[]= 
STR_STICK_NAMES;
//"RUD THR ELE AIL "
//"AIL ELE THR RUD "
//"AIL THR ELE RUD ";
// Now indexed using modn2x3 from below

const prog_uint8_t APM modn12x3[]= {
    1, 2, 3, 4,
    1, 3, 2, 4,
    4, 2, 3, 1,
    4, 3, 2, 1 };
//R=1
//E=2
//T=3
//A=4
#endif

#ifdef FIX_MODE
uint8_t modeFixValue( uint8_t value )
{
	return pgm_read_byte(stickScramble+g_eeGeneral.stickMode*4+value)+1 ;
}
#endif

uint8_t CS_STATE( uint8_t x)
{
#ifdef VERSION3
	return ((x)<CS_AND ? CS_VOFS : ((((x)<CS_EQUAL) || ((x)==CS_LATCH)|| ((x)==CS_FLIP)) ? CS_VBOOL : ((x)<CS_TIME ? CS_VCOMP : CS_TIMER))) ;
#else	
	return ((x)<CS_AND ? CS_VOFS : ((x)<CS_EQUAL ? CS_VBOOL : ((x)<CS_TIME ? CS_VCOMP : CS_TIMER))) ;
#endif
}

MixData *mixaddress( uint8_t idx )
{
    return &g_model.mixData[idx] ;
}

LimitData *limitaddress( uint8_t idx )
{
    return &g_model.limitData[idx];
}


void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att)
{
	uint8_t chanLimit = NUM_XCHNRAW ;
	uint8_t mix = att & MIX_SOURCE ;
	if ( mix )
	{
#if GVARS
		chanLimit += MAX_GVARS + 1 + 1 ;
#else
		chanLimit += 1 ;
#endif
		att &= ~MIX_SOURCE ;		
	}
    if(idx==0)
        lcd_putsnAtt(x,y,PSTR("----"),4,att);
    else if(idx<=4)
#ifdef FIX_MODE
        lcd_putsAttIdx(x,y,modi12x3,(idx-1),att) ;
#else
        lcd_putsnAtt(x,y,&modi12x3[(pgm_read_byte(modn12x3+g_eeGeneral.stickMode*4+(idx-1))-1)*4],4,att);
#endif
    else if(idx<=chanLimit)
#if GVARS
        lcd_putsAttIdx(x,y,PSTR(STR_CHANS_GV),(idx-5),att);
#else
        lcd_putsAttIdx(x,y,PSTR(STR_CHANS_RAW),(idx-5),att);
#endif
#ifdef FRSKY
    else
		{
#if defined(CPUM128) || defined(CPUM2561)
			if ( mix )
			{
				idx += TEL_ITEM_SC1-(chanLimit-NUM_XCHNRAW) ;
			}
#endif
  	  lcd_putsAttIdx(x,y,Str_telemItems,(idx-NUM_XCHNRAW),att);
		}
#endif
}

void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att)
{
	if ( idx1 == 0 )
	{
    lcd_putsnAtt(x,y,PSTR("--- "),4,att);
	}
	else
	{
		uint8_t x1 ;
		x1 = x + 4*FW-2 ;
		if ( idx1 < 10 )
		{
			x1 -= FWNUM ;			
		}
//		lcd_outdezNAtt(uint8_t x,uint8_t y,int32_t val,uint8_t mode,int8_t len)
//  	lcd_outdezNAtt(x+2*FW,y,idx1,LEFT|att,2);
  	lcd_outdezAtt(x1,y,idx1,att);
    lcd_putsnAtt(x,y,PSTR(STR_CH),2,att);
	}
    // !! todo NUM_CHN !!
//    lcd_putsnAtt(x,y,PSTR("--- CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16"
//                          "CH17CH18CH19CH20CH21CH22CH23CH24CH25CH26CH27CH28CH29CH30")+4*idx1,4,att);
}

void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att)//, bool nc)
{
    switch(idx1){
    case  0:            lcd_putsAtt(x+FW,y,PSTR("---"),att);return;
    case  MAX_DRSWITCH: lcd_putsAtt(x+FW,y,Str_ON,att);return;
    case -MAX_DRSWITCH: lcd_putsAtt(x+FW,y,Str_OFF,att);return;
    }
		if ( idx1 < 0 )
		{
  		lcd_putcAtt(x,y, '!',att);
		}
		int8_t z ;
		z = idx1 ;
		if ( z < 0 )
		{
			z = -idx1 ;			
		}
		z -= 1 ;
//		z *= 3 ;
  lcd_putsAttIdx(x+FW,y,Str_Switches,z,att) ;
}

void putsTmrMode(uint8_t x, uint8_t y, uint8_t attr, uint8_t type )
{ // Valid values of type are 0, 1 or 2 only
  int8_t tm = g_model.tmrMode;
	if ( type < 2 )		// 0 or 1
	{
		uint8_t abstm = tm ;
		if (tm<0)
		{
			abstm = -tm ;
		}
    if(abstm<TMR_VAROFS) {
        lcd_putsAttIdx(  x, y, PSTR(STR_TMR_MODE),abstm,attr);
        if(tm<(-TMRMODE_ABS)) lcd_putcAtt(x-1*FW,  y,'!',attr);
//        return;
    }

    else if(abstm<(TMR_VAROFS+MAX_DRSWITCH-1)) { //normal on-off
        putsDrSwitches( x-1*FW,y,tm>0 ? tm-(TMR_VAROFS-1) : tm+(TMR_VAROFS-1),attr);
//        return;
    }
	  else if(tm>=(TMR_VAROFS+MAX_DRSWITCH-1+MAX_DRSWITCH-1)) // CH%
		{
  		tm -= (TMR_VAROFS+MAX_DRSWITCH-1+MAX_DRSWITCH-1) - 7 ;
      lcd_putsAttIdx(  x, y, PSTR( CURV_STR), tm, attr ) ;
			if ( tm < (9+7) )	// Allow for 7 offset above
			{
				x -= FW ;		
			}
  		lcd_putcAtt(x+3*FW,  y,'%',attr);
		}	
		else
		{
    	lcd_putcAtt(x+3*FW,  y,'m',attr);
    	putsDrSwitches( x-1*FW,y,tm>0 ? tm-(TMR_VAROFS+MAX_DRSWITCH-1-1) : tm+(TMR_VAROFS+MAX_DRSWITCH-1-1),attr);//momentary on-off
		}
	}
	if ( ( type == 2 ) || ( ( type == 0 ) && ( tm == 1 ) ) )
	{
   	putsDrSwitches( x-1*FW, y, g_model.tmrModeB, attr );
	}
}

#ifdef FRSKY

uint16_t scale_telem_value( uint16_t val, uint8_t channel, uint8_t times2, uint8_t *p_att )
{
  uint32_t value ;
	uint16_t ratio ;
	FrSkyChannelData *fd ;
	
	fd = &g_model.frsky.channels[channel] ;
  value = val ;
  ratio = fd->opt.alarm.ratio ;
  if ( times2 )
  {
      ratio <<= 1 ;
  }
  value *= ratio ;
	if ( fd->opt.alarm.type == 3/*A*/)
  {
      value /= 100 ;
      *p_att |= PREC1 ;
  }
  else if ( ratio < 100 )
  {
      value *= 2 ;
      value /= 51 ;  // Same as *10 /255 but without overflow
      *p_att |= PREC2 ;
  }
  else
  {
      value /= 255 ;
  }
	return value ;
}

uint8_t putsTelemValue(uint8_t x, uint8_t y, uint8_t val, uint8_t channel, uint8_t att, uint8_t scale)
{
    uint32_t value ;
    //  uint8_t ratio ;
    uint8_t times2 ;
    uint8_t unit = ' ' ;
		FrSkyChannelData *fd ;

		fd = &g_model.frsky.channels[channel] ;
    value = val ;
    if (fd->opt.alarm.type == 2/*V*/)
    {
        times2 = 1 ;
    }
    else
    {
        times2 = 0 ;
    }

    if ( scale )
    {
			value = scale_telem_value( val, channel, times2, &att ) ;
    }
    else
    {
        if ( times2 )
        {
            value <<= 1 ;
        }
		  	if (fd->opt.alarm.type == 3/*A*/)
        {
            value *= 255 ;
            value /= 100 ;
            att |= PREC1 ;
        }
    }
    //              val = (uint16_t)staticTelemetry[i]*g_model.frsky.channels[i].ratio / 255;
    //              putsTelemetry(x0-2, 2*FH, val, g_model.frsky.channels[i].type, blink|DBLSIZE|LEFT);
    //  if (g_model.frsky.channels[channel].type == 0/*v*/)
    if ( (fd->opt.alarm.type == 0/*v*/) || (fd->opt.alarm.type == 2/*v*/) )
    {
      lcd_outdezNAtt(x, y, value, att|PREC1, 5) ;
			unit = 'v' ;
      if(!(att&NO_UNIT)) lcd_putcAtt(lcd_lastPos, y, unit, att);
    }
    else
    {
        lcd_outdezAtt(x, y, value, att);
		    if (fd->opt.alarm.type == 3/*A*/)
				{
					unit = 'A' ;
        	if(!(att&NO_UNIT)) lcd_putcAtt(lcd_lastPos, y, unit, att);
				}
    }
		return unit ;
}


#endif

int16_t getValue(uint8_t i)
{
    if(i<7) return calibratedStick[i];//-512..512
    if(i<PPM_BASE) return 0 ;
		else if(i<CHOUT_BASE)
		{
			int16_t x ;
			x = g_ppmIns[i-PPM_BASE] ;
			if(i<PPM_BASE+4)
			{
				x -= g_eeGeneral.trainer.calib[i-PPM_BASE] ;
			}
			return x*2;
		}
		else if(i<CHOUT_BASE+NUM_CHNOUT) return ex_chans[i-CHOUT_BASE];
    else if(i<CHOUT_BASE+NUM_CHNOUT+NUM_TELEM_ITEMS)
		{
			return get_telemetry_value( i-CHOUT_BASE-NUM_CHNOUT ) ;
		}
    return 0;
}

bool getSwitch(int8_t swtch, bool nc, uint8_t level)
{
    bool ret_value ;
    uint8_t cs_index ;

    switch(swtch){
    case  0:            return  nc;
    case  MAX_DRSWITCH: return  true;
    case -MAX_DRSWITCH: return  false;
    }

		if ( swtch > MAX_DRSWITCH )
		{
			return false ;
		}

    uint8_t dir = swtch>0;
    uint8_t aswtch = swtch ;
		if ( swtch < 0 )
		{
			aswtch = -swtch ;			
		}		 

#if defined(CPUM128) || defined(CPUM2561)
    if(aswtch<(MAX_DRSWITCH-NUM_CSW-EXTRA_CSW))
#else
    if(aswtch<(MAX_DRSWITCH-NUM_CSW))
#endif
		{
			aswtch = keyState((EnumKeys)(SW_BASE+aswtch-1)) ;
			return !dir ? (!aswtch) : aswtch ;
    }

    //custom switch, Issue 78
    //use putsChnRaw
    //input -> 1..4 -> sticks,  5..8 pots
    //MAX,FULL - disregard
    //ppm
#if defined(CPUM128) || defined(CPUM2561)
    cs_index = aswtch-(MAX_DRSWITCH-NUM_CSW-EXTRA_CSW);
#else
    cs_index = aswtch-(MAX_DRSWITCH-NUM_CSW);
#endif
		
#if defined(CPUM128) || defined(CPUM2561)
		if ( cs_index >= NUM_CSW )
		{
			CxSwData *cs = &g_model.xcustomSw[cs_index-NUM_CSW];
			if(!cs->func) return false;

    	if ( level>4 )
    	{
			    ret_value = Last_switch[cs_index] & 1 ;
    	    return swtch>0 ? ret_value : !ret_value ;
    	}

    	int8_t a = cs->v1;
    	int8_t b = cs->v2;
    	int16_t x = 0;
    	int16_t y = 0;
			uint8_t valid = 1 ;

    	// init values only if needed
    	uint8_t s = CS_STATE(cs->func);

    	if(s == CS_VOFS)
    	{
    	    x = getValue(cs->v1-1);
	#ifdef FRSKY
    	    if (cs->v1 > CHOUT_BASE+NUM_CHNOUT)
					{
						uint8_t idx = cs->v1-CHOUT_BASE-NUM_CHNOUT-1 ;
    	      y = convertTelemConstant( idx, cs->v2 ) ;
						valid = telemItemValid( idx ) ;
					}
    	    else
	#endif
    	        y = calc100toRESX(cs->v2);
    	}
    	else if(s == CS_VCOMP)
    	{
    	    x = getValue(cs->v1-1);
    	    y = getValue(cs->v2-1);
    	}

    	switch ((uint8_t)cs->func) {
    	case (CS_VPOS):
    	    ret_value = (x>y);
    	    break;
    	case (CS_VNEG):
    	    ret_value = (x<y) ;
    	    break;
    	case (CS_APOS):
    	{
    	    ret_value = (abs(x)>y) ;
    	}
    	//      return swtch>0 ? (abs(x)>y) : !(abs(x)>y);
    	break;
    	case (CS_ANEG):
    	{
    	    ret_value = (abs(x)<y) ;
    	}
    	//      return swtch>0 ? (abs(x)<y) : !(abs(x)<y);
    	break;

    	//  case (CS_AND):
    	//      return (getSwitch(a,0,level+1) && getSwitch(b,0,level+1));
    	//      break;
    	//  case (CS_OR):
    	//      return (getSwitch(a,0,level+1) || getSwitch(b,0,level+1));
    	//      break;
    	//  case (CS_XOR):
    	//      return (getSwitch(a,0,level+1) ^ getSwitch(b,0,level+1));
    	//      break;
    	case (CS_AND):
    	case (CS_OR):
    	case (CS_XOR):
    	{
    	    bool res1 = getSwitch(a,0,level+1) ;
    	    bool res2 = getSwitch(b,0,level+1) ;
    	    if ( cs->func == CS_AND )
    	    {
    	        ret_value = res1 && res2 ;
    	    }
    	    else if ( cs->func == CS_OR )
    	    {
    	        ret_value = res1 || res2 ;
    	    }
    	    else  // CS_XOR
    	    {
    	        ret_value = res1 ^ res2 ;
    	    }
    	}
    	break;

    	case (CS_EQUAL):
    	    ret_value = (x==y);
    	    break;
    	case (CS_NEQUAL):
    	    ret_value = (x!=y);
    	    break;
    	case (CS_GREATER):
    	    ret_value = (x>y);
    	    break;
    	case (CS_LESS):
    	    ret_value = (x<y);
    	    break;
#ifndef VERSION3
    	case (CS_EGREATER):
    	    ret_value = (x>=y);
    	    break;
    	case (CS_ELESS):
    	    ret_value = (x<=y);
    	    break;
#endif
    	case (CS_TIME):
    	    ret_value = CsTimer[cs_index] >= 0 ;
    	    break;
#ifdef VERSION3
		  case (CS_LATCH) :
  		case (CS_FLIP) :
  		  ret_value = Last_switch[cs_index] & 1 ;
  		break ;
#endif
    	default:
    	    ret_value = false;
    	    break;
    	}
			if ( valid == 0 )			// Catch telemetry values not present
			{
    	  ret_value = false;
			}
			if ( ret_value )
			{
				int8_t x ;
				x = cs->andsw ;
				if ( x )
				{
					if ( x > 8 )
					{
						x += 1 ;
					}
					if ( x < -8 )
					{
						x -= 1 ;
					}
					if ( x > 9+NUM_CSW+EXTRA_CSW )
					{
						x = 9 ;			// Tag TRN on the end, keep EEPROM values
					}
					if ( x < -(9+NUM_CSW+EXTRA_CSW) )
					{
						x = -9 ;			// Tag TRN on the end, keep EEPROM values
					}
    	  	ret_value = getSwitch( x, 0, level+1) ;
				}
			}
#ifdef VERSION3
			if ( cs->func < CS_LATCH )
			{
#endif
				Last_switch[cs_index] = ret_value ;
#ifdef VERSION3
			}
#endif
    	return swtch>0 ? ret_value : !ret_value ;
		}
#endif
		CSwData *cs = &g_model.customSw[cs_index];
    
		if(!cs->func) return false;

    if ( level>4 )
    {
    		ret_value = Last_switch[cs_index] & 1 ;
        return swtch>0 ? ret_value : !ret_value ;
    }

    int8_t a = cs->v1;
    int8_t b = cs->v2;
    int16_t x = 0;
    int16_t y = 0;
		uint8_t valid = 1 ;

    // init values only if needed
    uint8_t s = CS_STATE(cs->func);

    if(s == CS_VOFS)
    {
        x = getValue(cs->v1-1);
#ifdef FRSKY
        if (cs->v1 > CHOUT_BASE+NUM_CHNOUT)
				{
					uint8_t idx = cs->v1-CHOUT_BASE-NUM_CHNOUT-1 ;
          y = convertTelemConstant( idx, cs->v2 ) ;
					valid = telemItemValid( idx ) ;
				}
        else
#endif
            y = calc100toRESX(cs->v2);
    }
    else if(s == CS_VCOMP)
    {
        x = getValue(cs->v1-1);
        y = getValue(cs->v2-1);
    }

    switch ((uint8_t)cs->func) {
    case (CS_VPOS):
        ret_value = (x>y);
        break;
    case (CS_VNEG):
        ret_value = (x<y) ;
        break;
    case (CS_APOS):
    {
        ret_value = (abs(x)>y) ;
    }
    //      return swtch>0 ? (abs(x)>y) : !(abs(x)>y);
    break;
    case (CS_ANEG):
    {
        ret_value = (abs(x)<y) ;
    }
    //      return swtch>0 ? (abs(x)<y) : !(abs(x)<y);
    break;

    //  case (CS_AND):
    //      return (getSwitch(a,0,level+1) && getSwitch(b,0,level+1));
    //      break;
    //  case (CS_OR):
    //      return (getSwitch(a,0,level+1) || getSwitch(b,0,level+1));
    //      break;
    //  case (CS_XOR):
    //      return (getSwitch(a,0,level+1) ^ getSwitch(b,0,level+1));
    //      break;
    case (CS_AND):
    case (CS_OR):
    case (CS_XOR):
    {
        bool res1 = getSwitch(a,0,level+1) ;
        bool res2 = getSwitch(b,0,level+1) ;
        if ( cs->func == CS_AND )
        {
            ret_value = res1 && res2 ;
        }
        else if ( cs->func == CS_OR )
        {
            ret_value = res1 || res2 ;
        }
        else  // CS_XOR
        {
            ret_value = res1 ^ res2 ;
        }
    }
    break;

    case (CS_EQUAL):
        ret_value = (x==y);
        break;
    case (CS_NEQUAL):
        ret_value = (x!=y);
        break;
    case (CS_GREATER):
        ret_value = (x>y);
        break;
    case (CS_LESS):
        ret_value = (x<y);
        break;
#ifndef VERSION3
    case (CS_EGREATER):
        ret_value = (x>=y);
        break;
    case (CS_ELESS):
        ret_value = (x<=y);
        break;
#endif
    case (CS_TIME):
        ret_value = CsTimer[cs_index] >= 0 ;
        break;
#ifdef VERSION3
  	case (CS_LATCH) :
  	case (CS_FLIP) :
    	ret_value = Last_switch[cs_index] & 1 ;
	  break ;
#endif
    default:
        ret_value = false;
        break;
    }
		if ( valid == 0 )			// Catch telemetry values not present
		{
      ret_value = false;
		}
		if ( ret_value )
		{
			int8_t x ;
			x = cs->andsw ;
			if ( x )
			{
				if ( x > 8 )
				{
					x += 1 ;
				}
      	ret_value = getSwitch( x, 0, level+1) ;
			}
		}
#ifdef VERSION3
		if ( cs->func < CS_LATCH )
		{
#endif
			Last_switch[cs_index] = ret_value ;
#ifdef VERSION3
		}
#endif
    return swtch>0 ? ret_value : !ret_value ;

}


//#define CS_EQUAL     8
//#define CS_NEQUAL    9
//#define CS_GREATER   10
//#define CS_LESS      11
//#define CS_EGREATER  12
//#define CS_ELESS     13

inline uint8_t keyDown()
{
    return (~PINB) & 0x7E;
}

static void clearKeyEvents()
{
#ifdef SIMU
    while (keyDown() && main_thread_running) sleep(1/*ms*/);
#else
    while (keyDown());  // loop until all keys are up
#endif
    putEvent(0);
}

void check_backlight_voice()
{
	static uint8_t tmr10ms ;
    if(getSwitch(g_eeGeneral.lightSw,0) || g_LightOffCounter)
        BACKLIGHT_ON ;
    else
        BACKLIGHT_OFF ;

	uint8_t x ;
	x = g_blinkTmr10ms ;
	if ( tmr10ms != x )
	{
		tmr10ms = x ;
		Voice.voice_process() ;
	}
}

uint16_t stickMoveValue()
{
#define INAC_DEVISOR 256   // Issue 206 - bypass splash screen with stick movement
    uint16_t sum = 0;
    for(uint8_t i=0; i<4; i++)
        sum += anaIn(i)/INAC_DEVISOR;
    return sum ;
}

static void doSplash()
{
    {

#ifdef SIMU
	    if (!main_thread_running) return;
  	  sleep(1/*ms*/);
#endif

        check_backlight_voice() ;

        lcd_clear();
        lcd_img(0, 0, s9xsplash,0);
        if(!g_eeGeneral.hideNameOnSplash)
            lcd_putsnAtt(0*FW, 7*FH, g_eeGeneral.ownerName ,sizeof(g_eeGeneral.ownerName),BSS);
    
// Next code is debug for trim reboot problem
//#ifdef CPUM2561
//extern uint8_t SaveMcusr ;
//				lcd_outhex4( 0*FW, 6*FH, SaveMcusr ) ;
//#endif

        refreshDiplay();
//				lcdSetContrast() ;

        clearKeyEvents();

//#ifndef SIMU
//        for(uint8_t i=0; i<32; i++)
//            getADC_filt(); // init ADC array
//#endif
#ifndef SIMU
        getADC_osmp();
#endif
        uint16_t inacSum = stickMoveValue();
        //        for(uint8_t i=0; i<4; i++)
        //           inacSum += anaIn(i)/INAC_DEVISOR;

        uint16_t tgtime = get_tmr10ms() + SPLASH_TIMEOUT;  
        do
				{
        	refreshDiplay();
          check_backlight_voice() ;
#ifdef SIMU
            if (!main_thread_running) return;
            sleep(1/*ms*/);
#else
            getADC_osmp();
#endif
            uint16_t tsum = stickMoveValue();
            //            for(uint8_t i=0; i<4; i++)
            //               tsum += anaIn(i)/INAC_DEVISOR;

            if(keyDown() || (tsum!=inacSum))   return;  //wait for key release

        } while(tgtime != get_tmr10ms()) ;
    }
}

static void checkMem()
{
    if(g_eeGeneral.disableMemoryWarning) return;
    if(EeFsGetFree() < 200)
    {
        alert(PSTR(STR_EE_LOW_MEM));
    }

}

void alertMessages( const prog_char * s, const prog_char * t )
{
    lcd_clear();
    lcd_putsAtt(64-5*FW,0*FH,Str_Alert,DBLSIZE);
    lcd_puts_Pleft(4*FH,s);
    lcd_puts_Pleft(5*FH,t);
    lcd_puts_Pleft(6*FH,  PSTR(STR_PRESS_KEY_SKIP) ) ;
//		lcdSetContrast() ;
}


int16_t tanaIn( uint8_t chan )
{
 	int16_t v = anaIn(chan) ;
	return  (g_eeGeneral.throttleReversed) ? -v : v ;
}

static void checkTHR()
{
    if(g_eeGeneral.disableThrottleWarning) return;

    uint8_t thrchn=(2-(g_eeGeneral.stickMode&1));//stickMode=0123 -> thr=2121
 	  
#ifndef SIMU
		getADC_osmp();   // if thr is down - do not display warning at all
#endif
 	  
		int16_t lowLim = g_eeGeneral.calibMid[thrchn] ;

		lowLim = (g_eeGeneral.throttleReversed ? (- lowLim) - g_eeGeneral.calibSpanPos[thrchn] : lowLim - g_eeGeneral.calibSpanNeg[thrchn]);
		lowLim += THRCHK_DEADBAND ;
 
 	  int16_t v = tanaIn(thrchn);
 
 	  if(v<=lowLim) return;

    // first - display warning
    alertMessages( PSTR(STR_THR_NOT_IDLE), PSTR(STR_RST_THROTTLE) ) ;
    refreshDiplay();
    clearKeyEvents();

    //loop until all switches are reset
    while (1)
    {
#ifdef SIMU
      if (!main_thread_running) return;
      sleep(1/*ms*/);
#else
        getADC_osmp();
#endif
        check_backlight_voice() ;
        
				v = tanaIn(thrchn);
        
				if((v<=lowLim) || (keyDown()))
        {
            return;
        }
    }
}

static void checkAlarm() // added by Gohst
{
    if(g_eeGeneral.disableAlarmWarning) return;
    if(!g_eeGeneral.beeperVal) alert(PSTR(STR_ALARMS_DISABLE));
}

static void checkWarnings()
{
    if(sysFlags && sysFLAG_OLD_EEPROM)
    {
        alert(PSTR(STR_OLD_VER_EEPROM)); //will update on next save
        sysFlags &= ~(sysFLAG_OLD_EEPROM); //clear flag
    }
}

void putWarnSwitch( uint8_t x, uint8_t idx )
{
  lcd_putsAttIdx( x, 2*FH, Str_Switches, idx, 0) ;

}

uint8_t getCurrentSwitchStates()
{
  uint8_t i = 0 ;
  for( uint8_t j=0; j<8; j++ )
  {
    bool t=keyState( (EnumKeys)(SW_BASE_DIAG+7-j) ) ;
		i <<= 1 ;
    i |= t ;
  }
	return i ;
}

static void checkSwitches()
{
	uint8_t warningStates ;
	
	warningStates = g_eeGeneral.switchWarningStates ;

    if(g_eeGeneral.disableSwitchWarning) return; // if warning is on

    uint8_t x = warningStates & SWP_IL5;
    if(!(x==SWP_LEG1 || x==SWP_LEG2 || x==SWP_LEG3)) //legal states for ID0/1/2
    {
        warningStates &= ~SWP_IL5; // turn all off, make sure only one is on
        warningStates |=  SWP_ID0B;
				g_eeGeneral.switchWarningStates = warningStates ;
    }

//#if SERIALVOICE


//#undef BAUD
//#define BAUD 38400

//#ifndef SIMU

//#include <util/setbaud.h>

//	PORTD |= 0x04 ;		// Pullup on RXD1

//  UBRR1H = UBRRH_VALUE;
//  UBRR1L = UBRRL_VALUE;
//  UCSR1A &= ~(1 << U2X1); // disable double speed operation.

//  // set 8 N1
//  UCSR1B = 0 | (0 << RXCIE1) | (0 << TXCIE1) | (0 << UDRIE1) | (0 << RXEN0) | (0 << TXEN1) | (0 << UCSZ12);
//  UCSR1C = 0 | (1 << UCSZ11) | (1 << UCSZ10);

//  UCSR1B |= (1 << TXEN1) ; // enable TX
//  UCSR1B |= (1 << RXEN1) ; // enable RX
  
//  while (UCSR1A & (1 << RXC1)) UDR1; // flush receive buffer

//	uint16_t k = 0 ;
//	uint8_t p = 0 ;
//	uint8_t q = 'x' ;

//#endif


//#endif

	uint8_t first = 1 ;
    //loop until all switches are reset
    while (1)
    {
        uint8_t i = getCurrentSwitchStates() ;

        //show the difference between i and switch?
        //show just the offending switches.
        //first row - THR, GEA, AIL, ELE, ID0/1/2
        uint8_t x = i ^ warningStates ;

		    alertMessages( PSTR(STR_SWITCH_WARN), PSTR(STR_RESET_SWITCHES) ) ;

        if(x & SWP_THRB)
            putWarnSwitch(2 + 0*FW, 0 );
        if(x & SWP_RUDB)
            putWarnSwitch(2 + 3*FW + FW/2, 1 );
        if(x & SWP_ELEB)
            putWarnSwitch(2 + 7*FW, 2 );

        if(x & SWP_IL5)
        {
            if(i & SWP_ID0B)
                putWarnSwitch(2 + 10*FW + FW/2, 3 );
            else if(i & SWP_ID1B)
                putWarnSwitch(2 + 10*FW + FW/2, 4 );
            else if(i & SWP_ID2B)
                putWarnSwitch(2 + 10*FW + FW/2, 5 );
        }

        if(x & SWP_AILB)
            putWarnSwitch(2 + 14*FW, 6 );
        if(x & SWP_GEAB)
            putWarnSwitch(2 + 17*FW + FW/2, 7 );


//#if SERIALVOICE
//				k += 1 ;
//				if ( k > 2500 )
//				{
//					k = 0 ;
//					if ( p == 0 )
//					{
//						p = 1 ;
//			  	  UDR1 = 'A' ;						
//					}
//					else
//					{
//						p = 0 ;
//			  	  UDR1 = 'B' ;
//					}
//					UCSR1A = ( 1 << TXC1 ) ;		// CLEAR flag
//					while ( ( UCSR1A & ( 1 << TXC1 ) ) == 0 )
//					{
//						// wait
//					}
//#undef BAUD
//#define BAUD 19200
//				  UBRR1L = UBRRL_VALUE;
//				}
//				if (UCSR1A & (1 << RXC1))
//				{
//					q = UDR1 ;
//#undef BAUD
//#define BAUD 38400
//				  UBRR1L = UBRRL_VALUE;
//				}
//				lcd_putc( 0, 8, q ) ;

//#endif
        refreshDiplay();

				if ( first )
				{
    			clearKeyEvents();
					first = 0 ;
				}

        if((i==warningStates) || (keyDown())) // check state against settings
        {
//#if SERIALVOICE
//  UCSR1B &= ~(1 << TXEN1) ; // disable TX pin
//  UCSR1B &= ~(1 << RXEN1) ; // disable RX
//#endif
            return;  //wait for key release
        }

        check_backlight_voice() ;
    }


}

void putsDblSizeName( uint8_t y )
{
	for(uint8_t i=0;i<sizeof(g_model.name);i++)
		lcd_putcAtt(FW*2+i*2*FW-i-2, y, g_model.name[i],DBLSIZE);
}



#if defined(CPUM128) || defined(CPUM2561)

static uint8_t switches_states = 0 ;

// Can we save flash by using :
// uint8_t getCurrentSwitchStates()

int8_t getMovedSwitch()
{
	uint8_t skipping = 0 ;
  int8_t result = 0 ;

  static uint16_t s_last_time = 0 ;

	uint16_t time = get_tmr10ms() ;
  if ( (uint16_t)(time - s_last_time) > 10)
	{
		skipping = 1 ;
		switches_states = 0 ;
	}
  s_last_time = time ;

  uint8_t mask = 0x80 ;
  for (uint8_t i=MAX_PSWITCH-1; i>0; i--)
	{
  	bool next = getSwitch(i, 0, 0) ;

		if ( skipping )
		{
			if ( next )
			{
				switches_states |= mask ;
			}
		}
		else
		{
			uint8_t value = next ? mask : 0 ;
			if ( ( switches_states ^ value ) & mask )
			{ // State changed
				switches_states ^= mask ;
        result = next ? i : -i ;
				if ( ( result <= -4 ) && ( result >= -6 ) )
				{
					result = 0 ;
				}
				break ;
			}
		}
		mask >>= 1 ;
  }
	if ( result == 0 )
	{
		if ( getSwitch( 9, 0, 0) )
		{
			result = 9 ;
		}
	}

  if ( skipping )
    result = 0 ;

  return result;
}
#endif

#ifndef SIMU
static void checkQuickSelect()
{
    uint8_t i = keyDown(); //check for keystate
    uint8_t j;

    for(j=0; j<7; j++)
		{
			if ( i & 0x02 ) break ;
			i >>= 1 ;
		}

//    for(j=1; j<8; j++)
//        if(i & ((uint8_t)(1<<j))) break;
//    j--;

    if(j<6) {
        if(!eeModelExists(j)) return;

        eeLoadModel(g_eeGeneral.currModel = j);
        STORE_GENERALVARS;
        //        eeDirty(EE_GENERAL);

        lcd_clear();
        lcd_putsAtt(64-7*FW,0*FH,PSTR(STR_LOADING),DBLSIZE);

				putsDblSizeName( 3*FH ) ;
//        for(uint8_t i=0;i<sizeof(g_model.name);i++)
//            lcd_putcAtt(FW*2+i*2*FW-i-2, 3*FH, g_model.name[i],DBLSIZE);

        refreshDiplay();
        clearKeyEvents(); // wait for user to release key
    }
}
#endif

uint8_t StickScrollAllowed ;
uint8_t StickScrollTimer ;

MenuFuncP g_menuStack[5];

uint8_t  g_menuStackPtr = 0;
uint8_t  EnterMenu = 0 ;
//uint8_t  g_beepCnt;
//uint8_t  g_beepVal[5];

#define	ALERT_TYPE	0
#define MESS_TYPE		1

void almess( const prog_char * s, uint8_t type )
{
	const prog_char *h ;
  lcd_clear();
  lcd_puts_Pleft(4*FW,s);
	if ( type == ALERT_TYPE)
	{
    lcd_puts_P(64-6*FW,7*FH,PSTR(STR_PRESS_ANY_KEY));
		h = Str_Alert ;
	}
	else
	{
		h = PSTR(STR_MESSAGE) ;
	}
  lcd_putsAtt(64-7*FW,0*FH, h,DBLSIZE);
  refreshDiplay();
}


void message(const prog_char * s)
{
	almess( s, MESS_TYPE ) ;
//	lcdSetContrast() ;
}

void alert(const prog_char * s, bool defaults)
{
	if ( Main_running )
	{
		AlertMessage = s ;
		return ;
	}
	almess( s, ALERT_TYPE ) ;
  
	lcdSetRefVolt(defaults ? LCD_NOMCONTRAST : g_eeGeneral.contrast);
  audioVoiceDefevent(AU_ERROR, V_ALERT);

    clearKeyEvents();
    while(1)
    {
#ifdef SIMU
    if (!main_thread_running) return;
    sleep(1/*ms*/);
#endif
        if(keyDown())
        {
				    clearKeyEvents() ;
            return;  //wait for key release
        }
        if(heartbeat == 0x3)
        {
            wdt_reset();
            heartbeat = 0;
        }

        if(getSwitch(g_eeGeneral.lightSw,0) || g_eeGeneral.lightAutoOff || defaults)
        	BACKLIGHT_ON ;
		    else
    	    BACKLIGHT_OFF ;
        check_backlight_voice() ;
    }
}

#ifdef FMODE_TRIM
int8_t *TrimPtr[4] = 
{
    &g_model.trim[0],
    &g_model.trim[1],
    &g_model.trim[2],
    &g_model.trim[3]
} ;
#endif

#ifdef PHASES		
uint8_t getFlightPhase()
{
	uint8_t i ;
  PhaseData *phase = &g_model.phaseData[0];

  for ( i = 0 ; i < MAX_MODES ; i += 1 )
	{
    if ( phase->swtch && getSwitch( phase->swtch, 0 ) )
		{
      return i + 1 ;
    }
		phase += 1 ;
  }
  return 0 ;
}

int16_t getRawTrimValue( uint8_t phase, uint8_t idx )
{
	if ( phase )
	{
		return g_model.phaseData[phase-1].trim[idx] + TRIM_EXTENDED_MAX + 1 ;
	}	
	else
	{
#ifdef FMODE_TRIM
		return *TrimPtr[idx] ;
#else    
		return g_model.trim[idx] ;
#endif
	}
}

uint8_t getTrimFlightPhase( uint8_t phase, uint8_t idx )
{
  for ( uint8_t i=0 ; i<MAX_MODES ; i += 1 )
	{
    if (phase == 0) return 0;
    int16_t trim = getRawTrimValue( phase, idx ) ;
    if ( trim <= TRIM_EXTENDED_MAX )
		{
			return phase ;
		}
    uint8_t result = trim-TRIM_EXTENDED_MAX-1 ;
    if (result >= phase)
		{
			result += 1 ;
		}
    phase = result;
  }
  return 0;
}


int16_t getTrimValue( uint8_t phase, uint8_t idx )
{
  return getRawTrimValue( getTrimFlightPhase( phase, idx ), idx ) ;
}


void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim)
{
	if ( phase )
	{
		phase = getTrimFlightPhase( phase, idx ) ;
	}
	if ( phase )
	{
    if(trim < -125 || trim > 125)
//    if(trim < -500 || trim > 500)
		{
			trim = ( trim > 0 ) ? 125 : -125 ;
//			trim = ( trim > 0 ) ? 500 : -500 ; For later addition
		}	
  	g_model.phaseData[phase-1].trim[idx] = trim - ( TRIM_EXTENDED_MAX + 1 ) ;
	}
	else
	{
    if(trim < -125 || trim > 125)
		{
			trim = ( trim > 0 ) ? 125 : -125 ;
		}	
#ifdef FMODE_TRIM
   	*TrimPtr[idx] = trim ;
#else    
		g_model.trim[idx] = trim ;
#endif
	}
  STORE_MODELVARS_TRIM ;
}
#endif


static uint8_t checkTrim(uint8_t event)
{
    int8_t  k = (event & EVT_KEY_MASK) - TRM_BASE;
    int8_t  s = g_model.trimInc;
//    if (s>1) s = 1 << (s-1);  // 1=>1  2=>2  3=>4  4=>8
		if ( s == 4 )
		{
			s = 8 ;			  // 1=>1  2=>2  3=>4  4=>8
		}
		else
		{
			if ( s == 3 )
			{
				s = 4 ;			  // 1=>1  2=>2  3=>4  4=>8
			}
		}


	  if( (k>=0) && (k<8) && !IS_KEY_BREAK(event)) // && (event & _MSK_KEY_REPT))
    {
        //LH_DWN LH_UP LV_DWN LV_UP RV_DWN RV_UP RH_DWN RH_UP
        uint8_t idx = (uint8_t)k/2;

// SORT idx for stickmode if FIX_MODE on
#ifdef FIX_MODE
				idx = pgm_read_byte(stickScramble+g_eeGeneral.stickMode*4+idx ) ;
#endif
				if ( g_eeGeneral.crosstrim )
				{
					idx = 3 - idx ;			
				}
#ifdef PHASES		
				uint8_t phaseNo = getTrimFlightPhase( CurrentPhase, idx ) ;
    		int16_t tm = getTrimValue( phaseNo, idx ) ;
#else
#ifdef FMODE_TRIM
        int8_t tm = *TrimPtr[idx] ;
#else    
        int8_t tm = g_model.trim[idx] ;
#endif
#endif
        int8_t  v = (s==0) ? (abs(tm)/4)+1 : s;
#ifdef FIX_MODE
        bool thrChan = (2 == idx) ;
#else
        bool thrChan = ((2-(g_eeGeneral.stickMode&1)) == idx);
#endif
        bool thro = (thrChan && (g_model.thrTrim));
        if(thro) v = 2 ; // if throttle trim and trim trottle then step=2
        if(thrChan && g_eeGeneral.throttleReversed) v = -v;  // throttle reversed = trim reversed
        int16_t x = (k&1) ? tm + v : tm - v;   // positive = k&1

        if(((x==0)  ||  ((x>=0) != (tm>=0))) && (!thro) && (tm!=0)){
#ifdef PHASES		
						setTrimValue( phaseNo, idx, 0 ) ;
#else
#ifdef FMODE_TRIM
            *TrimPtr[idx]=0;
#else    
						g_model.trim[idx] = 0 ;
#endif
#endif
            killEvents(event);
            audioDefevent(AU_TRIM_MIDDLE);

        } else if(x>-125 && x<125){
#ifdef PHASES		
						setTrimValue( phaseNo, idx, x ) ;
#else
#ifdef FMODE_TRIM
            *TrimPtr[idx] = (int8_t)x;
#else    
						g_model.trim[idx] = (int8_t)x ;
#endif
            STORE_MODELVARS_TRIM;
#endif
            //if(event & _MSK_KEY_REPT) warble = true;
//            if(x <= 125 && x >= -125){
                audio.event(AU_TRIM_MOVE,(abs(x)/4)+60);
//            }
        }
        else
        {
#ifdef PHASES		
						setTrimValue( phaseNo, idx, (x>0) ? 125 : -125 ) ;
#else
#ifdef FMODE_TRIM
            *TrimPtr[idx] = (x<0) ? -125 : 125;
#else    
						g_model.trim[idx] = (x<0) ? -125 : 125 ;
#endif
            STORE_MODELVARS_TRIM;
#endif
            if(x <= 125 && x >= -125){
                audio.event(AU_TRIM_MOVE,(-abs(x)/4)+60);
            }
        }

        return 0;
    }
    return event;
}

//global helper vars
bool    checkIncDec_Ret;
#ifndef NOPOTSCROLL
struct t_p1 P1values ;
#endif

int16_t checkIncDec16( int16_t val, int16_t i_min, int16_t i_max, uint8_t i_flags)
{
    int16_t newval = val;
    uint8_t kpl=KEY_RIGHT, kmi=KEY_LEFT, kother = -1;
//		uint8_t skipPause = 0 ;

		uint8_t event = Tevent ;
//    if(event & _MSK_KEY_DBL){
//        uint8_t hlp=kpl;
//        kpl=kmi;
//        kmi=hlp;
//        event=EVT_KEY_FIRST(EVT_KEY_MASK & event);
//    }
    if(event==EVT_KEY_FIRST(kpl) || event== EVT_KEY_REPT(kpl) || (s_editMode && (event==EVT_KEY_FIRST(KEY_UP) || event== EVT_KEY_REPT(KEY_UP))) ) {
        newval++;

        audioDefevent(AU_KEYPAD_UP);

        kother=kmi;
    }else if(event==EVT_KEY_FIRST(kmi) || event== EVT_KEY_REPT(kmi) || (s_editMode && (event==EVT_KEY_FIRST(KEY_DOWN) || event== EVT_KEY_REPT(KEY_DOWN))) ) {
        newval--;

        audioDefevent(AU_KEYPAD_DOWN);

        kother=kpl;
    }
    if((kother != (uint8_t)-1) && keyState((EnumKeys)kother)){
        newval=-val;
        killEvents(kmi);
        killEvents(kpl);
    }
    if(i_min==0 && i_max==1)
		{
			if (event==EVT_KEY_FIRST(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE))
	    {
        s_editMode = false;
        newval=!val;
        killEvents(event);
//				skipPause = 1 ;
				if ( event==EVT_KEY_BREAK(BTN_RE) )
				{
					RotaryState = ROTARY_MENU_UD ;
				}
				event = 0 ;
	    }
			else
			{
				newval &= 1 ;
			}
		}

#if defined(CPUM128) || defined(CPUM2561)
//  if (s_editMode>0 && (i_flags & INCDEC_SWITCH))
  if ( i_flags & INCDEC_SWITCH )
	{
    int8_t swtch = getMovedSwitch();
    if (swtch)
		{
      newval = swtch ;
    }
  }
#endif
    //change values based on P1
#ifndef NOPOTSCROLL
    newval -= P1values.p1valdiff;
#endif
		if ( RotaryState == ROTARY_VALUE )
		{
			newval += Rotary.Rotary_diff ;
		}
    if(newval>i_max)
    {
        newval = i_max;
        killEvents(event);
        audioDefevent(AU_KEYPAD_UP);
    }
    else if(newval < i_min)
    {
        newval = i_min;
        killEvents(event);
        audioDefevent(AU_KEYPAD_DOWN);

    }
    if(newval != val) {
        if(newval==0) {
//						if ( !skipPause )
//						{
          	  pauseEvents(event);
//						}

            if (newval>val){
                audioDefevent(AU_KEYPAD_UP);
            } else {
                audioDefevent(AU_KEYPAD_DOWN);
            }

        }
        eeDirty(i_flags & (EE_GENERAL|EE_MODEL));
        checkIncDec_Ret = true;
    }
    else {
        checkIncDec_Ret = false;
    }
    return newval;
}

NOINLINE int8_t checkIncDec( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags)
{
    return checkIncDec16( i_val,i_min,i_max,i_flags);
}

int8_t checkIncDec_hm( int8_t i_val, int8_t i_min, int8_t i_max)
{
    return checkIncDec( i_val,i_min,i_max,EE_MODEL);
}

int8_t checkIncDec_hm0( int8_t i_val, int8_t i_max)
{
    return checkIncDec( i_val,0,i_max,EE_MODEL);
}

int16_t checkIncDec_hmu0( int16_t i_val, uint8_t i_max)
{
  return checkIncDec16( i_val,0,i_max,EE_MODEL) ;
}

int8_t checkIncDec_hg( int8_t i_val, int8_t i_min, int8_t i_max)
{
    return checkIncDec( i_val,i_min,i_max,EE_GENERAL);
}

int8_t checkIncDec_hg0( int8_t i_val, int8_t i_max)
{
    return checkIncDec( i_val,0 ,i_max,EE_GENERAL);
}

MenuFuncP lastPopMenu()
{
    return  g_menuStack[g_menuStackPtr+1];
}

void popMenu(bool uppermost)
{
    if(g_menuStackPtr>0 || uppermost)
		{
        g_menuStackPtr = uppermost ? 0 : g_menuStackPtr-1;
				EnterMenu = EVT_ENTRY_UP ;
    }else{
        alert(PSTR(STR_MSTACK_UFLOW));
    }
}

void chainMenu(MenuFuncP newMenu)
{
    g_menuStack[g_menuStackPtr] = newMenu;
		EnterMenu = EVT_ENTRY ;
}
void pushMenu(MenuFuncP newMenu)
{

//    g_menuStackPtr++;
    if(g_menuStackPtr >= DIM(g_menuStack)-1)
    {
//        g_menuStackPtr--;
        alert(PSTR(STR_MSTACK_OFLOW));
        return;
    }
		EnterMenu = EVT_ENTRY ;
    g_menuStack[++g_menuStackPtr] = newMenu;
}

uint8_t  g_vbat100mV ;
volatile uint8_t tick10ms = 0;
uint16_t g_LightOffCounter;
uint8_t  stickMoved = 0;

inline bool checkSlaveMode()
{
    // no power -> only phone jack = slave mode

#ifdef BUZZER_MOD
    return SlaveMode = SLAVE_MODE ;
#else
    static bool lastSlaveMode = false;

    static uint8_t checkDelay = 0;
    if (audio.busy()) {
        checkDelay = 20;
    }
    else if (checkDelay) {
        --checkDelay;
    }
    else {
        lastSlaveMode = SLAVE_MODE;//
    }
    return (SlaveMode = lastSlaveMode) ;
#endif
}


//uint16_t Timer2 = 0 ;

void resetTimer2()
{
	struct t_timerg *tptr ;

	tptr = &TimerG ;
	FORCE_INDIRECT(tptr) ;
  tptr->Timer2_pre = 0 ;
  tptr->s_timerVal[1] = 0 ;
  tptr->Timer2_running = 0 ;   // Stop and clear throttle started flag
}

void doBackLightVoice(uint8_t evt)
{
    uint8_t a = 0;
    uint16_t b ;
    uint16_t lightoffctr ;
		lightoffctr = g_LightOffCounter ;

    if(lightoffctr) lightoffctr--;
    if(evt) a = g_eeGeneral.lightAutoOff ; // on keypress turn the light on 5*100
    if(stickMoved)
		{
			if ( g_eeGeneral.lightOnStickMove > a )
			{
				a = g_eeGeneral.lightOnStickMove ;
			}
		}
    b = a * 250 ;
		b <<= 1 ;				// b = a * 500, but less code
		if(b>lightoffctr) lightoffctr = b;
		g_LightOffCounter = lightoffctr ;
    check_backlight_voice();
}

//static uint8_t v_ctr ;
//uint8_t v_first[8] ;


void putVoiceQueueUpper( uint8_t value )
{
	putVoiceQueueLong( value + 260 ) ;
}


void putVoiceQueue( uint8_t value )
{
	putVoiceQueueLong( value ) ;
}

void setVolume( uint8_t value )
{
	CurrentVolume = value ;
	putVoiceQueueLong( value + 0xFFF0 ) ;
}

void putVoiceQueueLong( uint16_t value )
{
	struct t_voice *vptr ;
	vptr = &Voice ;
	FORCE_INDIRECT(vptr) ;
	
	if ( vptr->VoiceQueueCount < VOICE_Q_LENGTH )
	{
		vptr->VoiceQueue[vptr->VoiceQueueInIndex++] = value ;
		if (vptr->VoiceQueueInIndex > ( VOICE_Q_LENGTH - 1 ) )
		{
			vptr->VoiceQueueInIndex = 0 ;			
		}
		vptr->VoiceQueueCount += 1 ;
	}
}

void t_voice::voice_process(void)
{
	if ( g_eeGeneral.speakerMode & 2 )
	{
		if ( Backlight )
		{
			VoiceLatch |= BACKLIGHT_BIT ;			
		}
		else
		{
			VoiceLatch &= ~BACKLIGHT_BIT ;			
		}

		if ( VoiceState == V_IDLE )
		{
			PORTB |= (1<<OUT_B_LIGHT) ;				// Latch clock high
			if ( VoiceQueueCount )
			{
				VoiceSerial = VoiceQueue[VoiceQueueOutIndex++] ;
				if (VoiceQueueOutIndex > ( VOICE_Q_LENGTH - 1 ) )
				{
					VoiceQueueOutIndex = 0 ;			
				}
				VoiceQueueCount -= 1 ;
//				if ( VoiceShift )
//				{
//					VoiceShift = 0 ;
//					VoiceSerial += 260 ;
//				}
				VoiceTimer = 17 ;
//				if ( ( VoiceSerial & 0x00FF ) >= 0xF0 )
				if ( VoiceSerial & 0x8000 )	// Looking for Volume setting
				{
//					if ( VoiceSerial == 0xFF )
//					{
//						VoiceShift = 1 ;
//						return ;
//					}
//					VoiceSerial |= 0xFF00 ;
					VoiceTimer = 40 ;
				}
				VoiceLatch &= ~VOICE_CLOCK_BIT & ~VOICE_DATA_BIT ;
				if ( VoiceSerial & 0x8000 )
				{
					VoiceLatch |= VOICE_DATA_BIT ;
				}
				PORTA_LCD_DAT = VoiceLatch ;			// Latch data set
				PORTB &= ~(1<<OUT_B_LIGHT) ;			// Latch clock low
				VoiceCounter = 31 ;
				VoiceState = V_CLOCKING ;
			}
			else
			{
				PORTA_LCD_DAT = VoiceLatch ;			// Latch data set
				PORTB &= ~(1<<OUT_B_LIGHT) ;			// Latch clock low
			}
		}
		else if ( VoiceState == V_STARTUP )
		{
			PORTB |= (1<<OUT_B_LIGHT) ;				// Latch clock high
			VoiceLatch |= VOICE_CLOCK_BIT | VOICE_DATA_BIT ;
			PORTA_LCD_DAT = VoiceLatch ;			// Latch data set
			if ( g_blinkTmr10ms > 60 )					// Give module 1.4 secs to initialise
			{
				VoiceState = V_WAIT_START_BUSY_OFF ;
			}
			PORTB &= ~(1<<OUT_B_LIGHT) ;			// Latch clock low
		}
		else if ( VoiceState != V_CLOCKING )
		{
			uint8_t busy ;
			PORTA_LCD_DAT = VoiceLatch ;			// Latch data set
			PORTB |= (1<<OUT_B_LIGHT) ;				// Drive high,pullup enabled
			DDRB &= ~(1<<OUT_B_LIGHT) ;				// Change to input
			asm(" rjmp 1f") ;
			asm("1:") ;
			asm(" nop") ;											// delay to allow input to settle
			asm(" rjmp 1f") ;
			asm("1:") ;
			busy = PINB & 0x80 ;
			DDRB |= (1<<OUT_B_LIGHT) ;				// Change to output
			// The next bit guarantees the backlight output gets clocked out
			if ( VoiceState == V_WAIT_BUSY_ON )	// check for busy processing here
			{
				if ( busy == 0 )									// Busy is active
				{
					VoiceState = V_WAIT_BUSY_OFF ;
				}
				else
				{
					if ( --VoiceTimer == 0 )
					{
						VoiceState = V_WAIT_BUSY_OFF ;
					}
				}
			}
			else if (	VoiceState == V_WAIT_BUSY_OFF)	// check for busy processing here
			{
				if ( busy )									// Busy is inactive
				{
					VoiceTimer = 3 ;
					VoiceState = V_WAIT_BUSY_DELAY ;
				}
			}
			else if (	VoiceState == V_WAIT_BUSY_DELAY)
			{
				if ( --VoiceTimer == 0 )
				{
					VoiceState = V_IDLE ;
				}
			}
			else if (	VoiceState == V_WAIT_START_BUSY_OFF)	// check for busy processing here
			{
				if ( busy )									// Busy is inactive
				{
					VoiceTimer = 20 ;
					VoiceState = V_WAIT_BUSY_DELAY ;
				}
			}
			PORTB &= ~(1<<OUT_B_LIGHT) ;			// Latch clock low
		}
	}
	else// no voice, put backlight control out
	{
		if ( Backlight ^ g_eeGeneral.blightinv )
		{
			PORTB |= (1<<OUT_B_LIGHT) ;				// Drive high,pullup enabled
		}
		else
		{
			PORTB &= ~(1<<OUT_B_LIGHT) ;			// Latch clock low
		}
	}
}

void pollRotary()
{
	// Rotary Encoder polling
	PORTA = 0 ;			// No pullups
	DDRA = 0x1F ;		// Top 3 bits input
	asm(" rjmp 1f") ;
	asm("1:") ;
//	asm(" nop") ;
//	asm(" nop") ;
	uint8_t rotary ;
	rotary = PINA ;
	DDRA = 0xFF ;		// Back to all outputs
	rotary &= 0xE0 ;
//	RotEncoder = rotary ;

	struct t_rotary *protary = &Rotary ;
	FORCE_INDIRECT(protary) ;

	if( protary->TezRotary != 0)
		protary->RotEncoder = 0x20; // switch is on
	else
		protary->RotEncoder = rotary ; // just read the lcd pin
	
	rotary &= 0xDF ;
	if ( rotary != protary->RotPosition )
	{
		uint8_t x ;
		x = protary->RotPosition & 0x40 ;
		x <<= 1 ;
		x ^= rotary & 0x80 ;
		if ( x )
		{
			protary->RotCount -= 1 ;
		}
		else
		{
			protary->RotCount += 1 ;
		}
		protary->RotPosition = rotary ;
	}
	if ( protary->TrotCount != protary->LastTrotCount )
	{
		protary->RotCount = protary->LastTrotCount = protary->TrotCount ;
	}
}

//const static prog_uint8_t APM rate[8] = { 0, 75, 40, 25, 10, 5, 2, 1 } ;
const static prog_uint8_t APM rate[8] = { 0, 0, 100, 40, 16, 7, 3, 1 } ;

uint8_t calcStickScroll( uint8_t index )
{
	uint8_t direction ;
	int8_t value ;

	if ( ( g_eeGeneral.stickMode & 1 ) == 0 )
	{
		index ^= 3 ;
	}
	
#ifdef FIX_MODE
	value = phyStick[index] ;
	value /= 8 ;
#else
	value = (calibratedStick[index] * 2) >> 8 ; // same as / 128
#endif

	direction = value > 0 ? 0x80 : 0 ;
	if ( value < 0 )
	{
		value = -value ;			// (abs)
	}
	if ( value > 7 )
	{
		value = 7 ;			
	}
	value = pgm_read_byte(rate+(uint8_t)value) ;
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;		// Seconds
	}
	return value | direction ;
}

//uint16_t MixCounter ;
//uint16_t MixRate ;

void perMain()
{
    static uint8_t lastTMR;
//    static uint8_t timer20mS ;
		uint8_t t10ms ;
		t10ms = g_tmr10ms ;
    tick10ms = t10ms - lastTMR ;
    lastTMR = t10ms ;
    //    uint16_t time10ms ;
    //		time10ms = get_tmr10ms();
    //    tick10ms = (time10ms != lastTMR);
    //    lastTMR = time10ms;

    perOutPhase(g_chans512, 0);
//		MixCounter += 1 ;
    if(tick10ms == 0) return ; //make sure the rest happen only every 10ms.

		{
			struct t_timerg *tptr ;

			tptr = &TimerG ;
			FORCE_INDIRECT(tptr) ;

    	//  if ( Timer2_running )
    	if ( tptr->Timer2_running & 1)  // ignore throttle started flag
    	{
    	  if ( (tptr->Timer2_pre += 1 ) >= 100 )
    	  {
    	      tptr->Timer2_pre -= 100 ;
    	      tptr->s_timerVal[1] += 1 ;
    	  }
    	}
		}

		if ( ppmInValid )
		{
			ppmInValid -= 1 ;
		}

    eeCheck();

		// Every 10mS update backlight output to external latch
		// Note: LcdLock not needed here as at tasking level

    lcd_clear();
    uint8_t evt=getEvent();
    evt = checkTrim(evt);


#ifndef NOPOTSCROLL
		int16_t p1d ;

		struct t_p1 *ptrp1 ;
		ptrp1 = &P1values ;
		FORCE_INDIRECT(ptrp1) ;

		int16_t c6 = calibratedStick[6] ;
    p1d = ( ptrp1->p1val-c6 )/32;
    if(p1d) {
        p1d = (ptrp1->p1valprev-c6)/2;
        ptrp1->p1val = c6 ;
    }
    ptrp1->p1valprev = c6 ;
    if ( g_eeGeneral.disablePotScroll || (scroll_disabled) )
    {
        p1d = 0 ;
    }
		ptrp1->p1valdiff = p1d ;
#endif

		struct t_rotary *protary = &Rotary ;
		FORCE_INDIRECT(protary) ;
		{
			int8_t x ;
			x = protary->RotCount - protary->LastRotaryValue ;
			if ( x == -1 )
			{
				x = 0 ;
			}
			protary->Rotary_diff = ( x ) / 2 ;
			protary->LastRotaryValue += protary->Rotary_diff * 2 ;
		}
    
		doBackLightVoice( evt | protary->Rotary_diff ) ;
// Handle volume
		uint8_t requiredVolume ;
		requiredVolume = g_eeGeneral.volume+7 ;

		if ( g_menuStack[g_menuStackPtr] == menuProc0)
		{
			if ( protary->Rotary_diff )
			{
				int16_t x = protary->RotaryControl ;
				x += protary->Rotary_diff ;
				if ( x > 125 )
				{
					protary->RotaryControl = 125 ;
				}
				else if ( x < -125 )
				{
					protary->RotaryControl = -125 ;
				}
				else
				{
					protary->RotaryControl = x ;					
				}
				protary->Rotary_diff = 0 ;
			}
			
			if ( g_model.anaVolume )	// Only check if on main screen
			{
				uint16_t v ;
				uint16_t divisor ;
				if ( g_model.anaVolume < 4 )
				{
					v = calibratedStick[g_model.anaVolume+3] + 1024 ;
					divisor = 2048 ;
				}
				else
				{
					v = g_model.gvars[g_model.anaVolume].gvar + 125 ;
					divisor = 250 ;
				}
				requiredVolume = v * (NUM_VOL_LEVELS-1) / divisor ;
			}
		}
		if ( requiredVolume != CurrentVolume )
		{
			setVolume( requiredVolume ) ;
		}
		
		if ( g_eeGeneral.stickScroll && StickScrollAllowed )
		{
		 	if ( StickScrollTimer )
			{
				static uint8_t repeater ;
				uint8_t direction ;
				uint8_t value ;
		
				if ( repeater < 128 )
				{
					repeater += 1 ;
				}
				value = calcStickScroll( 2 ) ;
				direction = value & 0x80 ;
				value &= 0x7F ;
				if ( value )
				{
					if ( repeater > value )
					{
						repeater = 0 ;
						if ( evt == 0 )
						{
							if ( direction )
							{
								evt = EVT_KEY_FIRST(KEY_UP) ;
							}
							else
							{
								evt = EVT_KEY_FIRST(KEY_DOWN) ;
							}
						}
					}
				}
				else
				{
					value = calcStickScroll( 3 ) ;
					direction = value & 0x80 ;
					value &= 0x7F ;
					if ( value )
					{
						if ( repeater > value )
						{
							repeater = 0 ;
							if ( evt == 0 )
							{
								if ( direction )
								{
									evt = EVT_KEY_FIRST(KEY_RIGHT) ;
								}
								else
								{
									evt = EVT_KEY_FIRST(KEY_LEFT) ;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			StickScrollTimer = 0 ;		// Seconds
		}	
		StickScrollAllowed = 1 ;

#if GVARS
		for( uint8_t i = 0 ; i < MAX_GVARS ; i += 1 )
		{
			if ( g_model.gvars[i].gvsource )
			{
				int16_t value ;
				uint8_t src = g_model.gvars[i].gvsource ;
				if ( src <= 4 )
				{
//					value = *TrimPtr[ convert_mode_helper(src) - 1 ] ;
#ifdef FIX_MODE
					value = getTrimValue( CurrentPhase, src - 1 ) ;
#else
					value = getTrimValue( CurrentPhase, convert_mode_helper(src) - 1 ) ;
#endif
				}
			  else if ( src == 5 )	// REN
				{
					value = Rotary.RotaryControl ;
				}
				else if ( src <= 9 )	// Stick
				{
#ifdef FIX_MODE
					value = calibratedStick[ src-5 - 1 ] / 8 ;
#else
					value = calibratedStick[ convert_mode_helper( src-5) - 1 ] / 8 ;
#endif
				}
				else if ( src <= 12 )	// Pot
				{
					value = calibratedStick[ ( src-6)] / 8 ;
				}
				else// if ( g_model.gvars[i].gvsource <= 28 )	// Chans
				{
					value = ex_chans[src-13] / 10 ;
				}
				g_model.gvars[i].gvar = limit( -125, value, 125 ) ;
			}
		}
#endif

			static uint8_t alertKey ;
			if ( AlertMessage )
			{
				almess( AlertMessage, ALERT_TYPE ) ;
				uint8_t key = keyDown() ;
				if ( alertKey )
				{
					if( key == 0 )
					{
						AlertMessage = 0 ;
					}
				}
				else if ( key )
				{
					alertKey = 1 ;
				}
	//    	if ( stickMoved )
	//			{
	//				AlertMessage = 0 ;
	//			}
			}
			else
			{
				alertKey = 0 ;

				if ( EnterMenu )
				{
					evt = EnterMenu ;
					EnterMenu = 0 ;
					audioDefevent(AU_MENUS);
				}
				Tevent = evt ;
    		g_menuStack[g_menuStackPtr](evt);
			}
//		if ( ++timer20mS > 9 )		// Only do next bit every 100mS
//		{
//			timer20mS = 0 ;
    	refreshDiplay();
//		}
		{
			uint8_t pg ;
			pg = PORTG ;
    	if( (checkSlaveMode()) && (!g_eeGeneral.enablePpmsim))
			{
    	    pg &= ~(1<<OUT_G_SIM_CTL); // 0=ppm out
    	}else{
    	    pg |=  (1<<OUT_G_SIM_CTL); // 1=ppm-in
    	}
			PORTG = pg ;
		}

    switch( g_blinkTmr10ms & 0x1f ) { //alle 10ms*32

    case 2:
    {
        //check v-bat
        //        Calculation By Mike Blandford
        //        Resistor divide on battery voltage is 5K1 and 2K7 giving a fraction of 2.7/7.8
        //        If battery voltage = 10V then A2D voltage = 3.462V
        //        11 bit A2D count is 1417 (3.462/5*2048).
        //        1417*18/256 = 99 (actually 99.6) to represent 9.9 volts.
        //        Erring on the side of low is probably best.

        int16_t ab = anaIn(7);
        ab = ab*16 + ab/8*(6+g_eeGeneral.vBatCalib) ;
        ab = (uint16_t) ab / (g_eeGeneral.disableBG ? 240 : BandGap ) ;  // ab might be more than 32767
        g_vbat100mV = (ab + g_vbat100mV + 1) >> 1 ;  // Filter it a bit => more stable display

        static uint8_t s_batCheck;
        s_batCheck+=16 ;
        if((s_batCheck==0) && (g_vbat100mV<g_eeGeneral.vBatWarn) && (g_vbat100mV>49)){

            audioVoiceDefevent(AU_TX_BATTERY_LOW, V_BATTERY_LOW);
            if (g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
        }
    }
    break;
    case 3:
    {
    	/*
        static prog_uint8_t APM beepTab[]= {
            // 0   1   2   3    4
            0,  0,  0,  0,   0, //quiet
            0,  1,  8, 30, 100, //silent
            1,  1,  8, 30, 100, //normal
            1,  1, 15, 50, 150, //for motor
            10, 10, 30, 50, 150, //for motor
        };
        memcpy_P(g_beepVal,beepTab+5*g_eeGeneral.beeperVal,5);
        //g_beepVal = BEEP_VAL;
        */
        /* all this gone and replaced in new sound system */
    }
    break;
    }


    stickMoved = 0; //reset this flag

}

int16_t g_ppmIns[8];
uint8_t ppmInState = 0; //0=unsync 1..8= wait for value i-1

#ifndef SIMU
#include <avr/interrupt.h>
#endif

//#include <avr/wdt.h>

//class AutoLock
//{
//  uint8_t m_saveFlags;
//public:
//  AutoLock(){
//    m_saveFlags = SREG;
//    cli();
//  };
//  ~AutoLock(){
//    if(m_saveFlags & (1<<SREG_I)) sei();
//    //SREG = m_saveFlags;// & (1<<SREG_I)) sei();
//  };
//};

//#define STARTADCONV (ADCSRA  = (1<<ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2) | (1<<ADSC) | (1 << ADIE))
int16_t BandGap = 240 ;

#ifndef SIMU
static uint16_t s_anaFilt[8];
uint16_t anaIn(uint8_t chan)
{
    //                     ana-in:   3 1 2 0 4 5 6 7
    //static prog_char APM crossAna[]={4,2,3,1,5,6,7,0}; // wenn schon Tabelle, dann muss sich auch lohnen

//    const static prog_char APM crossAna[]={3,1,2,0,4,5,6,7};
	uint8_t pchan = chan ;
	if ( chan == 3 )
	{
		pchan = 0 ;		
	}
	else
	{
		if ( chan == 0 )
		{
			pchan = 3 ;			
		}
	}
//    volatile uint16_t *p = &s_anaFilt[chan];
    //  AutoLock autoLock;
//    return  *p;
  uint16_t temp = s_anaFilt[pchan] ;
	if ( chan < 4 )	// A stick
	{
		if ( g_eeGeneral.stickReverse & ( 1 << chan ) )
		{
			temp = 2048 - temp ;
		}
	}
	return temp ;
}


#define ADC_VREF_TYPE 0x40
//void getADC_filt()
//{
//    static uint16_t t_ana[2][8];
//    //	uint8_t thro_rev_chan = g_eeGeneral.throttleReversed ? THR_STICK : 10 ;  // 10 means don't reverse
//    for (uint8_t adc_input=0;adc_input<8;adc_input++){
//        ADMUX=adc_input|ADC_VREF_TYPE;
//        // Start the AD conversion
//        ADCSRA|=0x40;
//        // Do this while waiting
//        s_anaFilt[adc_input] = (s_anaFilt[adc_input]/2 + t_ana[1][adc_input]) & 0xFFFE; //gain of 2 on last conversion - clear last bit
//        //t_ana[2][adc_input]  =  (t_ana[2][adc_input]  + t_ana[1][adc_input]) >> 1;
//        t_ana[1][adc_input]  = (t_ana[1][adc_input]  + t_ana[0][adc_input]) >> 1;

//        // Now wait for the AD conversion to complete
//        while ((ADCSRA & 0x10)==0);
//        ADCSRA|=0x10;

//        uint16_t v = ADCW;
//        //      if(adc_input == thro_rev_chan) v = 1024 - v;
//        t_ana[0][adc_input]  = (t_ana[0][adc_input]  + v) >> 1;
//    }
//}
/*
  s_anaFilt[chan] = (s_anaFilt[chan] + sss_ana[chan]) >> 1;
  sss_ana[chan] = (sss_ana[chan] + ss_ana[chan]) >> 1;
  ss_ana[chan] = (ss_ana[chan] + s_ana[chan]) >> 1;
  s_ana[chan] = (ADC + s_ana[chan]) >> 1;
  */

void getADC_osmp()
{
    //  uint16_t temp_ana[8] = {0};
    uint16_t temp_ana ;
    //	uint8_t thro_rev_chan = g_eeGeneral.throttleReversed ? THR_STICK : 10 ;  // 10 means don't reverse
    for (uint8_t adc_input=0;adc_input<8;adc_input++){
//        temp_ana = 0 ;
//        for (uint8_t i=0; i<2;i++) {  // Going from 10bits to 11 bits.  Addition = n.  Loop 2 times
            ADMUX=adc_input|ADC_VREF_TYPE;
            // Start the AD conversion
#if defined(CPUM128) || defined(CPUM2561)
			asm(" rjmp 1f") ;
			asm("1:") ;
			asm(" rjmp 1f") ;
			asm("1:") ;
#endif

            ADCSRA|=0x40;
            // Wait for the AD conversion to complete
            while (ADCSRA & 0x40);
//            ADCSRA|=0x10;
            //      temp_ana[adc_input] += ADCW;
            temp_ana = ADC;
            ADCSRA|=0x40;
            // Wait for the AD conversion to complete
            while (ADCSRA & 0x40);
//        }

#if defined(CPUM128) || defined(CPUM2561)
            temp_ana += ADC;
            ADCSRA|=0x40;
            // Wait for the AD conversion to complete
            while (ADCSRA & 0x40);
            temp_ana += ADC;
            ADCSRA|=0x40;
            // Wait for the AD conversion to complete
            while (ADCSRA & 0x40);
            temp_ana += ADC;
            temp_ana >>= 1 ;
#else

//        temp_ana /= 2; // divide by 2^n to normalize result.
        //    if(adc_input == thro_rev_chan)
        //        temp_ana = 2048 -temp_ana;

        //		s_anaFilt[adc_input] = temp_ana[adc_input] / 2; // divide by 2^n to normalize result.
				temp_ana += ADC ;
#endif
        s_anaFilt[adc_input] = temp_ana ;
        //    if(IS_THROTTLE(adc_input) && g_eeGeneral.throttleReversed)
        //        s_anaFilt[adc_input] = 2048 - s_anaFilt[adc_input];
    }
}


//void getADC_single()
//{
//    uint16_t result ;
//    //	  uint8_t thro_rev_chan = g_eeGeneral.throttleReversed ? THR_STICK : 10 ;  // 10 means don't reverse
//    for (uint8_t adc_input=0;adc_input<8;adc_input++){
//        ADMUX=adc_input|ADC_VREF_TYPE;
//        // Start the AD conversion
//        ADCSRA|=0x40;
//        // Wait for the AD conversion to complete
//        while ((ADCSRA & 0x10)==0);
//        ADCSRA|=0x10;
//        result = ADCW * 2; // use 11 bit numbers

//        //      if(adc_input == thro_rev_chan)
//        //          result = 2048 - result ;
//        s_anaFilt[adc_input] = result ; // use 11 bit numbers
//    }
//}

static void getADC_bandgap()
{
    ADMUX=0x1E|ADC_VREF_TYPE;
    // Start the AD conversion
    //  ADCSRA|=0x40;
    // Wait for the AD conversion to complete
    //  while ((ADCSRA & 0x10)==0);
    //  ADCSRA|=0x10;
    // Do it twice, first conversion may be wrong
    ADCSRA|=0x40;
    // Wait for the AD conversion to complete
    while (ADCSRA & 0x40) ;
//    ADCSRA|=0x10;
    BandGap = (BandGap * 7 + ADC + 4 ) >> 3 ;
    //  if(BandGap<256)
    //      BandGap = 256;
}

//getADCp getADC[3] = {
//  getADC_single,
//  getADC_osmp,
//  getADC_filt
//  };
#endif

volatile uint8_t g_tmr16KHz;


#ifndef SIMU
ISR(TIMER0_OVF_vect, ISR_NOBLOCK) //continuous timer 16ms (16MHz/1024)
{
    g_tmr16KHz++;
}

static uint16_t getTmr16KHz()
{
    while(1){
        uint8_t hb  = g_tmr16KHz;
        uint8_t lb  = TCNT0;
        if(hb-g_tmr16KHz==0) return (hb<<8)|lb;
    }
}

// Clocks every 128 uS
ISR(TIMER2_OVF_vect, ISR_NOBLOCK) //10ms timer
{
  cli();
#ifdef CPUM2561
  TIMSK2 &= ~ (1<<TOIE2) ; //stop reentrance
#else
  TIMSK &= ~ (1<<TOIE2) ; //stop reentrance
#endif
  sei();
  
	AUDIO_DRIVER();  // the tone generator
	// Now handle the Voice output
	// Check for LcdLocked (in interrupt), and voice_enabled
	if ( g_eeGeneral.speakerMode & 2 )
	{
		if ( LcdLock == 0 )		// LCD not in use
		{
			struct t_voice *vptr ;
			vptr = &Voice ;
			FORCE_INDIRECT(vptr) ;
			if ( vptr->VoiceState == V_CLOCKING )
			{
				if ( vptr->VoiceTimer )
				{
					vptr->VoiceTimer -= 1 ;
				}
				else
				{
					uint8_t tVoiceLatch = vptr->VoiceLatch ;
					
					PORTB |= (1<<OUT_B_LIGHT) ;				// Latch clock high
					if ( ( vptr->VoiceCounter & 1 ) == 0 )
					{
						tVoiceLatch &= ~VOICE_DATA_BIT ;
						if ( vptr->VoiceSerial & 0x4000 )
						{
							tVoiceLatch |= VOICE_DATA_BIT ;
						}
						vptr->VoiceSerial <<= 1 ;
					}
					tVoiceLatch ^= VOICE_CLOCK_BIT ;
					vptr->VoiceLatch = PORTA_LCD_DAT = tVoiceLatch ;			// Latch data set
					PORTB &= ~(1<<OUT_B_LIGHT) ;			// Latch clock low
					if ( --vptr->VoiceCounter == 0 )
					{
						vptr->VoiceState = V_WAIT_BUSY_ON ;
						vptr->VoiceTimer = 5 ;		// 50 mS
					}
				}
			}
		}
	}
  cli();
#ifdef CPUM2561
  TIMSK2 |= (1<<TOIE2) ;
#else
  TIMSK |= (1<<TOIE2) ;
#endif
  sei();
}

// Clocks every 10 mS
#ifdef CPUM2561
ISR(TIMER0_COMPA_vect, ISR_NOBLOCK) //10ms timer
#else
ISR(TIMER0_COMP_vect, ISR_NOBLOCK) //10ms timer
#endif
{ 
#ifdef CPUM2561
  OCR0A += 156 ;			// Interrupt every 128 uS
#else
  OCR0 += 156 ;			// Interrupt every 128 uS
#endif
//  static uint8_t cnt10ms = 77; // execute 10ms code once every 78 ISRs
//  if (cnt10ms-- == 0) { // BEGIN { ... every 10ms ... }
//    // Begin 10ms event
//    cnt10ms = 77;
		
		AUDIO_HEARTBEAT();  // the queue processing

        per10ms();
#ifdef FRSKY
		check_frsky() ;
#endif
        heartbeat |= HEART_TIMER10ms;
	// See if time for alarm checking
		struct t_alarmControl *pac = &AlarmControl ;
		FORCE_INDIRECT(pac) ;

		if (--pac->AlarmTimer == 0 )
		{
			pac->AlarmTimer = 100 ;		// Restart timer
//			pac->AlarmCheckFlag += 1 ;	// Flag time to check alarms
			pac->OneSecFlag = 1 ;
		}
		if (--pac->VoiceFtimer == 0 )
		{
			pac->VoiceFtimer = 10 ;		// Restart timer
			pac->VoiceCheckFlag = 1 ;	// Flag time to check alarms
		}

//  } // end 10ms event

}


// Timer3 used for PPM_IN pulse width capture. Counter running at 16MHz / 8 = 2MHz
// equating to one count every half millisecond. (2 counts = 1ms). Control channel
// count delta values thus can range from about 1600 to 4400 counts (800us to 2200us),
// corresponding to a PPM signal in the range 0.8ms to 2.2ms (1.5ms at center).
// (The timer is free-running and is thus not reset to zero at each capture interval.)
ISR(TIMER3_CAPT_vect, ISR_NOBLOCK) //capture ppm in 16MHz / 8 = 2MHz
{
    uint16_t capture=ICR3;
    cli();
#ifdef CPUM2561
    TIMSK3 &= ~(1<<ICIE3); //stop reentrance
#else
    ETIMSK &= ~(1<<TICIE3); //stop reentrance
#endif
    sei();

    static uint16_t lastCapt;
    uint16_t val = (capture - lastCapt) / 2;
    lastCapt = capture;

    // We prcoess g_ppmInsright here to make servo movement as smooth as possible
    //    while under trainee control
  	if (val>4000 && val < 16000) // G: Prioritize reset pulse. (Needed when less than 8 incoming pulses)
  	  ppmInState = 1; // triggered
  	else
  	{
  		if(ppmInState && ppmInState<=8)
			{
  	  	if(val>800 && val<2200)
				{
					ppmInValid = 100 ;
  		    g_ppmIns[ppmInState++ - 1] =
  	  	    (int16_t)(val - 1500)* (uint8_t)(g_eeGeneral.PPM_Multiplier+10)/10; //+-500 != 512, but close enough.

		    }else{
  		    ppmInState=0; // not triggered
  	  	}
  	  }
  	}

    cli();
#ifdef CPUM2561
    TIMSK3 |= (1<<ICIE3);
#else
    ETIMSK |= (1<<TICIE3);
#endif
    sei();
}

extern struct t_latency g_latency ;
//void main(void) __attribute__((noreturn));

#if STACK_TRACE
extern unsigned char __bss_end ;

unsigned int stack_free()
{
    unsigned char *p ;

    p = &__bss_end + 1 ;
    while ( *p == 0x55 )
    {
        p+= 1 ;
    }
    return p - &__bss_end ;
}
#endif

#ifdef CPUM2561
uint8_t SaveMcusr ;
#endif

int main(void)
{

    DDRA = 0xff;  PORTA = 0x00;
    DDRB = 0x81;  PORTB = 0x7e; //pullups keys+nc
    DDRC = 0x3e;  PORTC = 0xc1; //pullups nc
    DDRD = 0x00;  PORTD = 0xff; //all D inputs pullups keys
    DDRE = 0x08;  PORTE = 0xff-(1<<OUT_E_BUZZER); //pullups + buzzer 0
    DDRF = 0x00;  PORTF = 0x00; //all F inputs anain - pullups are off
    //DDRG = 0x10;  PORTG = 0xff; //pullups + SIM_CTL=1 = phonejack = ppm_in
    DDRG = 0x14; PORTG = 0xfB; //pullups + SIM_CTL=1 = phonejack = ppm_in, Haptic output and off (0)

#ifdef CPUM2561
  uint8_t mcusr = MCUSR; // save the WDT (etc) flags
	SaveMcusr = mcusr ;
  MCUSR = 0; // must be zeroed before disabling the WDT
#else
  uint8_t mcusr = MCUCSR;
  MCUCSR = 0;
#endif

#ifdef CPUM2561
	if ( mcusr == 0 )
	{
    wdt_enable(WDTO_60MS) ;
	}
#endif

#ifdef BLIGHT_DEBUG
	{
		uint32_t x ;
		x = 0 ;
		for ( ;; )
		{
			for ( x = 0 ; x < 500000 ; x += 1 )
			{
				PORTB |= 0x80 ;	// Backlight on
	      wdt_reset() ;
				asm(" nop") ;											// delay to allow input to settle
				asm(" nop") ;											// delay to allow input to settle
			}
			for ( x = 0 ; x < 500000 ; x += 1 )
			{
				PORTB &= 0x7F ;	// Backlight off
	      wdt_reset() ;
				asm(" nop") ;											// delay to allow input to settle
				asm(" nop") ;											// delay to allow input to settle
				asm(" nop") ;											// delay to allow input to settle
				asm(" nop") ;											// delay to allow input to settle
				asm(" nop") ;											// delay to allow input to settle
				asm(" nop") ;											// delay to allow input to settle

			}
		}
	}

#endif

//		PORTB |= (1<<OUT_B_LIGHT) ;				// Latch clock high
//		PORTA_LCD_DAT = 0 ; // VOICE_CLOCK_BIT ;			// Latch data set
//		Voice.VoiceLatch = 0 ; // VOICE_CLOCK_BIT ;
//		PORTB &= ~(1<<OUT_B_LIGHT) ;			// Latch clock low

#ifdef JETI
    JETI_Init();
#endif


#ifdef ARDUPILOT
    ARDUPILOT_Init();
#endif

#ifdef NMEA
    NMEA_Init();
#endif


    ADMUX=ADC_VREF_TYPE;
    ADCSRA=0x85 ;

    // TCNT0         10ms = 16MHz/160000  periodic timer
    //TCCR0  = (1<<WGM01)|(7 << CS00);//  CTC mode, clk/1024
#ifdef CPUM2561
    TCCR0B  = (5 << CS00);//  Norm mode, clk/1024
    OCR0A   = 156;
		TIMSK0 |= (1<<OCIE0A) | (1<<TOIE0) ;
    
		TCCR2B  = (2 << CS00);//  Norm mode, clk/8
		TIMSK2 |= (1<<TOIE2) ;
#else
    TCCR0  = (7 << CS00);//  Norm mode, clk/1024
    OCR0   = 156;
    TCCR2  = (2 << CS00);//  Norm mode, clk/8
		TIMSK |= (1<<OCIE0) | (1<<TOIE0) | (1<<TOIE2) ;
#endif
    // TCNT1 2MHz Pulse generator
    TCCR1A = (0<<WGM10);
    TCCR1B = (1 << WGM12) | (2<<CS10); // CTC OCR1A, 16MHz / 8
    //TIMSK |= (1<<OCIE1A); enable immediately before mainloop

    TCCR3A  = 0;
    TCCR3B  = (1<<ICNC3) | (2<<CS30);      //ICNC3 16MHz / 8
#ifdef CPUM2561
    TIMSK3 |= (1<<ICIE3);
#else
    ETIMSK |= (1<<TICIE3);
#endif

#if STACK_TRACE
    // Init Stack while interrupts are disabled
#define STACKPTR     _SFR_IO16(0x3D)
    {

        unsigned char *p ;
        unsigned char *q ;

        p = (unsigned char *) STACKPTR ;
        q = &__bss_end ;
        p -= 2 ;
        while ( p > q )
        {
            *p-- = 0x55 ;
        }
    }
#endif
		sei(); //damit alert in eeReadGeneral() nicht haengt

    g_menuStack[0] =  menuProc0;

	if (eeReadGeneral())
	{
		lcd_init() ;   // initialize LCD module after reading eeprom
  }
	else
  {
    eeGeneralDefault(); // init g_eeGeneral with default values
    lcd_init();         // initialize LCD module for ALERT box
    eeWriteGeneral();   // format/write back to eeprom
	}
	uint8_t cModel = g_eeGeneral.currModel;
	eeLoadModel( cModel ) ;
  
    
#ifdef FRSKY
    FRSKY_Init( 0 ) ;
#endif
		
		checkQuickSelect();

//		lcdSetContrast() ;
//    if(g_eeGeneral.lightSw || g_eeGeneral.lightAutoOff || g_eeGeneral.lightOnStickMove) // if lightswitch is defined or auto off
//        BACKLIGHT_ON;
//    else
//        BACKLIGHT_OFF;

    //we assume that startup is like pressing a switch and moving sticks.  Hence the lightcounter is set
    //if we have a switch on backlight it will be able to turn on the backlight.

		{
//			uint8_t sm ;
//			sm = stickMoved ;
			stickMoved = 1 ;
			doBackLightVoice(1) ;
			stickMoved = 0 ;

//    loc = g_eeGeneral.lightOnStickMove ;
//    if(g_eeGeneral.lightAutoOff > g_eeGeneral.lightOnStickMove)
//      loc = g_eeGeneral.lightAutoOff ;
    
//		g_LightOffCounter = (loc*250)<<1;

//    check_backlight_voice();
		}
    // moved here and logic added to only play statup tone if splash screen enabled.
    // that way we save a bit, but keep the option for end users!
		setVolume(g_eeGeneral.volume+7) ;
//		putVoiceQueueLong( g_eeGeneral.volume + 0xFFF7 ) ;
    
  if ( ( mcusr & (1<<WDRF) ) == 0 )
	{
		if(!g_eeGeneral.disableSplashScreen)
    {
	    if( g_eeGeneral.speakerMode )		// Not just beeper
			{
				audioVoiceDefevent( AU_TADA, V_HELLO ) ;
      }
  	  doSplash();
    }
    checkMem();
    //setupAdc(); //before checkTHR
    getADC_osmp();
    g_vbat100mV = anaIn(7) / 14 ;
    checkTHR();
    checkSwitches();
    checkAlarm();
    checkWarnings();
    clearKeyEvents(); //make sure no keys are down before proceeding
//    BandGap = 240 ;
		putVoiceQueueUpper( g_model.modelVoice ) ;
	}
		CurrentPhase = 0 ;
    perOut(g_chans512, 0);
		startPulses() ;
    wdt_enable(WDTO_500MS);

//    pushMenu(menuProcModelSelect);
//    popMenu(true);  
    g_menuStack[1] = menuProcModelSelect ;	// this is so the first instance of [MENU LONG] doesn't freak out!

//		lcdSetContrast() ;

    if(cModel!=g_eeGeneral.currModel)
    {
        STORE_GENERALVARS ;    // if model was quick-selected, make sure it sticks
        //    eeDirty(EE_GENERAL); // if model was quick-selected, make sure it sticks
        eeWaitComplete() ;
    }
#ifdef FRSKY
    FrskyAlarmSendState |= 0x40 ;
#endif

    // This bit depends on protocol
//    if ( (g_model.protocol == PROTO_PPM) || (g_model.protocol == PROTO_PPM16) )
//		{
//	    OCR1A = 2000 ;        // set to 1mS
//#ifdef CPUM2561
//  	  TIFR1 = 1 << OCF1A ;   // Clear pending interrupt
//#else
//			TIFR = 1 << OCF1A ;   // Clear pending interrupt
//#endif
//	    PULSEGEN_ON; // Pulse generator enable immediately before mainloop
//		}
		Main_running = 1 ;
    while(1){
        //uint16_t old10ms=get_tmr10ms();
        mainSequence() ;
    }
}

#ifdef FRSKY
extern int16_t AltOffset ;


NOINLINE int16_t getTelemetryValue( uint8_t index )
{
	int16_t value ;
	int16_t *p ;
	p = &FrskyHubData[index] ;
	FORCE_INDIRECT(p) ;
	cli() ;
	value = *p ;
	sei() ;
	return value ;
}

int16_t getAltbaroWithOffset()
{
 	return getTelemetryValue(FR_ALT_BARO) + AltOffset ;
}
#endif


void mainSequence()
{
	CalcScaleNest = 0 ;
  
	uint16_t t0 = getTmr16KHz();
	uint8_t numSafety = 16 - g_model.numVoice ;
  //      getADC[g_eeGeneral.filterInput]();
//    if ( g_eeGeneral.filterInput == 1)
//    {
//        getADC_filt() ;
//    }
//    else if ( g_eeGeneral.filterInput == 2)
//    {
  getADC_osmp() ;
//    }
//    else
//    {
//        getADC_single() ;
//    }
  ADMUX=0x1E|ADC_VREF_TYPE;   // Select bandgap
	pollRotary() ;
  perMain();      // Give bandgap plenty of time to settle
  getADC_bandgap() ;
  //while(get_tmr10ms()==old10ms) sleep_mode();
  if(heartbeat == 0x3)
  {
      wdt_reset();
      heartbeat = 0;
  }
  t0 = getTmr16KHz() - t0;
  if ( t0 > g_latency.g_timeMain ) g_latency.g_timeMain = t0 ;
  
  if ( AlarmControl.VoiceCheckFlag )		// Every 100 mS
  {
		uint8_t i ;
		static uint16_t timer ;
    
		timer += 1 ;

#if defined(CPUM128) || defined(CPUM2561)
		for ( i = numSafety ; i < NUM_CHNOUT+EXTRA_VOICE_SW ; i += 1 )
#else
		for ( i = numSafety ; i < NUM_CHNOUT ; i += 1 )
#endif
		{
			uint8_t curent_state ;
			uint8_t mode ;
			uint8_t value ;
    	SafetySwData *sd = &g_model.safetySw[i];
#if defined(CPUM128) || defined(CPUM2561)
    	if ( i >= NUM_CHNOUT )
			{
				sd = &g_model.xvoiceSw[i-NUM_CHNOUT];
			}
#endif

			mode = sd->opt.vs.vmode ;
			value = sd->opt.vs.vval ;
			if ( mode <= 5 )
			{
				if ( value > 250 )
				{
					value = g_model.gvars[value-248].gvar ; //Gvars 3-7
				}
			}

			if ( sd->opt.vs.vswtch )		// Configured
			{
				curent_state = getSwitch( sd->opt.vs.vswtch, 0 ) ;
				if ( AlarmControl.VoiceCheckFlag != 2 )
				{
					if ( ( mode == 0 ) || ( mode == 2 ) )
					{ // ON
						if ( ( Vs_state[i] == 0 ) && curent_state )
						{
							putVoiceQueue( value ) ;
						}
					}
					if ( ( mode == 1 ) || ( mode == 2 ) )
					{ // OFF
						if ( ( Vs_state[i] == 1 ) && !curent_state )
						{
//							uint8_t x ;
//							x = sd->opt.vs.vval ;
							if ( mode == 2 )
							{
								value += 1 ;							
							}
							putVoiceQueue( value ) ;
						}
					}
					if ( mode > 5 )
					{
						if ( ( Vs_state[i] == 0 ) && curent_state )
						{
							voice_telem_item( sd->opt.vs.vval ) ;
						}					
					}
					else if ( mode > 2 )
					{ // 15, 30 or 60 secs
						if ( curent_state )
						{
							uint16_t mask ;
							mask = 150 ;
							if ( mode == 4 ) mask = 300 ;
							if ( mode == 5 ) mask = 600 ;
							if ( timer % mask == 0 )
							{
								putVoiceQueue( value ) ;
							}
						}
					}
				}
				Vs_state[i] = curent_state ;
			}
		}
		
		for ( i = 0 ; i < NUM_CSW ; i += 1 )
		{
    	CSwData *cs = &g_model.customSw[i];
    	uint8_t cstate = CS_STATE(cs->func);

    	if(cstate == CS_TIMER)
			{
				int16_t y ;
				y = CsTimer[i] ;
				if ( y == 0 )
				{
					int8_t z ;
					z = cs->v1 ;
					if ( z >= 0 )
					{
						z = -z-1 ;
						y = z * 10 ;					
					}
					else
					{
						y = z ;
					}
				}
				else if ( y < 0 )
				{
					if ( ++y == 0 )
					{
						int8_t z ;
						z = cs->v2 ;
						if ( z >= 0 )
						{
							z += 1 ;
							y = z * 10 - 1  ;
						}
						else
						{
							y = -z-1 ;
						}
					}
				}
				else  // if ( CsTimer[i] > 0 )
				{
					y -= 1 ;
				}
				if ( cs->andsw )
				{
					int8_t x ;
					x = cs->andsw ;
					if ( x > 8 )
					{
						x += 1 ;
					}
	        if (getSwitch( x, 0, 0) == 0 )
				  {
						y = -1 ;
					}	
				}
				CsTimer[i] = y ;
			}
#ifdef VERSION3
			if ( cs->func == CS_LATCH )
			{
		    if (getSwitch( cs->v1, 0, 0) )
				{
					Last_switch[i] = 1 ;
				}
				else
				{
			    if (getSwitch( cs->v2, 0, 0) )
					{
						Last_switch[i] = 0 ;
					}
				}
			}
			if ( cs->func == CS_FLIP )
			{
		    if (getSwitch( cs->v1, 0, 0) )
				{
					if ( ( Last_switch[i] & 2 ) == 0 )
					{
						// Clock it!
			      if (getSwitch( cs->v2, 0, 0) )
						{
							Last_switch[i] = 3 ;
						}
						else
						{
							Last_switch[i] = 2 ;
						}
					}
				}
				else
				{
					Last_switch[i] &= ~2 ;
				}
			}
#endif
		}

#if defined(CPUM128) || defined(CPUM2561)
		for ( i = NUM_CSW ; i < NUM_CSW+EXTRA_CSW ; i += 1 )
		{
    	CxSwData *cs = &g_model.xcustomSw[i-NUM_CSW];
    	
			uint8_t cstate = CS_STATE(cs->func);

    	if(cstate == CS_TIMER)
			{
				int16_t y ;
				y = CsTimer[i] ;
				if ( y == 0 )
				{
					int8_t z ;
					z = cs->v1 ;
					if ( z >= 0 )
					{
						z = -z-1 ;
						y = z * 10 ;					
					}
					else
					{
						y = z ;
					}
				}
				else if ( y < 0 )
				{
					if ( ++y == 0 )
					{
						int8_t z ;
						z = cs->v2 ;
						if ( z >= 0 )
						{
							z += 1 ;
							y = z * 10 - 1  ;
						}
						else
						{
							y = -z-1 ;
						}
					}
				}
				else  // if ( CsTimer[i] > 0 )
				{
					y -= 1 ;
				}
				if ( cs->andsw )
				{
					int8_t x ;
					x = cs->andsw ;
					if ( x > 8 )
					{
						x += 1 ;
					}
					if ( x < -8 )
					{
						x -= 1 ;
					}
					if ( x > 9+NUM_CSW )
					{
						x = 9 ;			// Tag TRN on the end, keep EEPROM values
					}
					if ( x < -(9+NUM_CSW) )
					{
						x = -9 ;			// Tag TRN on the end, keep EEPROM values
					}
	        if (getSwitch( x, 0, 0) == 0 )
				  {
						y = -1 ;
					}	
				}
				CsTimer[i] = y ;
			}
#ifdef VERSION3
			if ( cs->func == CS_LATCH )
			{
		    if (getSwitch( cs->v1, 0, 0) )
				{
					Last_switch[i] = 1 ;
				}
				else
				{
			    if (getSwitch( cs->v2, 0, 0) )
					{
						Last_switch[i] = 0 ;
					}
				}
			}
			if ( cs->func == CS_FLIP )
			{
		    if (getSwitch( cs->v1, 0, 0) )
				{
					if ( ( Last_switch[i] & 2 ) == 0 )
					{
						// Clock it!
			      if (getSwitch( cs->v2, 0, 0) )
						{
							Last_switch[i] = 3 ;
						}
						else
						{
							Last_switch[i] = 2 ;
						}
					}
				}
				else
				{
					Last_switch[i] &= ~2 ;
				}
			}
#endif
		}
#endif
		AlarmControl.VoiceCheckFlag = 0 ;
		
#ifdef FRSKY
		// Vario
	  {

			static uint8_t varioRepeatRate = 0 ;
			
			if ( g_model.varioData.varioSource ) // Vario enabled
			{
				if ( getSwitch( g_model.varioData.swtch, 0, 0 ) )
				{
					uint8_t new_rate = 0 ;
					if ( varioRepeatRate )
					{
						varioRepeatRate -= 1 ;
					}
					if ( varioRepeatRate == 0 )
					{
						int16_t vspd ;
						if ( g_model.varioData.varioSource == 1 )
						{
							vspd = getTelemetryValue(FR_VSPD) ;
							if ( g_model.varioData.param > 1 )
							{
								vspd /= g_model.varioData.param ;							
							}
						}
						else // VarioSetup.varioSource == 2
						{
							vspd = getTelemetryValue(FR_A2_COPY) - 128 ;
							if ( ( vspd < 3 ) && ( vspd > -3 ) )
							{
								vspd = 0 ;							
							}
							vspd *= g_model.varioData.param ;
						}
						if ( vspd )
						{
							{
								if ( vspd < 0 )
								{
									vspd = -vspd ;
									if (!g_model.varioData.sinkTones )
									{
          		    	audio.event( AU_VARIO_DOWN ) ;
									}
								}
								else
								{
          		    audio.event( AU_VARIO_UP ) ;
								}
								if ( vspd < 75 )
								{
									new_rate = 8 ;
								}
								else if ( vspd < 125 )
								{
									new_rate = 6 ;
								}
								else if ( vspd < 175 )
								{
									new_rate = 4 ;
								}
								else
								{
									new_rate = 2 ;
								}
							}
						}
						else
						{
							if (g_model.varioData.sinkTones == 0 )
							{
								new_rate = 20 ;
         		    audio.event( AU_VARIO_UP ) ;
							}
						}
						varioRepeatRate = new_rate ;
					}
				}
			}
		}	
#endif // FrSky
	}
	
	if ( AlarmControl.OneSecFlag )		// Custom Switch Timers
  {
		uint8_t i ;
//      AlarmControl.AlarmCheckFlag = 0 ;
      // Check for alarms here
      // Including Altitude limit
//				Debug3 = 1 ;
//		MixRate = MixCounter ;
//		MixCounter = 0 ;
#ifdef FRSKY
      if (frskyUsrStreaming)
      {
#if ALT_ALARM
          int16_t limit ; //= g_model.FrSkyAltAlarm ;
          int16_t altitude ;
          if ( g_model.FrSkyAltAlarm )
          {
              if (g_model.FrSkyAltAlarm == 2)  // 400
              {
                  limit = 400 ;	//ft
              }
              else
              {
                  limit = 122 ;	//m
              }
							altitude = getAltbaroWithOffset() ;
//								if ( AltitudeDecimals )
//								{
								altitude /= 10 ;									
//								}
							if (g_model.FrSkyUsrProto == 0)  // Hub
							{
      					if ( g_model.FrSkyImperial )
								{
        					altitude = m_to_ft( altitude ) ;
								}
							}
              if ( altitude > limit )
              {
                  audioDefevent(AU_WARNING2) ;
              }
          }
#endif
					uint16_t total_volts = 0 ;
					uint8_t audio_sounded = 0 ;
					uint8_t low_cell = 220 ;		// 4.4V
				  for (uint8_t k=0; k<FrskyBattCells; k++)
					{
						total_volts += FrskyVolts[k] ;
						if ( FrskyVolts[k] < low_cell )
						{
							low_cell = FrskyVolts[k] ;
						}

#if VOLT_THRESHOLD
						if ( audio_sounded == 0 )
						{
	        		if ( FrskyVolts[k] < g_model.frSkyVoltThreshold )
							{
	            	audioDefevent(AU_WARNING3);
								audio_sounded = 1 ;
			        }
						}
#endif
	  			}
					// Now we have total volts available
					FrskyHubData[FR_CELLS_TOT] = total_volts / 5 ;
					if ( low_cell < 220 )
					{
						FrskyHubData[FR_CELL_MIN] = low_cell ;
					}

      }


      // this var prevents and alarm sounding if an earlier alarm is already sounding
      // firing two alarms at once is pointless and sounds rubbish!
      // this also means channel A alarms always over ride same level alarms on channel B
      // up to debate if this is correct!
      //				bool AlarmRaisedAlready = false;

      if (frskyStreaming)
			{
//            enum AlarmLevel level[4] ;
//            // RED ALERTS
//            if( (level[0]=FRSKY_alarmRaised(0,0)) == alarm_red) FRSKY_alarmPlay(0,0);
//            else if( (level[1]=FRSKY_alarmRaised(0,1)) == alarm_red) FRSKY_alarmPlay(0,1);
//            else	if( (level[2]=FRSKY_alarmRaised(1,0)) == alarm_red) FRSKY_alarmPlay(1,0);
//            else if( (level[3]=FRSKY_alarmRaised(1,1)) == alarm_red) FRSKY_alarmPlay(1,1);
//            // ORANGE ALERTS
//            else	if( level[0] == alarm_orange) FRSKY_alarmPlay(0,0);
//            else if( level[1] == alarm_orange) FRSKY_alarmPlay(0,1);
//            else	if( level[2] == alarm_orange) FRSKY_alarmPlay(1,0);
//            else if( level[3] == alarm_orange) FRSKY_alarmPlay(1,1);
//            // YELLOW ALERTS
//            else	if( level[0] == alarm_yellow) FRSKY_alarmPlay(0,0);
//            else if( level[1] == alarm_yellow) FRSKY_alarmPlay(0,1);
//            else	if( level[2] == alarm_yellow) FRSKY_alarmPlay(1,0);
//            else if( level[3] == alarm_yellow) FRSKY_alarmPlay(1,1);

					// Check for current alarm
#ifdef MAH_LIMIT			
					if ( g_model.currentSource )
					{
						if ( g_model.frsky.frskyAlarmLimit )
						{
							if ( ( FrskyHubData[FR_AMP_MAH] >> 6 ) >= g_model.frsky.frskyAlarmLimit )
							{
								if ( g_eeGeneral.speakerMode & 2 )
								{
									putVoiceQueue( V_CAPACITY ) ;
								}
								else
								{
									audio.event( g_model.frsky.frskyAlarmSound ) ;
								}
							}
						}
					}
#endif // MAH_LIMIT
				
//					struct t_FrSkyChannelData *aaa = &g_model.frsky.channels[0] ;
//        	for (int i=0; i<2; i++)
//					{
//						// To be enhanced by checking the type as well
//       		  if (aaa->opt.alarm.ratio)
//						{
//     		      if ( aaa->opt.alarm.type == 3 )		// Current (A)
//							{
//								if ( g_model.frsky.frskyAlarmLimit )
//								{
//    		          if ( (  getTelemetryValue(FR_A1_MAH+i) >> 6 ) >= g_model.frsky.frskyAlarmLimit )
//									{
//										if ( g_eeGeneral.speakerMode & 2 )
//										{
//											putVoiceQueue( V_CAPACITY ) ;
//										}
//										else
//										{
//											audio.event( g_model.frsky.frskyAlarmSound ) ;
//										}
//									}
//								}
//							}
//       		  }
//						aaa += 1 ;
//        	}
      }
#endif

			// Now for the Safety/alarm switch alarms
			// Carried out evey 100 mS
			{
				static uint8_t periodCounter ;
				uint8_t pCounter = periodCounter ;
					
				pCounter += 0x11 ;
				if ( ( pCounter & 0x0F ) > 11 )
				{
					pCounter &= 0xF0 ;
				}
				periodCounter = pCounter ;
				for ( i = 0 ; i < numSafety ; i += 1 )
				{
    			SafetySwData *sd = &g_model.safetySw[i] ;
					if (sd->opt.ss.mode == 1)
					{
						if ( ( pCounter & 0x30 ) == 0 )
						{
							if(getSwitch( sd->opt.ss.swtch,0))
							{
								audio.event( ((g_eeGeneral.speakerMode & 1) == 0) ? 1 : sd->opt.ss.val ) ;
							}
						}
					}
					if (sd->opt.ss.mode == 2)
					{
						if ( sd->opt.ss.swtch > MAX_DRSWITCH )
						{
							switch ( sd->opt.ss.swtch - MAX_DRSWITCH -1 )
							{
								case 0 :
									if ( ( pCounter & 0x70 ) == 0 )
									{
										voice_telem_item( sd->opt.ss.val ) ;
									}
								break ;
								case 1 :
									if ( ( pCounter & 0x0F ) == 0 )
									{
										voice_telem_item( sd->opt.ss.val ) ;
									}
								break ;
								case 2 :
									if ( ( pCounter & 0xF0 ) == 0x20 )
									{
										voice_telem_item( sd->opt.ss.val ) ;
									}
								break ;
							}
						}
						else if ( ( pCounter & 0x30 ) == 0 )		// Every 4 seconds
						{
							if(getSwitch( sd->opt.ss.swtch,0))
							{
								putVoiceQueue( sd->opt.ss.val + 128 ) ;
							}
						}
					}
				}
			}
	// New switch voices
	// New entries, Switch, (on/off/both), voice file index

		AlarmControl.OneSecFlag = 0 ;
//		uint8_t i ;
		
		if ( StickScrollTimer )
		{
			StickScrollTimer -= 1 ;				
		}
	}
}
#endif


int16_t calc1000toRESX(int16_t x)  // improve calc time by Pat MacKenzie
{
    int16_t y = x>>5;
    x+=y;
    y=y>>2;
    x-=y;
    return x+(y>>2);
    //  return x + x/32 - x/128 + x/512;
}

#if GVARS
int8_t REG100_100(int8_t x)
{
	return REG( x, -100, 100 ) ;
}

int8_t REG(int8_t x, int8_t min, int8_t max)
{
  int8_t result = x;
  if (x >= 126 || x <= -126) {
    x = (uint8_t)x - 126;
    result = g_model.gvars[x].gvar ;
    if (result < min) {
      g_model.gvars[x].gvar = result = min;
//      eeDirty( EE_MODEL | EE_TRIM ) ;
    }
    else if (result > max) {
      g_model.gvars[x].gvar = result = max;
//      eeDirty( EE_MODEL | EE_TRIM ) ;
    }
  }
  return result;
}
#endif

uint8_t IS_EXPO_THROTTLE( uint8_t x )
{
	if ( g_model.thrExpo )
	{
		return IS_THROTTLE( x ) ;
	}
	return 0 ;
}

#ifndef FIX_MODE
uint8_t IS_THROTTLE( uint8_t x )
{
	uint8_t y ;
	y = g_eeGeneral.stickMode&1 ;
	y = 2 - y ;
	return (((y) == x) && (x<4)) ;
}
#endif

int16_t calc100toRESX(int8_t x)
{
    return ((x*41)>>2) - x/64;
}


