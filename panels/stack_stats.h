#ifndef STACK_STATS_H
#define STACK_STATS_H

#include <vector>

struct stack_stats
{
  stack_stats(){
    mode = mean = median = maximum = minimum = 0;
  }
  stack_stats(unsigned short mn, unsigned short md, unsigned short min, unsigned short max, unsigned short mod,
	      std::vector<unsigned short> qnt, std::vector<float> lvls){
    mean = mn;
    median = md;
    maximum = max;
    minimum = min;
    mode = mod;
    if(qntiles.size() == levels.size()){
      qntiles = qnt;
      levels = lvls;
    }
  }
  unsigned short mean;
  unsigned short median;
  unsigned short minimum;
  unsigned short maximum;
  unsigned short mode;
  std::vector<unsigned short> qntiles;
  std::vector<float> levels;
};

#endif
