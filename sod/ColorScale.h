#ifndef COLORSCALE_H
#define COLORSCALE_H

#include <QColor>
#include <vector>
#include <map>
#include <set>

class ColorScale
{
 public:
  ColorScale();

  std::map<float, QColor> mappedColorsF(std::set<float> levels, bool use_log=false);
  unsigned char* arrayedColorIndexUS(unsigned short max_level); // returns argb quadruplets

};

#endif
