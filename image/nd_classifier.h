#ifndef ND_CLASSIFIER_H
#define ND_CLASSIFIER_H

#include <vector>
#include <map>

/* Usage notes:
   The classifier takes a data set desrcibing a number of individuals each represented
   by one row containing an arbitrary number of columns each containing one parameter
   describing the individual. Each row also is given a class in the classes vector. Hence
   the number of rows in the data set must be equal to the lenth of the classes vector.
   Furthermore, the col number is essentially the dimensionality of the data set.

   Classification is simply done by taking placing points in the data space and calculating
   the sum of a set of proximity variables; one for each member in the training set.

   Note that the ND_Classifier takes ownership of the data set given to it and will destroy that
   in the destructor. Hence make sure not to give the same data to more than one classifier.

*/

typedef unsigned int uint;

class ND_Classifier
{
 public:
  ND_Classifier();
  ~ND_Classifier();

  bool setTrainingSet(std::vector<int> classes, float* data, unsigned int rows, unsigned int cols, bool normalise);
  float* classify(float* data, unsigned int rows, unsigned int cols, std::vector<int>& classes, bool normalise);

 private:
  unsigned int dims;  // equivalent to rows 
  unsigned int t_size; // the size of the training set
  
  float* trainData;
  std::vector<int> t_classes;
  std::vector<int> rt_classes;  // initial classes redefined by the training set.
  std::vector<int> class_set;       // the number of unique classes, ordered.
  std::map<int, int> class_offsets; // the positions in the class_set vector.
  std::map<int, int> class_sizes;   // the number of members of each class.

  void normaliseData(float* data, unsigned int r_no, unsigned int d_no);
  void classify(float* row, float* proxData);
  void setClass();
};

#endif
