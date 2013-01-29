#include "ColorScale.h"
#include <math.h>
#include <iostream>

ColorScale::ColorScale()
{
}

std::map<float, QColor> ColorScale::mappedColorsF(std::set<float> levels, bool use_log)
{
  std::map<float, QColor> colors;
  if(!levels.size()){
    return(colors);
  }
  float min_value = *levels.begin();
  float max_value = *levels.rbegin();
  if(use_log && min_value <= 0){
    std::cerr << "ColorScale::mappedColorsF use_log is true, but 0 or negative values present\n"
	      << "use_log forced to false" << std::endl;
    use_log = false;
  }
  min_value = use_log ? log(min_value) : min_value;
  max_value = use_log ? log(max_value) : max_value;
  float range = max_value - min_value;
  if(!range){
    colors.insert(std::make_pair(*levels.begin(), QColor(255, 0, 0))); // do not insert min_value as this would be the 
    return(colors);
  }
  QColor c;
  for(std::set<float>::iterator it=levels.begin(); it != levels.end(); ++it){
    float v = use_log ? log(*it) : *it;
    // we want to run colors from blue (240) -> purple (300)
    // via red (300) in the opposite direction.
    // we make the maximum value 300 since we do not want to run
    // blue --> blue (360 = full circle)
    int hue = (int)(300.0 * (v - min_value) / range);
    hue = (360 + (240 - hue)) % 360;
    c.setHsv(hue, 255, 220);
    colors.insert(std::make_pair(*it, c));
    std::cout << "Inserting color level: " << *it << "  HSV: " << hue << ",255,220" << std::endl;
  }
  return(colors);
}

// return an argb array for direct conversion to an image.
// Qt says the specification is in argb, but in fact it ends
// being bgra (rgba reversed!)
unsigned char* ColorScale::arrayedColorIndexUS(unsigned short max_level)
{
  unsigned char* colors = new unsigned char[ 4 * max_level + 1 ];
  QColor c;
  for(unsigned int i=0; i <= (int)max_level; ++i){
    int hue = (int)( (300 * i) / (int)max_level );
    hue = (360 + (240 - hue)) % 360;
    c.setHsv(hue, 255, 220);
    // it should be possible to do *(int*)(colors + i * 4) = (int)c.qRgba
    // but I cannot find the clear documentation for this, and I want 
    // to get something working before checking that.
    colors[ i * 4 ] = 255;
    colors[ i * 4 + 1] = c.red();
    colors[ i * 4 + 2] = c.green();
    colors[ i * 4 + 3] = c.blue();
  }
  return(colors);
}
