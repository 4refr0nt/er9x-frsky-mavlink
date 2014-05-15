/*
 * Author - Erez Raviv <erezraviv@gmail.com>
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
 * ============================================================
 * Templates file
 *
 * eccpm
 * crow
 * throttle cut
 * flaperon
 * elevon
 * v-tail
 * throttle hold
 * Aileron Differential
 * Spoilers
 * Snap Roll
 * ELE->Flap
 * Flap->ELE
 *
 *
 *
 * =============================================================
 * Assumptions:
 * All primary channels are per modi12x3
 * Each template added to the end of each channel
 *
 *
 *
 */

#include "er9x.h"
#include "templates.h"
#include "language.h"

#ifndef NO_TEMPLATES

const static prog_char APM string_1[] = STR_T_S_4CHAN   ;
const static prog_char APM string_2[] = STR_T_TCUT      ;
const static prog_char APM string_3[] = STR_T_STICK_TCUT;
const static prog_char APM string_4[] = STR_T_V_TAIL    ;
const static prog_char APM string_5[] = STR_T_ELEVON    ;
const static prog_char APM string_6[] = STR_T_HELI_SETUP;
const static prog_char APM string_7[] = STR_T_GYRO      ;
const static prog_char APM string_8[] = STR_T_SERVO_TEST;

const prog_char *const n_Templates[8] PROGMEM = {
    string_1,
    string_2,
    string_3,
    string_4,
    string_5,
    string_6,
    string_7,
    string_8
};

#endif

static MixData* setDest(uint8_t dch)
{
    uint8_t i = 0;
    MixData *md = &g_model.mixData[0];

    while ((md->destCh<=dch) && (md->destCh) && (i<MAX_MIXERS)) i++, md++;
    if(i==MAX_MIXERS) return &g_model.mixData[0];

    memmove(md+1, md, (MAX_MIXERS-(i+1))*sizeof(MixData) );
    memset( md, 0, sizeof(MixData) ) ;
    md->destCh = dch;
		md->weight = 100 ;
    return md ;
}

#ifdef NO_TEMPLATES
inline
#endif 
void clearMixes()
{
    memset(g_model.mixData,0,sizeof(g_model.mixData)); //clear all mixes
}

#ifndef NO_TEMPLATES
void clearCurves()
{
    memset(g_model.curves5,0,sizeof(g_model.curves5)); //clear all curves
    memset(g_model.curves9,0,sizeof(g_model.curves9)); //clear all curves
}

static void setCurve(uint8_t c, const prog_int8_t *ar )
{
	int8_t *p ;
	
	p = g_model.curves5[c] ;
  for(uint8_t i=0; i<5; i++)
	{
		*p++ = pgm_read_byte(ar++) ;
	}
}

void setSwitch(uint8_t idx, uint8_t func, int8_t v1, int8_t v2)
{
  CSwData *cs = &g_model.customSw[idx-1] ;
  cs->func = func ;
  cs->andsw = 0 ;
  cs->v1   = v1 ;
  cs->v2   = v2 ;
}

#endif

#ifndef FIX_MODE
NOINLINE uint8_t convert_mode_helper(uint8_t x)
{
    return pgm_read_byte(modn12x3 + g_eeGeneral.stickMode*4 + (x) - 1) ;
}
#endif

#ifndef NO_TEMPLATES
const prog_int8_t heli_ar1[] PROGMEM = {-100, 20, 30, 70, 90};
const prog_int8_t heli_ar2[] PROGMEM = {80, 70, 60, 70, 100};
const prog_int8_t heli_ar3[] PROGMEM = {100, 90, 80, 90, 100};
const prog_int8_t heli_ar4[] PROGMEM = {-30, -15, 0, 50, 100};
const prog_int8_t heli_ar5[] PROGMEM = {-100, -50, 0, 50, 100};
#endif


#ifdef NO_TEMPLATES
void applyTemplate()
#else
void applyTemplate(uint8_t idx)
#endif
{
    MixData *md = &g_model.mixData[0];

    //CC(STK)   -> vSTK
    //ICC(vSTK) -> STK
#define ICC(x) icc[(x)-1]
    uint8_t icc[4] = {0};
    for(uint8_t i=1; i<=4; i++) //generate inverse array
        for(uint8_t j=1; j<=4; j++) if(CC(i)==j) icc[j-1]=i;


#ifndef NO_TEMPLATES
    uint8_t j = 0;

    //Simple 4-Ch
    if(idx==j++) 
    {
#endif
        clearMixes();
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_RUD);
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_ELE);
        md=setDest(ICC(STK_THR));  md->srcRaw=CM(STK_THR);
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_AIL);

#ifndef NO_TEMPLATES
    }

    //T-Cut
    if(idx==j++)
    {
        md=setDest(ICC(STK_THR));  md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_THR;  md->mltpx=MLTPX_REP;
    }

    //sticky t-cut
    if(idx==j++)
    {
        md=setDest(ICC(STK_THR));  md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_SWC;  md->mltpx=MLTPX_REP;
        md=setDest(14);            md->srcRaw=CH(14);
        md=setDest(14);            md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_SWB;  md->mltpx=MLTPX_REP;
        md=setDest(14);            md->srcRaw=MIX_MAX;  md->swtch=DSW_THR;  md->mltpx=MLTPX_REP;

        setSwitch(0xB,CS_VNEG, CM(STK_THR), -99);
        setSwitch(0xC,CS_VPOS, CH(14), 0);
    }

    //V-Tail
    if(idx==j++) 
    {
        clearMixes();
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_RUD);
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_ELE);  md->weight=-100;
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_RUD);
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_ELE);
    }

    //Elevon\\Delta
    if(idx==j++)
    {
        clearMixes();
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_ELE);
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_AIL);
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_ELE);
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_AIL);  md->weight=-100;
    }


    //Heli Setup
    if(idx==j++)
    {
        clearMixes();  //This time we want a clean slate
        clearCurves();

        //Set up Mixes
        //3 cyclic channels
        md=setDest(1);  md->srcRaw=MIX_CYC1;
        md=setDest(2);  md->srcRaw=MIX_CYC2;
        md=setDest(3);  md->srcRaw=MIX_CYC3;

        //rudder
        md=setDest(4);  md->srcRaw=CM(STK_RUD);

        //Throttle
        md=setDest(5);  md->srcRaw=CM(STK_THR); md->swtch= DSW_ID0; md->curve=CV(1); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=CM(STK_THR); md->swtch= DSW_ID1; md->curve=CV(2); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=CM(STK_THR); md->swtch= DSW_ID2; md->curve=CV(3); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=MIX_MAX;      md->weight=-100; md->swtch= DSW_THR; md->mltpx=MLTPX_REP;

        //gyro gain
        md=setDest(6);  md->srcRaw=MIX_FULL; md->weight=30; md->swtch=-DSW_GEA;

        //collective
        md=setDest(11); md->srcRaw=CM(STK_THR);  md->weight=70; md->swtch= DSW_ID0; md->curve=CV(4); md->carryTrim=TRIM_OFF;
        md=setDest(11); md->srcRaw=CM(STK_THR);  md->weight=70; md->swtch= DSW_ID1; md->curve=CV(5); md->carryTrim=TRIM_OFF;
        md=setDest(11); md->srcRaw=CM(STK_THR);  md->weight=70; md->swtch= DSW_ID2; md->curve=CV(6); md->carryTrim=TRIM_OFF;

        g_model.swashType = SWASH_TYPE_120;
        g_model.swashCollectiveSource = CH(11);

        //Set up Curves
        setCurve(CURVE5(1),heli_ar1);
        setCurve(CURVE5(2),heli_ar2);
        setCurve(CURVE5(3),heli_ar3);
        setCurve(CURVE5(4),heli_ar4);
        setCurve(CURVE5(5),heli_ar5);
        setCurve(CURVE5(6),heli_ar5);
    }

    //Gyro Gain
    if(idx==j++)
    {
        md=setDest(6);  md->srcRaw=STK_P2; md->weight= 50; md->swtch=-DSW_GEA; md->sOffset=100;
        md=setDest(6);  md->srcRaw=STK_P2; md->weight=-50; md->swtch= DSW_GEA; md->sOffset=100;
    }

    //Servo Test
    if(idx==j++)
    {
        md=setDest(15); md->srcRaw=CH(16);   md->speedUp = 8; md->speedDown = 8;
        md=setDest(16); md->srcRaw=MIX_FULL; md->weight= 110; md->swtch=DSW_SW1;
        md=setDest(16); md->srcRaw=MIX_MAX;  md->weight=-110; md->swtch=DSW_SW2; md->mltpx=MLTPX_REP;
        md=setDest(16); md->srcRaw=MIX_MAX;  md->weight= 110; md->swtch=DSW_SW3; md->mltpx=MLTPX_REP;

        setSwitch(1,CS_LESS,CH(15),CH(16));
        setSwitch(2,CS_VPOS,CH(15),   105);
        setSwitch(3,CS_VNEG,CH(15),  -105);
    }



    STORE_MODELVARS;
    eeWaitComplete() ;

#endif

}


