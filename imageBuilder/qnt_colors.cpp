#include "qnt_colors.h"
#include <iostream>

using namespace std;

qnt_colors::qnt_colors(){
  colors.push_back(QColor(255, 255, 255));
}

qnt_colors::qnt_colors(QColor c){
  colors.push_back(c);
}
 
qnt_colors::qnt_colors(QString pname, std::vector<QColor>& col, std::vector<float>& bks){
  par = pname;
  colors = col;
  breaks = bks;
  if(colors.size() > breaks.size())
    return;
  if(colors.size() && colors.size() < (breaks.size() + 1)){
    breaks.resize( colors.size() - 1 );
  }
  if(!colors.size()){
    breaks.resize(0);
    colors.push_back( QColor(255, 255, 255) );
  }
}

void qnt_colors::setColor(float v, float& r, float& g, float& b){
  unsigned int i=0;
  if(breaks.size()){
    while( i < breaks.size() && v > breaks[i])
      ++i;
  }
  r = float(colors[i].red()) / 255.0;
  g = float(colors[i].green()) / 255.0;
  b = float(colors[i].blue()) / 255.0;
}


void qnt_colors::setColor(float v, unsigned char& r, unsigned char& g, unsigned char& b){
  unsigned int i=0;
  if(breaks.size()){
    while( i < breaks.size() && v > breaks[i])
      ++i;
  }
  r = colors[i].red();
  g = colors[i].green();
  b = colors[i].blue();
}

