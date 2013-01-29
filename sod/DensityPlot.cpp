#include "DensityPlot.h"
#include "posInfo.h"
#include <math.h>
#include <string.h>
#include <algorithm>

DensityPlot::DensityPlot(std::vector<float> x, std::vector<float> y, PosInfo* pos)
{
  pos_info = 0;
  if(x.size() && y.size() == x.size()){
    points_x = x;
    points_y = y;
  }else{
    return;
  }
  pos_info = pos;
  // x_min = *min_element(points_x.begin(), points_x.end());
  // y_min = *min_element(points_y.begin(), points_y.end());
  
  // x_max = *max_element(points_x.begin(), points_x.end());
  // y_max = *max_element(points_y.begin(), points_y.end());
}

//unsigned short* DensityPlot::densityPlot(unsigned int width, unsigned int height, int radius, unsigned short* max_value)
unsigned short* DensityPlot::densityPlot(int radius, unsigned short* max_value)
{
  if(!pos_info || !pos_info->w() || !pos_info->h())
    return(0);
  
  // if(!width || !height)
  //   return(0);
  radius = abs(radius);

  // float x_range = x_max - x_min;
  // float y_range = y_max - y_min;
  // if(!x_range || !y_range)   // we'll end up with divide by 0 errors;
  //   return(0);

  unsigned short* density = new unsigned short[ pos_info->w() * pos_info->h() ];
  memset((void*)density, 0, sizeof(unsigned short)*pos_info->w()*pos_info->h());
  //  unsigned short* density = new unsigned short[ width * height ];
  //  memset((void*)density, 0, sizeof(unsigned short)*width*height);

  // define a set of offsets.. 
  std::vector<int> x_off;
  std::vector<int> y_off;
  x_off.reserve(radius * radius);
  y_off.reserve(radius * radius);
  for(int yo=-radius; yo <= radius; ++yo){
    for(int xo=-radius; xo <= radius; ++xo){
      if( sqrt((xo*xo)+(yo*yo)) <= radius ){
	x_off.push_back(xo);
	y_off.push_back(yo);
      }
    }
  }
  
  // points_x and points_y must have the same length
  int w = pos_info->w();
  int h = pos_info->h();
  for(unsigned int i=0; i < points_x.size(); ++i){
    //int x = (int)((float)(width-1) * ((points_x[i] - x_min) / x_range));
    //int y = (int)((float)(height-1) * ((points_y[i] - y_min) / y_range));
    int x = pos_info->x(points_x[i]);
    int y = pos_info->y(points_y[i]);
    

    for(unsigned int j=0; j < x_off.size(); ++j){
      int xp = x + x_off[j];
      int yp = y + y_off[j];
      if(xp < 0 || xp >= w)
	continue;
      if(yp < 0 || yp >= h)
	continue;
      density[ yp * w + xp ]++;
    }
  }
  if(max_value){
    *max_value = density[0];
    for(unsigned int i=1; i < (w*h); ++i)
      *max_value = *max_value < density[i] ? density[i] : *max_value;
  }
  return(density);
}
    
  
