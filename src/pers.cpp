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

 This file contains any upper level code for data persistency.
 The Layer below is file.cpp and then drivers.cpp

 */

#include "th9x.h"


EFile theFile;  //used for any file operation
EFile theFile2; //sometimes we need two files 

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2

void generalDefault()
{
  memset(&g_eeGeneral,0,sizeof(g_eeGeneral));
  g_eeGeneral.myVers   =  1;
  g_eeGeneral.currModel=  0;
  g_eeGeneral.contrast = 30;
  g_eeGeneral.vBatWarn = 90;
  int16_t sum=0;
  for (int i = 0; i < 4; ++i) {
    sum += g_eeGeneral.calibMid[i]  = 0x200;
    sum += g_eeGeneral.calibSpan[i] = 0x180;
  }
  g_eeGeneral.chkSum = sum;
}
void eeWriteGeneral()
{
  theFile.writeRlc(FILE_GENERAL,FILE_TYP_GENERAL,(uint8_t*)&g_eeGeneral, sizeof(EEGeneral));
}
bool eeLoadGeneral()
{
  theFile.open(FILE_GENERAL);
  theFile.readRlc((uint8_t*)&g_eeGeneral, sizeof(EEGeneral));
  uint16_t sum=0;
  for(int i=0; i<8;i++) sum+=g_eeGeneral.calibMid[i];
#ifdef SIM
  if(g_eeGeneral.myVers != 1)    printf("bad g_eeGeneral.myVers == 1\n");
  if(g_eeGeneral.chkSum != sum)  printf("bad g_eeGeneral.chkSum == sum\n");
#endif  
  return g_eeGeneral.myVers == 1 && g_eeGeneral.chkSum == sum;
}

void modelDefault(uint8_t id)
{
  memset(&g_model,0,sizeof(g_model));
  strcpy_P(g_model.name,PSTR("MODEL     "));
  g_model.stickMode=1;
  g_model.name[5]='0'+(id+1)/10;
  g_model.name[6]='0'+(id+1)%10;
  g_model.mixData[0].destCh = 1;
  g_model.mixData[0].srcRaw = 1;
  g_model.mixData[0].weight = 100;
}
void eeLoadModelName(uint8_t id,char*buf,uint8_t len)
{
  if(id<MAX_MODELS)
  {
    //eeprom_read_block(buf,(void*)modelEeOfs(id),sizeof(g_model.name));
    theFile.open(FILE_MODEL(id));
    memset(buf,' ',len);
    if(theFile.readRlc((uint8_t*)buf,sizeof(g_model.name)) == sizeof(g_model.name) )
    {
      uint16_t sz=theFile.size();
      buf+=len;
      while(sz){ --buf; *buf='0'+sz%10; sz/=10;}
    }
  }
}
void eeLoadModel(uint8_t id)
{
  if(id<MAX_MODELS)
  {
    theFile.open(FILE_MODEL(id));
    if(theFile.readRlc((uint8_t*)&g_model, sizeof(g_model)) != sizeof(g_model) )
    {
#ifdef SIM
      printf("bad model%d data using default\n",id+1);
#endif
      modelDefault(id);
    }
  }
}
void eeSaveModel(uint8_t id)
{
  if(id<MAX_MODELS)
  {
    theFile.writeRlc(FILE_MODEL(id),FILE_TYP_MODEL,(uint8_t*)&g_model, sizeof(g_model));
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

  theFile.open(FILE_MODEL(id));
  theFile2.create(FILE_MODEL(i),FILE_TYP_MODEL);
  uint8_t buf[15];
  uint8_t l;
  while((l=theFile.read(buf,15))) theFile2.write(buf,l);
  theFile2.closeTrunc();
  //todo error handling
  return true;
}
void eeReadAll()
{
  if(!EeFsOpen() || !eeLoadGeneral())
  {
#ifdef SIM
    printf("bad eeprom contents\n");
#else
    alert(PSTR("Bad EEprom Data"));
#endif
    EeFsFormat();
    generalDefault();
    eeWriteGeneral();

    modelDefault(0);
    eeSaveModel(0);
  }
  eeLoadModel(g_eeGeneral.currModel);
}


static uint8_t  s_eeDirtyMsk;
static uint16_t s_eeDirtyTime10ms;
void eeDirty(uint8_t msk)
{
  if(!msk) return;
  s_eeDirtyMsk      |= msk;
  s_eeDirtyTime10ms  = g_tmr10ms;
}
void eeCheck(bool immediately)
{
  uint8_t msk  = s_eeDirtyMsk;
  if(!msk) return;
  if( !immediately && ((g_tmr10ms - s_eeDirtyTime10ms) < 100)) return;
  s_eeDirtyMsk = 0;
  if(msk & EE_GENERAL) eeWriteGeneral();
  if(msk & EE_MODEL)   eeSaveModel(g_eeGeneral.currModel);
  beep();
}
