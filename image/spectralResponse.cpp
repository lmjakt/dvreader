#include "spectralResponse.h"

SpectralResponse::SpectralResponse(){
  // So far no need to do anything.
}

SpectralResponse::~SpectralResponse(){
  // and no need to do much here..
}

float** SpectralResponse::bg_response(float** images, int ch_no, int width, int height){
  if(ch_no <= 0 || width <= 0 || height <= 0)
    return(0);
  float** bgr = new float*[ch_no];
  int size = width * height;
  for(int i=0; i < ch_no; ++i)
    bgr[i] = new float[size];
  
  // and then quite simply
  float psum = 0;
  for(int i=0; i < size; ++i){
    psum = 0;
    for(int j=0; j < ch_no; ++j)
      psum += images[j][i];
    if(psum){
      for(int j=0; j < ch_no; ++j)
	bgr[j][i] = images[j][i] / psum;
    }else{
      for(int j=0; j < ch_no; ++j)
	bgr[j][i] = -1;
    }
  }
  return(bgr);
}
