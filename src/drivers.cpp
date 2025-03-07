/*
 * Author	Thomas Husterer <thus1@t-online.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 This file contains any code to immediatley access the hardware except 
 initialisation and interrupt handling (this is in th9x.cpp).

 */


#include "th9x.h"

void memswap(void *dest, const void *src, uint8_t n)
{
  char *p=(char*) dest;
  char *q=(char*) src;
  while(n){
    char c=*p;
    *p = *q;
    *q = c;
    p++; q++; n--;
  }
}




#ifndef SIM
#include "avr/interrupt.h"

///opt/cross/avr/include/avr/eeprom.h
static inline void __attribute__ ((always_inline))
eeprom_write_byte_cmp (uint8_t dat, uint16_t pointer_eeprom)
{
  //see /home/thus/work/avr/avrsdk4/avr-libc-1.4.4/libc/misc/eeprom.S:98 143
  while(EECR & (1<<EEWE)) /* make sure EEPROM is ready */
    ;
  EEAR  = pointer_eeprom;

  EECR |= 1<<EERE;
  if(dat == EEDR) return;

  EEDR  = dat;
  uint8_t flags=SREG;
  cli();
  EECR |= 1<<EEMWE;
  EECR |= 1<<EEWE;
  SREG = flags;
}

void eeWriteBlockCmp(const void *i_pointer_ram, void *i_pointer_eeprom, size_t size)
{
  const char* pointer_ram = (const char*)i_pointer_ram;
  uint16_t    pointer_eeprom = (uint16_t)i_pointer_eeprom;
  while(size){
    eeprom_write_byte_cmp(*pointer_ram++,pointer_eeprom++);
    size--;
  }
}

#endif




static uint8_t s_evt;
void putEvent(uint8_t evt)
{
  //#ifdef SIM
  //  printf("putEvent %d %x\n",evt,evt);
  //#endif
  g_lightAct1s = g_tmr1s;
  g_actTime1s = g_tmr1s;
  s_evt = evt;
}
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
  //#define KSTATE_SHORT   96
#define KSTATE_SLOW    96
#define KSTATE_START   97
#define KSTATE_PAUSE   98
#define KSTATE_KILLED  99
  uint8_t m_vals:FILTERBITS;
  uint8_t m_dblcnt:2;
  uint8_t m_cnt;
  uint8_t m_state;
public:
  void input(bool val, EnumKeys enuk);
  bool state()       { return m_vals==FFVAL;                 }
  void slowEvents()  { m_state = KSTATE_SLOW;   m_cnt    = 0;}
  void pauseEvents() { m_state = KSTATE_PAUSE;  m_cnt    = 0;}
  void killEvents()  { m_state = KSTATE_KILLED; m_dblcnt = 0;}
  uint8_t getDbl()   { return m_dblcnt;                      }
};


Key keys[NUM_KEYS];
void Key::input(bool val, EnumKeys enuk)
{       
  //  uint8_t old=m_vals;
  m_vals <<= 1;  if(val) m_vals |= 1; //portbit einschieben
  m_cnt++;

  if(m_state && m_vals==0){  //gerade eben sprung auf 0
    if(m_state!=KSTATE_KILLED) {
      putEvent(EVT_KEY_BREAK(enuk));
      if(!( m_state == 32 && m_cnt<16)){
        m_dblcnt=0;
      }
        //      }
    }
    m_cnt   = 0;
    m_state = KSTATE_OFF;
  }
  switch(m_state){
    case KSTATE_OFF: 
      if(m_vals==FFVAL){ //gerade eben sprung auf ff
        m_state = KSTATE_START;
        if(m_cnt>20) m_dblcnt=0; //pause zu lang fuer double
        m_cnt   = 0;
      }
      break;
      //fallthrough
    case KSTATE_START: 
      putEvent(EVT_KEY_FIRST(enuk));
      m_dblcnt++;
      m_state   = 32;
      m_cnt     = 0;
      break;
    case 32: 
      //if(m_dblcnt>1) printf("double=%d\n",m_dblcnt);
      if(m_cnt == 24)        putEvent(EVT_KEY_LONG(enuk));
      //fallthrough
    case 16: 
    case 8: 
      if(m_cnt >= 64)  { //2 4 8 16 32 pulses in every 640ms
        m_state >>= 1;
        m_cnt     = 0;
      }
      //fallthrough
    case 4: 
      //case 2: 
#ifdef xSIM
        printf(".");
        fflush(stdout);
#endif
      if( (m_cnt & (m_state-1)) == 0) 
      {
#ifdef xSIM
        printf("\n");
#endif
        putEvent(EVT_KEY_REPT(enuk));
      }
      break;

    case KSTATE_SLOW:  //slow
      if(m_cnt >= 32)  {
        m_cnt   = 0;
        putEvent(EVT_KEY_REPT(enuk));
      }
      
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
  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0;
  switch(enuk){
    case SW_ElevDR : return PINE & (1<<INP_E_ElevDR);
    case SW_AileDR : return PINE & (1<<INP_E_AileDR);
    case SW_RuddDR : return PING & (1<<INP_G_RuddDR);
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0    : return !(PING & (1<<INP_G_ID1));
    case SW_ID1    : return (PING & (1<<INP_G_ID1))&& (PINE & (1<<INP_E_ID2));
    case SW_ID2    : return !(PINE & (1<<INP_E_ID2));
    case SW_Gear   : return PINE & (1<<INP_E_Gear);
    case SW_ThrCt  : return PINE & (1<<INP_E_ThrCt);
    case SW_Trainer: return PINE & (1<<INP_E_Trainer);
    default:;
  }
  return 0;
}

void slowEvents(uint8_t event)
{
  if(!event) return;
  event &= EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].slowEvents();
}
void pauseEvents(uint8_t event)
{
  if(!event) return;
  event &= EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].pauseEvents();
}
void killEventsRaw(uint8_t event)
{
  if(!event) return;
  event &= EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].killEvents();
}
void killEvents()
{
  killEventsRaw(g_event);
  g_event=0;
}

uint8_t getEventDbl(uint8_t event)
{
  assert(event);
  if(!event) return 0;
  event &= EVT_KEY_MASK;
  if(event < (int)DIM(keys))  return keys[event].getDbl();
  return 0;
}

//uint16_t g_anaIns[8];
volatile uint16_t g_tmr10ms;
volatile uint16_t g_tmr1s;
uint16_t g_lightAct1s;
uint16_t g_actTime1s;
volatile uint8_t  g_blinkTmr10ms;

volatile uint8_t  g_nextBeep;
void per10ms()
{
  g_tmr10ms++;
  g_blinkTmr10ms++;
  static int8_t s_hlp1s;
  if( --s_hlp1s < 0){
    g_tmr1s++;
    s_hlp1s=99;
  }
  static int8_t skipCnt;
  skipCnt-=2;
  if(skipCnt < 0){
    skipCnt  += 2 + g_eeGeneral.keySpeed;
    uint8_t enuk  = KEY_MENU;
    uint8_t    in = ~PINB;
    for(int i=1; i<7; i++)
    {
      //INP_B_KEY_MEN 1  .. INP_B_KEY_LFT 6
      keys[enuk].input(in & (1<<i),(EnumKeys)enuk);
      ++enuk;
    }
  }
  static  uchar_p  APM crossTrim[]={
    1<<INP_D_TRM_LH_DWN,
    1<<INP_D_TRM_LH_UP,
    1<<INP_D_TRM_LV_DWN,
    1<<INP_D_TRM_LV_UP,
    1<<INP_D_TRM_RV_DWN,
    1<<INP_D_TRM_RV_UP,
    1<<INP_D_TRM_RH_DWN,
    1<<INP_D_TRM_RH_UP
  };
  uint8_t enuk  = TRM_LH_DWN;
  uint8_t in    = ~PIND;
  for(int i=0; i<8; i++)
  {
    // INP_D_TRM_RH_UP   0 .. INP_D_TRM_LH_UP   7
    keys[enuk].input(in & pgm_read_byte(crossTrim+i),(EnumKeys)enuk);
    ++enuk;
  }
 
  static uint8_t s_beepState;
  static uint8_t s_beepCnt;
  if(s_beepCnt) s_beepCnt--;
  static uint8_p APM beepTab[]= {
    /* volumes: 
       Key        Tmr     Trim0,   Err 
       Store      Bat     TmrLong
       Max   Trim    WarnNoDup
       WarnInact
       1    2     3        4       5  */

    0,   0,    0,       0,      0, //quiet
    0,   0,    1,      30,    100, //silent
    1,   3,    1,      30,    100, //normal
    4,   4,    4,      50,    150, //for motor
  };
  switch(s_beepState){
    case 0: //wait for next job
      {
        if(g_nextBeep==0) break;
        if(g_nextBeep==6){ //double warn1
          s_beepCnt   = pgm_read_byte(beepTab+5*BEEP_VOL+3-1);
          s_beepState = 3;
        }else{
          s_beepCnt   = pgm_read_byte(beepTab+5*BEEP_VOL+g_nextBeep-1);
          s_beepState = 1;
        }
        g_nextBeep  = 0;
        if(s_beepCnt)
          PORTE |=  (1<<OUT_E_BUZZER);
        else    
          s_beepState = 0;
      }
      break;
    case 1: //beep once
    case 3: //beep twice
      //printf("."); 
      if(s_beepCnt==0){
        PORTE &= ~(1<<OUT_E_BUZZER);
        s_beepState--;
        s_beepCnt = 10; //pause for state == 2
        //printf("%d\n",g_tmr10ms); 
      }
      break;
    case 2: //pause betw beep twice
      if(s_beepCnt==0){
        PORTE |=  (1<<OUT_E_BUZZER);
        s_beepCnt   = pgm_read_byte(beepTab+5*BEEP_VOL+2);
        s_beepState--;
      }
      break;
  }

}
