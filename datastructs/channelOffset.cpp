#include "channelOffset.h"
#include <iostream>

using namespace std;

void ChannelOffset::changeOffset(int offsetDirection){
  switch(offsetDirection){
  case XPLUS :
    xo++;
    break;
  case XMINUS :
    xo--;
    //xo = xo < 0 ? 0 : xo;
    break;
  case YPLUS :
    yo++;
    break;
  case YMINUS :
    yo--;
    //yo = yo < 0 ? 0 : yo;
    break;
  case ZPLUS :
    zo++;
    break;
  case ZMINUS :
    zo--;
    //zo = zo < 0 ? 0 : zo;
    break;
  case XYRESET :
    xo = yo = 0;
    break;
  case ZRESET :
    zo = 0;
    break;
  default :
    cerr << "unknown offset id : " << endl;
  }
}
