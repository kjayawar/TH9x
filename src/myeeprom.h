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

 */
#ifndef eeprom_h
#define eeprom_h


//eeprom data
//#define EE_VERSION 2
#define MAX_MODELS 16
#define MAX_MIXERS 25


typedef struct t_TrainerData1_r0 {
  uint8_t srcChn:3; //0-7 = ch1-8
  int8_t  swtch:5;
  int8_t  studWeight:6;
  uint8_t mode:2;   //off,add-mode,subst-mode
} __attribute__((packed)) TrainerData1_r0; //

typedef struct t_TrainerData_r0 {
  int16_t       calib[4];
  TrainerData1_r0  chanMix[4];
} __attribute__((packed)) TrainerData_r0; //

#define GENVERS0 1
typedef struct t_EEGeneral_r0 {  //<r119
  uint8_t   myVers;
  int16_t   calibMid[4];
  int16_t   calibSpan[4];
  uint16_t  chkSum;
  uint8_t   currModel; //0..15
  uint8_t   contrast;
  uint8_t   vBatWarn;
  int8_t    vBatCalib;  
  int8_t    lightSw;
  TrainerData_r0 trainer; //
  uint8_t   view;     //index of subview in main scrren
  uint8_t   warnOpts; //bitset for several warnings
  uint8_t   stickMode;   // 1
} __attribute__((packed)) EEGeneral_r0;//<r119;
#define GENVERS119   2
#define GENVERS119_3 3
typedef struct t_EEGeneral_r119 {
  uint8_t   myVers;
  int16_t   calibMid[4];
  int16_t   calibSpanNeg[4]; //ge119
  int16_t   calibSpanPos[4]; //ge119
  uint16_t  chkSum;
  uint8_t   currModel; //0..15
  uint8_t   contrast;
  uint8_t   vBatWarn;
  int8_t    vBatCalib; 
  int8_t    lightSw;
  TrainerData_r0 trainer;
  uint8_t   adcFilt:2;     // was view in earlier versions
  uint8_t   reserve:2;     // was view in earlier versions
  uint8_t   thr0pos:4;     // was view in earlier versions
#define WARN_THR (!(g_eeGeneral.warnOpts & 0x01))
#define WARN_SW  (!(g_eeGeneral.warnOpts & 0x02))
#define WARN_MEM (!(g_eeGeneral.warnOpts & 0x04))
#define BEEP_VOL ( g_eeGeneral.beepVol )
  uint8_t   warnOpts:3; //bitset for several warnings
  uint8_t   beepVol:2;  //
  uint8_t   view:3;     //index of subview in main screen
  uint8_t   stickMode;   // 1
} __attribute__((packed)) EEGeneral_r119;
#define GENVERS150   4
#define GENVERS150_5 5
#define GENVERS_TOP GENVERS150_5
typedef struct t_EEGeneral_r150 {
  uint8_t   myVers;
  int16_t   calibMid[7];             //ge150 4->7
  int16_t   calibSpanNeg[7]; //ge119 //ge150 4->7
  int16_t   calibSpanPos[7]; //ge119 //ge150 4->7
  //uint16_t  chkSum;
  uint8_t   inactivityMin;    //ge150
  uint8_t   resv;             //ge150
  uint8_t   currModel; //0..15
  uint8_t   contrast;
  uint8_t   vBatWarn;
  int8_t    vBatCalib; 
  int8_t    lightSw;
  TrainerData_r0 trainer;
  uint8_t   adcFilt:2;     // was view in earlier versions
  uint8_t   reserve:2;     // was view in earlier versions
  uint8_t   thr0pos:4;     // was view in earlier versions
#define WARN_THR (!(g_eeGeneral.warnOpts & 0x01))
#define WARN_SW  (!(g_eeGeneral.warnOpts & 0x02))
#define WARN_MEM (!(g_eeGeneral.warnOpts & 0x04))
#define BEEP_VOL ( g_eeGeneral.beepVol )
  uint8_t   warnOpts:3; //bitset for several warnings
  uint8_t   beepVol:2;  //
  uint8_t   view:3;     //index of subview in main screen
  uint8_t   stickMode;   // 1
} __attribute__((packed)) EEGeneral_r150;
#define EEGeneral_TOP EEGeneral_r150


/////////////////////////////////////////////////////////////////////////////
//eeprom modelspec
/////////////////////////////////////////////////////////////////////////////


typedef struct t_ExpoData_r84 {
  int8_t  expNorm;
  int8_t  expDr;
  int8_t  drSw;
  int8_t  expNormWeight;
  int8_t  expSwWeight;
} __attribute__((packed)) ExpoData_r84; //5*4=20

typedef struct t_ExpoData_r171 {
  int8_t  exp5:5;
  uint8_t mode3:3; //0=end 1=pos 2=neg 3=both 4=trimNeg

  int8_t  weight6:6;
  uint8_t chn:2;  //

  int8_t  drSw:5;
  uint8_t curve:3; //
} __attribute__((packed)) ExpoData_r171; //3*15=45


typedef struct t_TrimData_r0 {
  int8_t  trim;    //quadratisch
  int16_t trimDef_lt133;
} __attribute__((packed)) TrimData_r0;//<r143

typedef struct t_TrimData_r143 {
  int8_t  trim;    //quadratisch
} __attribute__((packed)) TrimData_r143;


typedef struct t_LimitData_r84 {
  int8_t  min;
  int8_t  max; 
  bool    revert:1;
  int8_t  offset:7;
} __attribute__((packed)) LimitData_r84;
typedef struct t_LimitData_r167 {
  int8_t  min:7;
  bool    scale:1;
  int8_t  max:7; 
  bool    resv:1;
  bool    revert:1;
  int8_t  offset:7;
} __attribute__((packed)) LimitData_r167;

typedef struct t_MixData_r0 {
  uint8_t destCh:4; //        1..NUM_CHNOUT,X1-X4
  uint8_t srcRaw:4; //0=off   1..8      ,X1-X4
  int8_t  weight;
  int8_t  swtch:5;
  uint8_t curve:3; //0=symmetrisch 1=no neg 2=no pos
  uint8_t speedUp:4;         // Servogeschwindigkeit aus Tabelle (10ms Cycle)
  uint8_t speedDown:4;      // 0 nichts
} __attribute__((packed)) MixData_r0;
//more data:
// destCh 12 -> 16                                     4+1?
// srcRaw 13 -> S1-4,P1-3,ful,s1-4,p1-3,max,ch1-16 32  4+1
//              ful           half
// curve  8->16 ,neg                                   3+2(1)
// sw-mode -100,0,disable                              0+2
// mix-mode + 1 * =                                    0+2
//
//  uint8_t destCh:4;
//  uint8_t curve:4; 

//  uint8_t srcRaw:5;
//  uint8_t mixmode:2; 
//  uint8_t curveNeg:1; 

//  int8_t  swtch:5;
//  int8_t  swtchmode:2;

//  int8_t  weight;

//  uint8_t speedUp:4;
//  uint8_t speedDown:4;
//


#define MDVERS84 1
typedef struct t_ModelData_r84 {
  char      name[10];             // 10 must be first for eeLoadModelName
  uint8_t   mdVers;               // 1
  uint8_t   tmrMode;              // 1
  uint16_t  tmrVal;               // 2
  uint8_t   protocol;             // 1
  char      res[3];               // 3
  LimitData_r84 limitData[NUM_CHNOUT];// 4*8
  ExpoData_r84  expoData[4];          // 5*4
  MixData_r0   mixData[MAX_MIXERS];  //0 4*25
  int8_t    curves5[2][5];        // 10
  int8_t    curves9[2][9];        // 18
  TrimData_r0  trimData[4];          // 3*4
} __attribute__((packed)) ModelData_r84; //210

#define MDVERS143 2
typedef struct t_ModelData_r143 {
  char      name[10];             // 10 must be first for eeLoadModelName
  uint8_t   mdVers;               // 1
  uint8_t   tmrMode;              // 1
  uint16_t  tmrVal;               // 2
  uint8_t   protocol;             // 1
  char      res[3];               // 3
  LimitData_r84 limitData[NUM_CHNOUT];// 4*8
  ExpoData_r84  expoData[4];          // 5*4
  MixData_r0   mixData[MAX_MIXERS];  //0 4*25
  int8_t    curves3[3][3];        // 9  new143
  int8_t    curves5[2][5];        // 10
  int8_t    curves9[2][9];        // 18
  TrimData_r143  trimData[4];    // 3*4 -> 1*4
} __attribute__((packed)) ModelData_r143; //211

#define MDVERS167 3
typedef struct t_ModelData_r167 {
  char      name[10];             // 10 must be first for eeLoadModelName
  uint8_t   mdVers;               // 1
  uint8_t   tmrMode;              // 1
  uint16_t  tmrVal;               // 2
  uint8_t   protocol;             // 1
  char      res[3];               // 3
  LimitData_r167 limitData[NUM_CHNOUT];// 4*8
  ExpoData_r84  expoData[4];          // 5*4
  MixData_r0   mixData[MAX_MIXERS];  //0 4*25
  int8_t    curves3[3][3];        // 9  new143
  int8_t    curves5[2][5];        // 10
  int8_t    curves9[2][9];        // 18
  TrimData_r143  trimData[4];    // 3*4 -> 1*4
} __attribute__((packed)) ModelData_r167; //211

#define MDVERS171 4
typedef struct t_ModelData_r171 {
  char      name[10];             // 10 must be first for eeLoadModelName
  uint8_t   mdVers;               // 1
  uint8_t   tmrMode;              // 1
  uint16_t  tmrVal;               // 2
  uint8_t   protocol;             // 1
  char      res[3];               // 3
  LimitData_r167 limitData[NUM_CHNOUT];// 4*8
  ExpoData_r171  expoTab[15];      // 5*4 -> 4*15
  MixData_r0   mixData[MAX_MIXERS];  //0 4*25
  int8_t    curves3[3][3];        // 9  new143
  int8_t    curves5[2][5];        // 10
  int8_t    curves9[2][9];        // 18
  TrimData_r143  trimData[4];    // 3*4 -> 1*4
} __attribute__((packed)) ModelData_r171; //251

#define MDVERS_TOP MDVERS171
#define ModelData_TOP ModelData_r171


extern EEGeneral_TOP g_eeGeneral;
extern ModelData_TOP g_model;












#endif
/*eof*/
