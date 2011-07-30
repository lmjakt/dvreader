#ifndef VECTORADJUSTTHREAD
#define VECTORADJUSTTHREAD

#include <QThread>
#include <vector>

struct dpoint;

// note that the argument end to the constructor should be called
// group_size or some such word. It's not the last, one, but rather
// the number of points to adjust. It's used like an stl::end()
// iterator.

class VectorAdjustThread : public QThread
{
 public:
  VectorAdjustThread(std::vector<dpoint*>& points, 
		     std::vector<std::vector<float> >& distances,
		     float* dimFactors, bool linear,
		     unsigned int beg, unsigned int end,
		     bool useSlowMethod);
  ~VectorAdjustThread();
  float totalStress();

 private:
  void run();
  void runSlow();
  void runFast();

  std::vector<dpoint*>& points;
  std::vector<std::vector<float> >& distances;
  float* dimFactors;
  bool linear;
  unsigned int beg;
  unsigned int end;
  bool useSlowMethod;  // remember the coordinate vectors
  unsigned int p_size;
  
  float stress;
};

#endif
