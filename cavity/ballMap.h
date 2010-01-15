#ifndef BALLMAP_H
#define BALLMAP_H

#include <map>
#include "../dataStructs.h"
#include "../image/volumeMask.h"

class CavityBall;

///// WARNING:: Instances of this class must be passed around as pointers
/////           (i.e. create with new, don't declare)
////             As the appropriate copy and assign functions have not been
////             created.

class BallMap 
{
 public:
  BallMap(uint width, uint height, uint depth){
    mask = new VolumeMask((unsigned long)width, (unsigned long)height,
			  (unsigned long)depth);
  }
  ~BallMap(){
    delete mask;
  }
  bool insert(o_set o, CavityBall* cb){
    balls.insert(std::make_pair(o, cb));
    return(true);
    /* if(mask->setMask(true, o)){ */
    /*   balls.insert(std::make_pair(o, cb)); */
    /*   return(true); */
    /* } */
    /* return(false); */
  } 
  CavityBall* ball(o_set o){
    if(balls.count(o))
      return(balls[o]);
    return(0);
    /* if(!mask->mask(o)) */
    /*   return(0); */
    /* return(balls[o]); */
  }
  const std::map<o_set, CavityBall*>& allBalls(){
    return(balls);
  }
  unsigned int size(){
    return(balls.size());
  }
 private:
  std::map<o_set, CavityBall*> balls;
  VolumeMask* mask;
};
  
#endif
