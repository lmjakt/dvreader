#include "p_parameter.h"

using namespace std;

p_parameter::p_parameter(std::string func, float* img, int w, int h, int xo, int yo, map<string, float>& pars)
{
  imageData = img;
  width = w;
  height = h;
  x = xo; y = yo;
  params = pars;
  function = func;
}

p_parameter::p_parameter()
{
  imageData = 0;
  width = height = 0;
}

bool p_parameter::param(string par, float& f)
{
  if(!params.count(par))
    return(false);
  f = params[par];
  return(true);
}

bool p_parameter::param(string par, int& i)
{
  if(!params.count(par))
    return(false);
  i = (int)params[par];
  return(true);
}

bool p_parameter::hasParam(string par)
{
  return((bool)params.count(par));
}

void p_parameter::setImage(float* img)
{
  delete []imageData;
  imageData = img;
}

float* p_parameter::image()
{
  return(imageData);
}

void p_parameter::setArea(p_parameter& p)
{
  x = p.x;
  y = p.y;
  width = p.width;
  height = p.height;
}
