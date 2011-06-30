#include "BackgroundDrawer.h"
#include "posInfo.h"
#include "distanceMapper.h"
#include <math.h>
#include <string.h>
#include <iostream>

BackgroundDrawer::BackgroundDrawer(std::vector<dpoint*> points, PosInfo* pos)
  : points(points), pos(pos)
{
}

BackgroundDrawer::~BackgroundDrawer()
{
}

unsigned char* BackgroundDrawer::simple_gaussian(std::vector<unsigned int> dims, 
						 unsigned char* color_matrix, float var)
{
  // 1. get float from each one.
  // 2. determine scaling using global max.. 
  // 3. make a colour picture from it..
  std::vector<float*> f_images;
  std::vector<unsigned int> used_dims;
  float max_value = 0;
  for(unsigned int i; i < dims.size(); ++i){
    float* f = gaussianf(dims[i], var);
    if(f){
      f_images.push_back(f);
      used_dims.push_back(i);  // so that the colour is ok
      float mx = maxValue(f, pos->w() * pos->h());
      if(mx > max_value) max_value = mx;
    }
  }
  if(!f_images.size())
    return(0);
  unsigned char* image = new unsigned char[pos->w() * pos->h() * 4];
  memset((void*)image, 0, pos->w() * pos->h() * 4);
  for(unsigned int i=0; i < f_images.size(); ++i){
    unsigned int ci = used_dims[i];
    float alpha = color_matrix[ ci * 4 ];
    float red =  color_matrix[ ci * 4 + 1];
    float green =  color_matrix[ ci * 4 + 2];
    float blue =  color_matrix[ ci * 4 + 3];
    size_t l = pos->w() * pos->h();
    for(size_t j=0; j < l; ++j){
      float m = f_images[i][j] / max_value;
      image[ j*4 ] += (unsigned char)(blue * m);      // REVERSED ORDER !!! 
      image[ j*4+1 ] += (unsigned char)(green * m);
      image[ j*4+2 ] += (unsigned char)(red * m);
      image[ j*4+3 ] += (unsigned char)(alpha * m);
      //      std::cout << j << "  " << m  << " * " << alpha 
      //	<< " = " << m * alpha << "  and * " << red << " = " << m * red << std:: endl; 
      //std::cout << j << ": " << f_images[i][j] << "," << m << "|| ";
    }
    std::cout << "argb: " << alpha << "," << red << "," << green << "," << blue << std::endl;
    std::cout << "max value is " << max_value << std::endl;
    delete []f_images[i];
  }
  delete []color_matrix;
  return(image);
}

// uses a gaussian defined as x = exp( -d^2/-var^2 )
// for this equation using an effectual radius of 2 * var
// works reasonably well. (var should probably be termed
// something else).
float* BackgroundDrawer::gaussianf(unsigned int dim, float var)
{
  if(pos->w() <= 0 || pos->h() <= 0 || var <= 0)
    return(0);
  float* gm = new float[ pos->w() * pos->h()];
  memset((void*)gm, 0, sizeof(float) * pos->w() * pos->h());
  int radius = (int) 2 * var;
  int width = pos->w();
  int height = pos->h();
  for(unsigned int i=0; i < points.size(); ++i){
    if(points[i]->position.size() <= dim || points[i]->dimNo < 2)
      continue;
    int px = pos->x(points[i]->coordinates[0]);
    int py = pos->y(points[i]->coordinates[1]);
    
    int bx = px - radius < 0 ? 0 : px - radius;
    int by = py - radius < 0 ? 0 : py - radius;
    int ex = (px + radius) > width ? width : px + radius;
    int ey = (py + radius) > height ? height : py + radius;
    float d = 0;
    for(int x=bx; x < ex; ++x){
      for(int y=by; y < ey; ++y){
	d = sqrt( (px - x) * (px - x) + (py - y) * (py - y) );
	gm[ y * width + x ] += points[i]->position[dim] * expf(-(d*d)/(var*var));
	// std::cout << points[i]->position[dim] << " * " << expf(-(d*d)/(var*var))
	// 	  << " = "  << points[i]->position[dim] * expf(-(d*d)/(var*var))
	// 	  << "  ==>  " << gm[ y * width + x ] << std::endl;
      }
    }
  }
  return(gm);
}

float BackgroundDrawer::maxValue(float* f, size_t s)
{
  if(!s)
    return(0);
  float max = f[0];
  for(size_t i=1; i < s; ++i)
    if(max < f[i]) max = f[i];
  return(max);
}
