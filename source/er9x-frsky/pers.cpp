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
#include "templates.h"
#include "language.h"

#ifdef FRSKY
#include "frsky.h"
#endif

static void validateName( char *name, uint8_t size ) ;

const prog_char APM Str_EEPROM_Overflow[] =  STR_EE_OFLOW ;

EFile theFile;  //used for any file operation
EFile theFile2; //sometimes we need two files
EFile theWriteFile; //separate write file

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2
/*#define partCopy(sizeDst,sizeSrc)                         \
      pSrc -= (sizeSrc);                                  \
      pDst -= (sizeDst);                                  \
      memmove(pDst, pSrc, (sizeSrc));                     \
      memset (pDst+(sizeSrc), 0,  (sizeDst)-(sizeSrc));
#define fullCopy(size) partCopy(size,size)
*/
void eeGeneralDefault()
{
  memset(&g_eeGeneral,0,sizeof(g_eeGeneral));
  g_eeGeneral.myVers   =  MDVERS;
//  g_eeGeneral.currModel=  0;
  g_eeGeneral.contrast = LCD_NOMCONTRAST;
  g_eeGeneral.vBatWarn = 90;
  g_eeGeneral.stickMode=  1;
  for (uint8_t i = 0; i < 7; ++i) {
    g_eeGeneral.calibMid[i]     = 0x200;
    g_eeGeneral.calibSpanNeg[i] = 0x300;
    g_eeGeneral.calibSpanPos[i] = 0x300;
  }
  strncpy_P(g_eeGeneral.ownerName,PSTR(STR_ME),10);
  g_eeGeneral.chkSum = evalChkSum() ;
}

uint16_t evalChkSum()
{
  uint16_t sum=0;
	uint16_t *p ;
	p = ( uint16_t *)g_eeGeneral.calibMid ;
  for (int i=0; i<12;i++)
	{
    sum += *p++ ;
	}
  return sum;
}


static bool eeLoadGeneral()
{
  theFile.openRd(FILE_GENERAL);
  memset(&g_eeGeneral, 0, sizeof(EEGeneral));
//  uint8_t sz = theFile.readRlc((uint8_t*)&g_eeGeneral, sizeof(EEGeneral));
  theFile.readRlc((uint8_t*)&g_eeGeneral, sizeof(EEGeneral));

	validateName( g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName) ) ;

//  for(uint8_t i=0; i<sizeof(g_eeGeneral.ownerName);i++) // makes sure name is valid
//  {
//      uint8_t idx = char2idx(g_eeGeneral.ownerName[i]);
//      g_eeGeneral.ownerName[i] = idx2char(idx);
//  }

  if(g_eeGeneral.myVers<MDVERS)
	{
    sysFlags |= sysFLAG_OLD_EEPROM; // if old EEPROM - Raise flag

  	g_eeGeneral.myVers   =  MDVERS; // update myvers
		STORE_GENERALVARS;
	}
//  if(sz>(sizeof(EEGeneral)-20)) for(uint8_t i=0; i<12;i++) sum+=g_eeGeneral.calibMid[i];
  return g_eeGeneral.chkSum == evalChkSum() ;
}

void modelDefaultWrite(uint8_t id)
{
  memset(&g_model, 0, sizeof(ModelData));
  strncpy_P(g_model.name,PSTR(STR_MODEL), 10);
	div_t qr ;
	qr = div( id+1, 10 ) ;
  g_model.name[5]='0'+qr.quot;
  g_model.name[6]='0'+qr.rem;
#ifdef VERSION3
  g_model.modelVersion = 3 ;
#else
  g_model.modelVersion = 2 ;
#endif
	g_model.trimInc = 2 ;

#ifdef NO_TEMPLATES
  applyTemplate(); //default 4 channel template
#else
  applyTemplate(0); //default 4 channel template
#endif
  theFile.writeRlc(FILE_MODEL(id),FILE_TYP_MODEL,(uint8_t*)&g_model,sizeof(g_model),200);
}

void eeLoadModelName(uint8_t id,char*buf,uint8_t len)
{
  if(id<MAX_MODELS)
  {
    //eeprom_read_block(buf,(void*)modelEeOfs(id),sizeof(g_model.name));
    theFile.openRd(FILE_MODEL(id));
    memset(buf,' ',len);
    if(theFile.readRlc((uint8_t*)buf,sizeof(g_model.name)) == sizeof(g_model.name) )
    {
      uint16_t sz=theFile.size();
      buf+=len;
      while(sz)
			{
				div_t qr ;
				qr = div( sz, 10 ) ;
				--buf;
				*buf='0'+qr.rem;
				sz = qr.quot;
			}
    }
  }
}

//uint16_t eeFileSize(uint8_t id)
//{
//    theFile.openRd(FILE_MODEL(id));
//    return theFile.size();
//}

static void validateName( char *name, uint8_t size )
{
	for(uint8_t i=0; i<size;i++) // makes sure name is valid
  {
//		uint8_t idx = char2idx(name[i]);
		name[i] = idx2char(char2idx(name[i])) ;
	}
}


bool eeModelExists(uint8_t id)
{
    return EFile::exists(FILE_MODEL(id));
}

#ifdef FIX_MODE
extern MixData *mixaddress( uint8_t idx ) ;
#endif

void eeLoadModel(uint8_t id)
{
  if(id<MAX_MODELS)
  {
        theFile.openRd(FILE_MODEL(id));
        memset(&g_model, 0, sizeof(ModelData));
        uint16_t sz = theFile.readRlc((uint8_t*)&g_model, sizeof(g_model));

        if(sz<256) // if not loaded a fair amount
        {
            modelDefaultWrite(id);
        }
				validateName( g_model.name, sizeof(g_model.name) ) ;

//        for(uint8_t i=0; i<sizeof(g_model.name);i++) // makes sure name is valid
//        {
//            uint8_t idx = char2idx(g_model.name[i]);
//            g_model.name[i] = idx2char(idx);
//        }
		if ( g_model.numBlades == 0 )
		{
			g_model.numBlades = g_model.xnumBlades + 2 ;				
		}

        resetTimer1();
        resetTimer2();

#ifdef FRSKY
  FrskyAlarmSendState |= 0x40 ;		// Get RSSI Alarms
        FRSKY_setModelAlarms();
#endif
#ifdef FIX_MODE

// check for updating mix sources
		if ( g_model.modelVersion < 2 )
		{
    	for(uint8_t i=0;i<MAX_MIXERS;i++)
			{
        MixData *md = mixaddress( i ) ;
        if (md->srcRaw)
				{
        	if (md->srcRaw <= 4)		// Stick
					{
						md->srcRaw = modeFixValue( md->srcRaw-1 ) ;
					}
				}
			}


			for (uint8_t i = 0 ; i < NUM_CSW ; i += 1 )
			{
    		CSwData *cs = &g_model.customSw[i];
    		uint8_t cstate = CS_STATE(cs->func);
				uint8_t t = 0 ;
    		if(cstate == CS_VOFS)
				{
					t = 1 ;
				}
				else if(cstate == CS_VCOMP)
				{
					t = 1 ;
      		if (cs->v2)
					{
    		    if (cs->v2 <= 4)		// Stick
						{
    	    		cs->v2 = modeFixValue( cs->v2-1 ) ;
						}
					}
				}
				if ( t )
				{
      		if (cs->v1)
					{
    		    if (cs->v1 <= 4)		// Stick
						{
    	    		cs->v1 = modeFixValue( cs->v1-1 ) ;
						}
					}
				}
			}

#if defined(CPUM128) || defined(CPUM2561)
			for (uint8_t i = NUM_CSW ; i < NUM_CSW+EXTRA_CSW ; i += 1 )
			{
	    	CxSwData *cs = &g_model.xcustomSw[i-NUM_CSW];
    		uint8_t cstate = CS_STATE(cs->func);
				uint8_t t = 0 ;
    		if(cstate == CS_VOFS)
				{
					t = 1 ;
				}
				else if(cstate == CS_VCOMP)
				{
					t = 1 ;
      		if (cs->v2)
					{
    		    if (cs->v2 <= 4)		// Stick
						{
    	    		cs->v2 = modeFixValue( cs->v2-1 ) ;
						}
					}
				}
				if ( t )
				{
      		if (cs->v1)
					{
    		    if (cs->v1 <= 4)		// Stick
						{
    	    		cs->v1 = modeFixValue( cs->v1-1 ) ;
						}
					}
				}
			}
#endif	// CPUs
	    memmove( &Xmem.texpoData, &g_model.expoData, sizeof(Xmem.texpoData) ) ;
			for (uint8_t i = 0 ; i < 4 ; i += 1 )
			{
				uint8_t dest = modeFixValue( i ) - 1 ;
	    	memmove( &g_model.expoData[dest], &Xmem.texpoData[i], sizeof(Xmem.texpoData[0]) ) ;
			}

// sort expo/dr here

			alert(PSTR("CHECK MIX/DR SOURCES"));
			g_model.modelVersion = 2 ;
      eeDirty( EE_MODEL ) ;
			eeWaitComplete() ;
		}
#endif	// FIX_MODE

#ifdef VERSION3
		if ( g_model.modelVersion < 3 )
		{
			for (uint8_t i = 0 ; i < NUM_CSW ; i += 1 )
			{
    		CSwData *cs = &g_model.customSw[i];
				if ( cs->func == CS_LATCH )
				{
					cs->func = CS_GREATER ;
				}
				if ( cs->func == CS_FLIP )
				{
					cs->func = CS_LESS ;
				}
			}

#if defined(CPUM128) || defined(CPUM2561)
			for (uint8_t i = NUM_CSW ; i < NUM_CSW+EXTRA_CSW ; i += 1 )
			{
	    	CxSwData *cs = &g_model.xcustomSw[i-NUM_CSW];
				if ( cs->func == CS_LATCH )
				{
					cs->func = CS_GREATER ;
				}
				if ( cs->func == CS_FLIP )
				{
					cs->func = CS_LESS ;
				}
			}
#endif	// CPUs
			g_model.modelVersion = 3 ;
      eeDirty( EE_MODEL ) ;
			eeWaitComplete() ;
		}
#endif
  }
}

bool eeDuplicateModel(uint8_t id)
{
  uint8_t i;
  for( i=id+1; i<MAX_MODELS; i++)
  {
    if(! EFile::exists(FILE_MODEL(i))) break;
  }
  if(i==MAX_MODELS) return false; //no free space in directory left

  theFile.openRd(FILE_MODEL(id));
  theFile2.create(FILE_MODEL(i),FILE_TYP_MODEL,600);
  uint8_t buf[15];
  uint8_t l;
  while((l=theFile.read(buf,15)))
  {
    theFile2.write(buf,l);
//    if(theFile.write_errno()==ERR_TMO)
//    {
//        //wait for 10ms and try again
//        uint16_t tgtime = get_tmr10ms() + 100;
//        while (!=tgtime);
//        theFile2.write(buf,l);
//    }
    wdt_reset();
  }
  theFile2.closeTrunc();
  //todo error handling
  return true;
}

bool eeReadGeneral()
{
  return (EeFsOpen() && EeFsck() >= 0 && eeLoadGeneral()) ;
}

//void eeReadAll()
//{
//  if(!EeFsOpen()  ||
//     EeFsck() < 0 ||
//     !eeLoadGeneral()
//  )
//  {
//    alert(PSTR(STR_BAD_EEPROM), true);
//    message(PSTR(STR_EE_FORMAT));
//    EeFsFormat();
//    //alert(PSTR("format ok"));
//    generalDefault();
//    // alert(PSTR("default ok"));

//    uint16_t sz = theFile.writeRlc(FILE_GENERAL,FILE_TYP_GENERAL,(uint8_t*)&g_eeGeneral,sizeof(EEGeneral),200);
//    if(sz!=sizeof(EEGeneral)) alert(PSTR(STR_GENWR_ERROR));

//    modelDefaultWrite(0);
//    //alert(PSTR("modef ok"));
//    //alert(PSTR("modwrite ok"));

//  }
//  eeLoadModel(g_eeGeneral.currModel);
//}

void eeWriteGeneral()
{
  alert(PSTR(STR_BAD_EEPROM), true);
  message(PSTR(STR_EE_FORMAT));
  EeFsFormat();
  //alert(PSTR("format ok"));
  // alert(PSTR("default ok"));

  uint16_t sz = theFile.writeRlc(FILE_GENERAL,FILE_TYP_GENERAL,(uint8_t*)&g_eeGeneral,sizeof(EEGeneral),200);
  if(sz!=sizeof(EEGeneral)) alert(PSTR(STR_GENWR_ERROR));

  modelDefaultWrite(0);
  //alert(PSTR("modef ok"));
  //alert(PSTR("modwrite ok"));
}


static uint8_t  s_eeLongTimer ;
static uint8_t  s_eeDirtyMsk;
static uint16_t s_eeDirtyTime10ms;
void eeDirty(uint8_t msk)
{
	uint8_t lmask = msk & 7 ;
  if(!lmask) return;
  s_eeDirtyMsk      |= lmask;
  s_eeDirtyTime10ms  = get_tmr10ms() ;
	s_eeLongTimer = msk >> 4 ;
}
#define WRITE_DELAY_10MS 200

uint8_t Ee_lock = 0 ;
extern uint8_t heartbeat;


void eeWaitComplete()
{
  while(s_eeDirtyMsk)
  {
		eeCheck(true) ;
    if(heartbeat == 0x3)
    {
      wdt_reset();
      heartbeat = 0;
    }
  }
}

extern uint8_t EepromActive ;

void eeCheck(bool immediately)
{
	EepromActive = 0 ;
  uint8_t msk  = s_eeDirtyMsk;
  if(!msk) return;
	EepromActive = '1' + s_eeLongTimer ;
  if( !immediately )
	{
		if ( ( get_tmr10ms() - s_eeDirtyTime10ms) < WRITE_DELAY_10MS) return ;
		if ( s_eeLongTimer )
		{
			if ( --s_eeLongTimer )
			{
  			s_eeDirtyTime10ms  = get_tmr10ms() ;
				return ;
			}
		}
	}
	s_eeLongTimer = 0 ;
  if ( Ee_lock ) return ;
  Ee_lock = EE_LOCK ;      	// Lock eeprom writing from recursion
  if ( msk & EE_TRIM )
  {
    Ee_lock |= EE_TRIM_LOCK ;    // So the lower levels know what is happening
  }
  

  if(msk & EE_GENERAL)
	{
		EepromActive = '2' ;
		
  	s_eeDirtyMsk &= ~EE_GENERAL ;
    if(theWriteFile.writeRlc(FILE_TMP, FILE_TYP_GENERAL, (uint8_t*)&g_eeGeneral,
                        sizeof(EEGeneral),20) == sizeof(EEGeneral))
    {
      EFile::swap(FILE_GENERAL,FILE_TMP);
    }else{
      if(theWriteFile.write_errno()==ERR_TMO){
        s_eeDirtyMsk |= EE_GENERAL; //try again
        s_eeDirtyTime10ms = get_tmr10ms() - WRITE_DELAY_10MS;
    		if(heartbeat == 0x3)
    		{
    		    wdt_reset();
    		    heartbeat = 0;
    		}

      }else{
        alert(Str_EEPROM_Overflow);
      }
    }
    //first finish GENERAL, then MODEL !!avoid Toggle effect
  }
  else if(msk & EE_MODEL)
	{
		EepromActive = '3' ;
  	s_eeDirtyMsk &= ~(EE_MODEL | EE_TRIM) ;
    if(theWriteFile.writeRlc(FILE_TMP, FILE_TYP_MODEL, (uint8_t*)&g_model,
                        sizeof(g_model),20) == sizeof(g_model))
    {
      EFile::swap(FILE_MODEL(g_eeGeneral.currModel),FILE_TMP);
    }else{
      if(theWriteFile.write_errno()==ERR_TMO){
        s_eeDirtyMsk |= EE_MODEL; //try again
        if ( msk & EE_TRIM )
        {
          s_eeDirtyMsk |= EE_TRIM; //try again
        }
        s_eeDirtyTime10ms = get_tmr10ms() - WRITE_DELAY_10MS;
    		if(heartbeat == 0x3)
    		{
    		    wdt_reset();
    		    heartbeat = 0;
    		}
      }else{
        if ( ( msk & EE_TRIM ) == 0 )		// Don't stop if trim adjust
        {
          alert(Str_EEPROM_Overflow);
        }
      }
    }
  }
  Ee_lock = 0 ;				// UnLock eeprom writing


}


