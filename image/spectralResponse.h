#ifndef SPECTRALRESPONSE_H
#define SPECTRALRESPONSE_H

// Attempts to define localised bacground by considering the spectral output:
// Assumes any given pixel will be composed by background + specific signals
// where the specific signal is lacking from at least one channel.

// The idea is the background has a specific spectral output that is fairly
// consistent. Hence we can say that the background output for each channel
// should follow something
//
// ch[i] = bg * bf[i]
//
// where ch[i] is the measured output at the pixel for that channel
// and bg is the intensity of the background at that location, and
// bf[i] is the background response factor for that channel (i.e. the response
// of that channel to begin with.

// We can deduce bg and bf from the data if we assume that most of the background will
// have a similar spectral output and that the image contains much more background than
// foreground.

// If that is the case, then, we can simply calculate for all pixels the background factors
// as 
//
// bf[i] = ch[i] / sum(ch)
//

// if indeed the background is caused by something with a consistent spectral ouput then we
// should see very sharp distributions of the bf[i] for the background pixels. If there are more
// than one background components with different spectral responses, this won't work particularly
// well.

// To begin with, we'll just convert images to response factors.

// one could also attempt a two step process: 
// 1. calculate response rates for the background;
// 2. Subtract the inferred background 
// 3. Identify other peaks in the response. .. subtract these..
// .. and so on.

// Not really sure why I'm bothering to make a class, but it might be useful if it turns out
// we can actually do something clever like the above.. 



class SpectralResponse
{
 public:
  SpectralResponse();
  ~SpectralResponse();

  float** bg_response(float** images, int ch_no, int width, int height);

 private:
  
};

#endif
