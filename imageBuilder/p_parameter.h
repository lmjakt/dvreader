#ifndef P_PARAMETER_H
#define P_PARAMETER_H

#include <map>
#include <string>

struct p_parameter {
public:
  p_parameter(std::string func, float* img, int w, int h, int xo, int yo, std::map<std::string, float>& pars);
  p_parameter();
  ~p_parameter(){};
  bool param(std::string par, float& f);
  bool param(std::string par, int& i);
  bool hasParam(std::string par);
  
  void setImage(float* img);
  float* image();
  void setArea(p_parameter& p);

  int width, height;
  int x, y; // the offset.
  std::string function;

private:
  std::map<std::string, float> params;
  float* imageData;
};

#endif
