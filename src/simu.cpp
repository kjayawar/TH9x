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

#include "fx.h"
#include "FXExpression.h"
#include "FXPNGImage.h"
#include <unistd.h>
#include "simpgmspace.h"
#include "lcd.h"
#include "fxkeys.h"
#include "th9x.h"
#include <time.h>
#include <ctype.h>


volatile unsigned char pinb,portb,pinc,pind,pine,ping;
unsigned char dummyport;
char g_title[80];
const char *eepromFile = "eeprom.bin";

extern unsigned char displayBuf[DISPLAY_W*DISPLAY_H/8+DISPLAY_W];

void lcd_img_f(int ofs,unsigned char x,unsigned char y,int i_w,int i_h)
{
  
  prog_uchar  buf[1000];
  FILE *fp=fopen("../th9x-orig/flash.bin", "r");
  fseek(fp,ofs,SEEK_SET);
  fread(buf,1,i_h/8*i_w,fp);
  fclose(fp);
  prog_uchar  *q = buf;//+0x10e*3+0x10;
  int h=i_h;
  while(h>0){
    int w=i_w;
    unsigned char *p = &displayBuf[ y / 8 * DISPLAY_W + x ];
    while(w>0){
      *p = pgm_read_byte(q); p++; q++;
      w--;
    }
    h-=8;
    y+=8;
  }
}


FILE *fp = 0;

void eeWriteBlockCmp(const void *i_pointer_ram, void *pointer_eeprom, size_t size)
{
  if(!fp) fp = fopen(eepromFile, "r+");
  long ofs = (long) pointer_eeprom;
  const char* pointer_ram= (const char*)i_pointer_ram;
  //printf("eeWr p=%10p blk%3d ofs=%2d l=%d",pointer_ram,
  //       (int)pointer_eeprom/16,
  //       (int)pointer_eeprom%16,
  //       (int)size);
  while(size){
    if(fseek(fp, ofs , SEEK_SET)==-1) perror("error in seek");
    char buf[1];
    fread(buf, 1, 1,fp);

    if(buf[0] !=  pointer_ram[0]){
      //printf("X");
      g_tmr10ms++;
      if(fseek(fp, ofs , SEEK_SET)==-1) perror("error in seek");
      fwrite(pointer_ram, 1, 1,fp);
    }else{
      //printf(".");
    }

    size--;
    ofs++;
    (const char*)pointer_ram++;
  }
  //fclose(fp);
  //puts("");
}
void eeprom_write_blockxx (const void *pointer_ram,
                    void *pointer_eeprom,
                    size_t size)
{
  printf("eeprom_write_block p=%p ofs=%d l=%2d\n",pointer_ram,(int)pointer_eeprom,(int)size);
  //  FILE *fp=fopen(eepromFile, "r+");
  if(!fp) fp = fopen(eepromFile, "r+");
  if(fseek(fp, (long) pointer_eeprom, SEEK_SET)==-1) perror("error in seek");
  fwrite(pointer_ram, size, 1,fp);
  //fclose(fp);
}

void eeprom_read_block (void *pointer_ram,
                   const void *pointer_eeprom,
                   size_t size)
{
  //FILE *fp=fopen(eepromFile, "r");
  if(!fp) fp = fopen(eepromFile, "r+");
  if(fseek(fp, (long) pointer_eeprom, SEEK_SET)==-1) perror("error in seek");
  fread(pointer_ram, size, 1, fp);
  //fclose(fp);
}


#define W  DISPLAY_W
#define H  DISPLAY_H
#define W2 W*2
#define H2 H*2
class Th9xSim: public FXMainWindow
{
  FXDECLARE(Th9xSim)
public:
  Th9xSim(){};
  Th9xSim(FXApp* a);
  long onKeypress(FXObject*,FXSelector,void*);
  long onArrowPress(FXObject*,FXSelector,void*);
  long onChore(FXObject*,FXSelector,void*);
  long onTimeout(FXObject*,FXSelector,void*);
  void makeSnapshot(const FXDrawable* drawable);
  void doEvents();
  void refreshDiplay();
  void init2();
private:


  FX::FXuchar    buf2[W2*H2/8]; 
  FXBitmap      *bmp;
  FXBitmapFrame *bmf;
  bool          firstTime;

public:
  FXSlider      *sliders[8];
  FXKnob        *knobs[8];
  FXKnob        *knobsppm[8];
  FXArrowButton *arrow[3];
  FXArrowButton *arrow2[3];
  FXToggleButton *togButPpm;
};
// Message Map
FXDEFMAP(Th9xSim) Th9xSimMap[]={

  //________Message_Type_________ID_____________________Message_Handler_______
  FXMAPFUNC(SEL_CHORE,     1,    Th9xSim::onChore),
  FXMAPFUNC(SEL_TIMEOUT,   2,    Th9xSim::onTimeout),
  FXMAPFUNC(SEL_COMMAND,   1000,    Th9xSim::onArrowPress),
  FXMAPFUNC(SEL_KEYPRESS,  0,    Th9xSim::onKeypress),
  };

FXIMPLEMENT(Th9xSim,FXMainWindow,Th9xSimMap,ARRAYNUMBER(Th9xSimMap))


Th9xSim::Th9xSim(FXApp* a)
:FXMainWindow(a,"Th9xSim",NULL,NULL,DECOR_ALL,0,0,0,0)
{

  firstTime=true;
  for(int i=0; i<(W*H/8); i++) displayBuf[i]=0;//rand();
  for(int i=0; i<(W2*H2/8); i++) buf2[i]=0;//rand();
  bmp = new FXBitmap(a,&buf2,BITMAP_KEEP,W2,H2);

  FXHorizontalFrame *hf00=new FXHorizontalFrame(this,LAYOUT_CENTER_X);
  FXHorizontalFrame *hf01=new FXHorizontalFrame(this,LAYOUT_CENTER_X);
  FXHorizontalFrame *hf02=new FXHorizontalFrame(this,LAYOUT_CENTER_X);
  //FXHorizontalFrame *hf2=new FXHorizontalFrame(this,LAYOUT_FILL_X);
  FXHorizontalFrame *hf10=new FXHorizontalFrame(this,LAYOUT_CENTER_X);
  FXHorizontalFrame *hf11=new FXHorizontalFrame(this,LAYOUT_CENTER_X);
  FXHorizontalFrame *hf1=new FXHorizontalFrame(this,LAYOUT_FILL_X);

  //rh lv rv lh
  for(int i=0; i<4; i++){
    switch(i)
    {
#define L LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_X|LAYOUT_FIX_Y
#define X0 10
#define Y0 20
      case 0:
        sliders[i]=new FXSlider(hf1,NULL,0,L|SLIDER_HORIZONTAL,X0+0,Y0+120,100,20);
        break;
      case 1:
        sliders[i]=new FXSlider(hf1,NULL,0,L|SLIDER_VERTICAL,X0+100,Y0+20,20,100);
        break;
      case 2:
        sliders[i]=new FXSlider(hf1,NULL,0,L|SLIDER_VERTICAL,X0+120,Y0+20,20,100);
        break;
      case 3:
        sliders[i]=new FXSlider(hf1,NULL,0,L|SLIDER_HORIZONTAL,X0+140,Y0+120,100,20);
        break;
      default:;
    }
    sliders[i]->setRange(0+i*50,1023);
    sliders[i]->setTickDelta(7);
    sliders[i]->setValue(i==1 ? 200 : 512+i*25);
  }
  arrow[0]= new FXArrowButton(hf10,this,1000,ARROW_LEFT);
  arrow[1]= new FXArrowButton(hf10,this,1000,ARROW_UP);
  arrow[2]= new FXArrowButton(hf10,this,1000,ARROW_RIGHT);
  for(int i=4; i<8; i++){
    knobs[i]= new FXKnob(hf11,NULL,0,KNOB_TICKS|LAYOUT_LEFT);
    knobs[i]->setRange(0,1023);
    knobs[i]->setValue(512);
  }
  
  arrow2[0]= new FXArrowButton(hf00,this,1000,ARROW_LEFT);
  arrow2[1]= new FXArrowButton(hf00,this,1000,ARROW_UP);
  arrow2[2]= new FXArrowButton(hf00,this,1000,ARROW_RIGHT);
  togButPpm = new FXToggleButton(hf00,"on", "off", NULL, NULL, NULL, 0, TOGGLEBUTTON_NORMAL);
  for(int i=0; i<8; i++){
    knobsppm[i]= new FXKnob(i<4?hf01:hf02,NULL,0,KNOB_TICKS|LAYOUT_LEFT);
    knobsppm[i]->setRange(1000,2000);
    knobsppm[i]->setValue(1500+i*20);
  }


  bmf = new FXBitmapFrame(this,bmp,0,0,0,0,0,0,0,0,0);
  bmf->setOnColor(FXRGB(0,0,0));

  //getApp()->addChore(this,1);
  getApp()->addTimeout(this,2,100);
}
void Th9xSim::makeSnapshot(const FXDrawable* drawable)
{
     // Construct and create an FXImage object
     FXPNGImage snapshot(getApp(), NULL, 0, drawable->getWidth(), drawable->getHeight());
     snapshot.create();

     // Create a window device context and lock it onto the image
     FXDCWindow dc(&snapshot);

     // Draw from the widget to this
     dc.drawArea(drawable, 0, 0, drawable->getWidth(), drawable->getHeight(), 0, 0);

     // Release lock
     dc.end();

     // Grab pixels from server side back to client side
     snapshot.restore();

     // Save recovered pixels to a file
     FXFileStream stream;
     char buf[100];
     sprintf(buf,"PNG/snapshot%s.png",g_title);
     for(unsigned i=4; i<strlen(buf); i++)
     {
       if(!isalnum(buf[i]) && buf[i]!='.' ) buf[i]='_';
     }

     if (stream.open(buf, FXStreamSave))
     //if (stream.open("snapshot.png", FXStreamSave))
     {
         snapshot.savePixels(stream);
         stream.close();
         printf("Snapshot written: %s \n",buf);
     }
     else printf("Kann Datei %s nicht öffnen",buf);
}
void Th9xSim::doEvents()
{
  //getApp()->addChore(this,1);
  getApp()->runOneEvent(false);
}

long Th9xSim::onArrowPress(FXObject*sender,FXSelector sel,void*v)
{
  int which,val;
  if(sender==arrow[0]) { which=1; val=0;}
  if(sender==arrow[1]) { which=1; val=512;}
  if(sender==arrow[2]) { which=1; val=1023;}
  if(sender==arrow2[0]){ which=2; val=1000;}
  if(sender==arrow2[1]){ which=2; val=1500;}
  if(sender==arrow2[2]){ which=2; val=2000;}
  if(which == 1){
    for(int i=0; i<4; i++) sliders[i]->setValue(val);
    for(int i=4; i<7; i++) knobs[i]->setValue(val);
  }
  if(which == 2){
    for(int i=0; i<8; i++) knobsppm[i]->setValue(val);
  }
  return 0;
}
long Th9xSim::onKeypress(FXObject*,FXSelector,void*v)
{
  FXEvent *evt=(FXEvent*)v;
  //printf("keypress %x\n",evt->code);
  if(evt->code=='s'){
    makeSnapshot(bmf);
  }
  return 0;
}


void Th9xSim::init2()
{
  init();
  // eeReadAll();
  // checkMem();
  // checkTHR();
  // checkSwitches();
}
extern uint16_t       s_trainerLast10ms;
long Th9xSim::onTimeout(FXObject*,FXSelector,void*)
{
  if(togButPpm->getState()){
    for(int i=0; i<8; i++){
      g_ppmIns[i]=knobsppm[i]->getValue()-1500;
      if(g_ppmIns[i]<-400){
        g_trainerSlaveActiveChns = i;
        s_trainerLast10ms    = g_tmr10ms;
	break;
      }
    }
  }

  per10ms();
  getApp()->addChore(this,1);
  getApp()->addTimeout(this,2,10);
  return 0;
}
void Th9xSim::refreshDiplay()
{
  //lcd_img_f(0x7d3,0,0,0x6b,0x18);
  //lcd_img_f(,0,0,,);
  //lcd_img_f(0x008c,0,0,0x40,0x20);
  if(portb & 1<<OUT_B_LIGHT)  bmf->setOffColor(FXRGB(200,150,152));
  else                        bmf->setOffColor(FXRGB(150,200,152));

  for(int x=0;x<W;x++){
    for(int y=0;y<H;y++)
    {
      int o2 = x/4 + y*W*2*2/8;
      if( displayBuf[x+(y/8)*W] & (1<<(y%8))) {
        buf2[o2]      |=   3<<(x%4*2);
        buf2[o2+W2/8] |=   3<<(x%4*2);
      }
      else {
        buf2[o2]      &= ~(3<<(x%4*2));
        buf2[o2+W2/8] &= ~(3<<(x%4*2));
        //buf2[x2/8+y2*W2/8] &= ~(3<<(x%8));
      }
    }
  }
     
  bmp->setData (buf2,0);
  bmp->render(); 
  bmf->setBitmap( bmp );  

  if(hasFocus()) {
    static FXuint keys1[]={
      KEY_Return,    INP_B_KEY_MEN,
      KEY_Page_Up,   INP_B_KEY_MEN,
      KEY_KP_1,      INP_B_KEY_MEN,
      KEY_Page_Down, INP_B_KEY_EXT,
      KEY_BackSpace, INP_B_KEY_EXT,
      KEY_KP_0,      INP_B_KEY_EXT,
      KEY_Down,      INP_B_KEY_DWN,
      KEY_Up,        INP_B_KEY_UP,
      KEY_Right,     INP_B_KEY_RGT,
      KEY_Left,      INP_B_KEY_LFT
    };

    pinb &= ~ 0x7e;
    for(unsigned i=0; i<DIM(keys1);i+=2){
      if(getApp()->getKeyState(keys1[i]))  pinb |= (1<<keys1[i+1]);
    }

    static FXuint keys2[]={KEY_F8, KEY_F7, KEY_F4, KEY_F3, KEY_F6, KEY_F5, KEY_F1, KEY_F2  };
    pind  = 0;
    for(unsigned i=0; i<DIM(keys2);i++){
      if(getApp()->getKeyState(keys2[i])) pind |= (1<<i);
    }
    
    struct SwitchKey {
      FXuint key;
      volatile unsigned char& pin;
      unsigned char shift;
      unsigned char value;
    };
    
    static SwitchKey keys3[] = {
#if defined(JETI) || defined(FRSKY)
      { KEY_1, pinc,  INP_C_ThrCt, 0 },
      { KEY_6, pinc,  INP_C_AileDR, 0 },
#else
      { KEY_1, pine,  INP_E_ThrCt, 0 },
      { KEY_6, pine,  INP_E_AileDR, 0 },
#endif
      { KEY_2, ping,  INP_G_RuddDR, 0 },
      { KEY_3, pine,  INP_E_ElevDR, 0 },
      //KEY_4, ping,  INP_G_ID1,      0,
      //KEY_5, pine,  INP_E_ID2,      0,
      { KEY_7, pine,  INP_E_Gear, 0 },
      { KEY_8, pine,  INP_E_Trainer, 0 } };

    for(unsigned i=0; i<DIM(keys3); i++){
      bool ks = getApp()->getKeyState(keys3[i].key);
      if (ks != keys3[i].value) {
        if (ks) keys3[i].pin ^= (1<<keys3[i].shift);
        keys3[i].value = ks;
      }
    }
    
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    static FXuint id=0,k4st=0,k5st=0;
    bool ks=getApp()->getKeyState(KEY_4);
    if(ks != k4st){
      if(ks && id>0) id--;
      k4st = ks;
    }
    ks=getApp()->getKeyState(KEY_5);
    if(ks != k5st){
      if(ks && id<2) id++;
      k5st = ks;
    }
    switch(id){
      case 0: ping |=  (1<<INP_G_ID1);  pine &= ~(1<<INP_E_ID2); break;
      case 1: ping &= ~(1<<INP_G_ID1);  pine &= ~(1<<INP_E_ID2); break;
      case 2: ping &= ~(1<<INP_G_ID1);  pine |=  (1<<INP_E_ID2); break;
    }

  }


}
long Th9xSim::onChore(FXObject*,FXSelector,void*)
{
  refreshDiplay();
  perMain();
  return 0;
}

Th9xSim *th9xSim;
void doFxEvents()
{
  //puts("doFxEvents");
  th9xSim->getApp()->runOneEvent(false);
  th9xSim->refreshDiplay();
}

int main(int argc,char **argv)
{
  
  if(argc>=2){
    eepromFile = argv[1];
  }
  printf("eeprom = %s\n",eepromFile);

  pine = 0xff & ~(1<<INP_E_ID2);// & ~(1<<INP_E_ElevDR);
  ping = 0xff ^ ( 1<<INP_G_RuddDR);
  // Each FOX GUI program needs one, and only one, application object.
  // The application objects coordinates some common stuff shared between
  // all the widgets; for example, it dispatches events, keeps track of
  // all the windows, and so on.
  // We pass the "name" of the application, and its "vendor", the name
  // and vendor are used to search the registry database (which stores
  // persistent information e.g. fonts and colors).
  FXApp application("Th9xSim","thus");

  // Here we initialize the application.  We pass the command line arguments
  // because FOX may sometimes need to filter out some of the arguments.
  // This opens up the display as well, and reads the registry database
  // so that persistent settings are now available.
  application.init(argc,argv);

  // This creates the main window. We pass in the title to be displayed
  // above the window, and possibly some icons for when its iconified.
  // The decorations determine stuff like the borders, close buttons,
  // drag handles, and so on the Window Manager is supposed to give this
  // window.
  //FXMainWindow *main=new FXMainWindow(&application,"Hello",NULL,NULL,DECOR_ALL);
  th9xSim = new Th9xSim(&application);
  application.create();

  // Pretty self-explanatory:- this shows the window, and places it in the
  // middle of the screen.
  th9xSim->show(PLACEMENT_SCREEN);
  th9xSim->init2();

  return application.run();
}

uint16_t anaIn(uint8_t chan)
{
  if(chan<4)  return th9xSim->sliders[chan]->getValue();
  return th9xSim->knobs[chan]->getValue();
  //return 512 -  512*10*chan/100;
  //return (rand() & 0x1f) + 0x2f8;
}
