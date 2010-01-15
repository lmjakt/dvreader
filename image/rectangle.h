#ifndef RECTANGLE_H
#define RECTANGLE_H

struct Rectangle {
  long x, y;
  long width, height;

  Rectangle(){
    x = y = width = height = 0;
  }
  Rectangle(long X, long Y, long W, long H){
    x=X; y=Y; width=W; height=H;
  }
  Rectangle(int X, int Y, int W, int H){
    x=(long)X; y=(long)Y; width=(long)W; height=(long)H;
  }
  bool contains(long xp, long yp){
    return( xp >= x && xp <= xp + width
	    &&
	    yp >= y && yp <= yp + height);
  }
};

#endif
