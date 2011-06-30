#ifndef POSINFO_H
#define POSINFO_H

// not reall position information, but more about how to
// convert different values..

class PosInfo {
  float minX, minY, maxX, maxY;
  int width, height;
  int margin;
  
  float xMult, yMult;
  
 public:
  PosInfo()
    : minX(0), minY(0), maxX(1), maxY(1), width(0), height(0), margin(0), xMult(0), yMult(0)
  {
  }
  PosInfo(int m)
    : minX(0), minY(0), maxX(1), maxY(1), width(0), height(0), margin(m), xMult(0), yMult(0)
  {
  }
  void setMargin(int m)
  {
    margin = m;
  }
  void setDims(int w, int h)
  {
    width = w;
    height = h;
    xMult = ((float)(width - 2 * margin)) / (maxX - minX);
    yMult = ((float)(height - 2 * margin)) / (maxY - minY);
  }
  void setRanges(float min_x, float max_x, float min_y, float max_y)
  {
    minX = min_x;
    maxX = max_x;
    minY = min_y;
    maxY = max_y;
    maxX = maxX <= minX ? minX + 1 : maxX;
    maxY = maxY <= minY ? minY + 1 : maxY;
    xMult = ((float)(width - 2 * margin)) / (maxX - minX);
    yMult = ((float)(height - 2 * margin)) / (maxY - minY);
  }
  int x(float xf)
  {
    return( int( (xf-minX) * xMult ) + margin);
  }
  int y(float yf)
  {
    return( int( (yf-minY) * yMult ) + margin);
  }
  float rx(int x)
  {
    return( minX + (float(x - margin) / xMult) );
  }
  float ry(int y)
  {
    return( minY + (float(y - margin) / yMult) );
  }
  int w()
  {
    return(width);
  }
  int h()
  {
    return(height);
  }
};

#endif
