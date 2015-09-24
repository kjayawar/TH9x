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
#include "FXPNGIcon.h"
#include <unistd.h>
#include "simpgmspace.h"
#include "fxkeys.h"
#include "th9x.h"
#include <time.h>
#include <ctype.h>

#define W  DISPLAY_W
#define H  DISPLAY_H
#define W2 W*2
#define H2 H*2

char g_title[80];

class Th9xSim: public FXMainWindow
{
  FXDECLARE(Th9xSim)
public:
  Th9xSim(){};
  Th9xSim(FXApp* a);
  long onKeypress(FXObject*,FXSelector,void*);
  long onArrowPress(FXObject*,FXSelector,void*);
  long onTimeout(FXObject*,FXSelector,void*);
  void makeSnapshot(const FXDrawable* drawable);
  void doEvents();
  void refreshDisplay();
private:


  FX::FXuchar    buf2[W2*H2/8]; 
  FXBitmap      *bmp;
  FXBitmapFrame *bmf;
  bool          firstTime;

public:
  FXSlider      *sliders[8];
  FXKnob        *knobs[ANA_CHANS];
  FXKnob        *knobsppm[8]; //sim trainer port
  FXArrowButton *arrow[3];
  FXArrowButton *arrow2[3];
  FXButton      *but[8];
  FXButton      *trim[8];
  FXCheckButton *swtch[8];
  FXToggleButton *togButPpm;
};
// Message Map
FXDEFMAP(Th9xSim) Th9xSimMap[]={

  //________Message_Type_________ID_____________________Message_Handler_______
  FXMAPFUNC(SEL_TIMEOUT,   2,    Th9xSim::onTimeout),
  FXMAPFUNC(SEL_COMMAND,   1000,    Th9xSim::onArrowPress),
  FXMAPFUNC(SEL_KEYPRESS,  0,    Th9xSim::onKeypress),
  };

FXIMPLEMENT(Th9xSim,FXMainWindow,Th9xSimMap,ARRAYNUMBER(Th9xSimMap))



Th9xSim::Th9xSim(FXApp* a)
:FXMainWindow(a,"Th9xSim",NULL,NULL,DECOR_ALL|LAYOUT_FIX_HEIGHT,0,0,0,200)
{

  firstTime=true;
  for(int i=0; i<(W*H/8); i++) displayBuf[i]=0;//rand();
  for(int i=0; i<(W2*H2/8); i++) buf2[i]=0;//rand();
  bmp = new FXBitmap(a,&buf2,BITMAP_KEEP,W2,H2);
  FXFileStream fs;
  bool ret=fs.open("PNG/fernkompl.png",FX::FXStreamLoad);
  assert(ret);
  FXIcon  *icon_fern = new FXPNGIcon(a);
  icon_fern->loadPixels(fs);
  fs.close();

  //FXImageFrame *th=
  new FXImageFrame(this,icon_fern, FRAME_NONE,0,0,800,800);
  // FXVerticalFrame  *vf=new FXVerticalFrame(this,
  //      LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,
  //                                          250,100,200,500, 0,0,0,0, 0,0
  // );

  FXHorizontalFrame *hf0=new FXHorizontalFrame(this,LAYOUT_FIX_X|LAYOUT_FIX_Y,0,0);
  FXHorizontalFrame *hf1=new FXHorizontalFrame(this,LAYOUT_FIX_X|LAYOUT_FIX_Y,0,50);
  

  new FXLabel(hf0,"Trainer");
  togButPpm = new FXToggleButton(hf0,"on", "off", NULL, NULL, NULL, 0, TOGGLEBUTTON_NORMAL);
  arrow2[0]= new FXArrowButton(hf0,this,1000,ARROW_LEFT);
  arrow2[1]= new FXArrowButton(hf0,this,1000,ARROW_UP);
  arrow2[2]= new FXArrowButton(hf0,this,1000,ARROW_RIGHT);
  for(int i=0; i<8; i++){
    knobsppm[i]= new FXKnob(hf0,NULL,0,KNOB_TICKS|LAYOUT_LEFT);
    //    knobsppm[i]= new FXKnob(i<4?hf01:hf02,NULL,0,KNOB_TICKS|LAYOUT_LEFT);
    knobsppm[i]->setRange(1000,2000);
    knobsppm[i]->setValue(1500+i*20);
  }
#define BUT_OPT LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_X|LAYOUT_FIX_Y
#define BH   30
#define BW   40
#define BH2 ((BH)/2)
#define BW2 ((BW)/2)
  for(int i=0; i<8; i++){ switch(i) {
      case 6: but[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT, 90-BW2, 723-BH2, BW,BH); break;
      case 5: but[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,170-BW2, 723-BH2, BW,BH); break;
      case 4: but[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,130-BW2, 683-BH2, BW,BH); break;
      case 3: but[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,130-BW2, 763-BH2, BW,BH); break;
      case 1: but[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,600-BW2, 710-BH2, BW,BH); break;
      case 2: but[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,600-BW2, 768-BH2, BW,BH); break;
      default:but[i] = 0;
    }
    if(but[i]){
      but[i]->setBackColor(fxcolorfromname("gray10"));
    }
  }
#define CH   20
#define CW   20
#define CH2 ((CH)/2)
#define CW2 ((CW)/2)
  for(int i=0; i<8; i++){ switch(i) {
      case 0: swtch[i] = new FXCheckButton(this,"",NULL,0,BUT_OPT,  40-CW2, 251-CH2, CW,CH); break;
      case 1: swtch[i] = new FXCheckButton(this,"",NULL,0,BUT_OPT,  72-CW2, 240-CH2, CW,CH); break;
      case 2: swtch[i] = new FXCheckButton(this,"",NULL,0,BUT_OPT,  99-CW2, 288-CH2, CW,CH); break;
      case 3: swtch[i] = new FXCheckButton(this,"",NULL,0,BUT_OPT, 563-CW2, 285-CH2, CW,CH); break;
      case 4: swtch[i] = new FXCheckButton(this,"",NULL,0,BUT_OPT, 631-CW2, 295-CH2, CW,CH); break;
      case 5: swtch[i] = new FXCheckButton(this,"",NULL,0,BUT_OPT, 660-CW2, 254-CH2, CW,CH); break;
      case 6: swtch[i] = new FXCheckButton(this,"",NULL,0,BUT_OPT, 687-CW2, 261-CH2, CW,CH); break;
      default:swtch[i] = 0;
    }
    if(swtch[i]){
      swtch[i]->setBackColor(fxcolorfromname("gray20"));
    }
  }
#define TH   16
#define TW   16
#define TH2 ((CH)/2)
#define TW2 ((CW)/2)
  for(int i=0; i<8; i++){ switch(i) {
      case 6: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,155-TW2,584 -TH2, TW,TH); break;
      case 7: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,185-TW2,584 -TH2, TW,TH); break;
      case 2: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,289-TW2,457 -TH2, TW,TH); break;
      case 3: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,289-TW2,487 -TH2, TW,TH); break;
      case 4: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,445-TW2,457 -TH2, TW,TH); break;
      case 5: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,445-TW2,487 -TH2, TW,TH); break;
      case 1: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,535-TW2,586 -TH2, TW,TH); break;
      case 0: trim[i] = new FXButton(this,"",NULL,NULL,0,BUT_OPT,565-TW2,586 -TH2, TW,TH); break;
    }
    trim[i]->setBackColor(fxcolorfromname("gray60"));
  }




#define L LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_X|LAYOUT_FIX_Y
  for(int i=0; i<4; i++){
    switch(i)
    {
#define X0L 150
#define Y0  450
#define X0R 588
      case 3: sliders[i]=new FXSlider(this,NULL,0,L|SLIDER_HORIZONTAL,X0L-50,Y0+50,100,20);  break;
      case 1: sliders[i]=new FXSlider(this,NULL,0,L|SLIDER_VERTICAL,  X0L+50,Y0-50,20,100);  break;
      case 2: sliders[i]=new FXSlider(this,NULL,0,L|SLIDER_VERTICAL,  X0R-70,Y0-50,20,100);  break;
      case 0: sliders[i]=new FXSlider(this,NULL,0,L|SLIDER_HORIZONTAL,X0R-50,Y0+50,100,20);  break;
      default:;
    }
    sliders[i]->setRange(0+i*50,1023);
    sliders[i]->setTickDelta(7);
    sliders[i]->setValue(i==1 ? 200 : 512+i*25);
    sliders[i]->setBackColor(fxcolorfromname("black"));
    //sliders[i]->setLineColor(fxcolorfromname("white"));
  }
#define KNOB_OPT KNOB_NEEDLE|LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT
  new FXLabel(hf1,"Sticks/Potis/Vbat/Vref");
  arrow[0]= new FXArrowButton(hf1,this,1000,ARROW_LEFT);
  arrow[1]= new FXArrowButton(hf1,this,1000,ARROW_UP);
  arrow[2]= new FXArrowButton(hf1,this,1000,ARROW_RIGHT);
  for(int i=4; i<ANA_CHANS; i++){
    switch(i){
      case 4: knobs[i]= new FXKnob(this,NULL,0,KNOB_OPT,580-20,198-20,50,50,0,0,0,0); break;
      case 5: knobs[i]= new FXKnob(this,NULL,0,KNOB_OPT,140-20,195-20,50,50,0,0,0,0); break;
      case 6: knobs[i]= new FXKnob(this,NULL,0,KNOB_OPT,158-20,268-20,50,50,0,0,0,0); break;
      default:    knobs[i]= new FXKnob(hf1,NULL,0,KNOB_TICKS|LAYOUT_LEFT);
    }
    knobs[i]->setRange(0,1023);
    knobs[i]->setValue(512);
    knobs[i]->setBackColor(fxcolorfromname("black"));
    knobs[i]->setLineColor(fxcolorfromname("white"));
  }




  bmf = new FXBitmapFrame(this,bmp, LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,235,660,256,128,0,0,0,0);
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
  refreshDisplay();
  getApp()->addTimeout(this,2,10);
  return 0;
}
void Th9xSim::refreshDisplay()
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
      if( lcd_buf[x+(y/8)*W] & (1<<(y%8))) {
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

    //PORTB  7      6       5       4       3       2       1       0
    //       light  KEY_LFT KEY_RGT KEY_UP  KEY_DWN KEY_EXT KEY_MEN  PPM
    pinb &= ~ 0x7e;
    for(unsigned i=0; i<DIM(keys1);i+=2){
      FXuint bit=keys1[i+1];
      if(getApp()->getKeyState(keys1[i]))  pinb |= (1<<bit);
      if(but[bit]->getState())  pinb |= (1<<bit);
    }

    //PORTD  7      6       5       4       3       2       1       0
    //     TRM_D_DWN _UP  TRM_C_DWN _UP   TRM_B_DWN _UP   TRM_A_DWN _UP      
    static FXuint keys2[]={KEY_F8, KEY_F7, KEY_F4, KEY_F3, KEY_F6, KEY_F5, KEY_F1, KEY_F2  };
    pind  = 0;
    for(unsigned i=0; i<DIM(keys2);i++){
      if(getApp()->getKeyState(keys2[i])) pind |= (1<<i);
      if(trim[i]->getState())             pind |= (1<<i);
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

Th9xSim *th9xSim;
void doFxEvents()
{
  //puts("doFxEvents");
  th9xSim->getApp()->runOneEvent(false);
  th9xSim->refreshDisplay();
}

int main(int argc,char **argv)
{
  eepromFile = (argc>=2 ? argv[1] : "eeprom.bin");
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
#ifndef __APPLE__
  th9xSim->show(PLACEMENT_SCREEN);
#else
  th9xSim->show(); // Otherwise the main window gets centred across my two monitors, split down the middle.
#endif

  StartMainThread();

  return application.run();
}

uint16_t simADC(uint8_t chan)
{
  uint16_t ret;
  if(chan<4)  ret = th9xSim->sliders[chan]->getValue();
  else        ret = th9xSim->knobs[chan]->getValue();
  return ret + (rand()&0x3) - 1;
  //return 512 -  512*10*chan/100;
  //return (rand() & 0x1f) + 0x2f8;
}
