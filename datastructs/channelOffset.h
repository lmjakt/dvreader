#ifndef CHANNELOFFSET_H
#define CHANNELOFFSET_H

enum OffsetDirection { 
  XPLUS, XMINUS, YPLUS, YMINUS, ZPLUS, ZMINUS, XYRESET, ZRESET
};

class ChannelOffset {
  int xo, yo, zo;
  int wl;   // the wavelength..
  
 public :
  ChannelOffset(int w, int xoff, int yoff, int zoff){
    wl = w;
    xo = xoff;
    yo = yoff;
    zo = zoff;
  }
  ChannelOffset(int w){
    wl = w;
    xo = yo = zo = 0;
  }
  ChannelOffset(){
    wl = xo = yo = zo = 0;
  }
  void changeOffset(int offsetDirection);   // use a switch and the above offset directions..
  int x(){ return(xo); }
  int y(){ return(yo); }
  int z(){ return(zo); }
  int w(){ return(wl); }
};

#endif
