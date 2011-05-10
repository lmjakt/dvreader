#ifndef BLOB_MT_MAPPER_COLLECTION_H
#define BLOB_MT_MAPPER_COLLECTION_H

#include <map>
#include <QString>
#include <QPointF>
#include "blob_set.h"
#include "BlobModelSet.h"
#include <vector>

// y = k + xr (dy short for dy/dx)
class Line {
  float k;
  float r;
 public:
  Line(){
    k = 0;
    r = 1.0;
  }
  Line(float y_intercept, float dy){
    k = y_intercept;
    r = dy;
  }
  
  bool friend operator <(const Line& a, const QPointF& b){
    return( (a.k + b.x() * a.r) < b.y() );
  }
  bool friend operator >(const Line& a, const QPointF& b){
    return( (a.k + b.x() * a.r) > b.y() );
  }
  bool friend operator <(const Line& a, const Line& b){
    if(a.r == b.r)
      return(a.k < b.k);
    return(a.r < b.r);
  }
  bool friend operator ==(const Line& a, const Line& b){
    return( a.k == b.k && a.r == b.r );
  }
};


// the aread between two lines
class AreaRange {    
  Line minLine;
  Line maxLine;
  QString x_par, y_par;
 public:
  AreaRange(){};
  AreaRange(QString xpar, QString ypar, float k1, float r1, float k2, float r2){
    minLine = Line(k1, r1);
    maxLine = Line(k2, r2);
    x_par = xpar;
    y_par = ypar;
  }
  bool within(QPointF p){
    return(minLine < p && maxLine > p);
  }
  QString xpar(){
    return(x_par);
  }
  QString ypar(){
    return(y_par);
  }
  void setLines(float k1, float r1, float k2, float r2){
    minLine = Line(k1, r1);
    maxLine = Line(k2, r2);
  }
};

class Range {
  float min;
  float max;
public:
  Range(){
    min = max = 0;
  }
  Range(float mn, float mx){
    min = mn; max = mx;
  }
  bool within(float f){
    return( f >= min && f <= max );
  }
  void setRange(float mn, float mx){
    min = mn; max = mx;
  }
};

// Contains the ranges for a specific class and blob
class Criteria {
  std::map<QString, Range> ranges;
  std::map<QString, AreaRange> areaRanges;
  unsigned int mapper_id;   // should be a simple power of two.
 public:
  Criteria(){
    mapper_id = 0;
  }
  Criteria(unsigned int m_id, std::map<QString, Range> rng, std::map<QString, AreaRange> a_ranges){
    ranges = rng;
    areaRanges = a_ranges;
    mapper_id = m_id;
  }
  bool within(QString& par, float f){
    if(!ranges.count(par))
      return(false);
    return( ranges[par].within(f));
  }
  bool within(QString& par, QPointF p){
    if(!areaRanges.count(par))
      return(false);
    return( areaRanges[par].within(p) );
  }
  unsigned int m_id(){
    return(mapper_id);
  }
};

// The class indicates the logical id of the blob_set, i.e.
// the identities of its members (eg. 1,2 = 3; 1,4 = 5 etc)
// Within each class the constituent blob types have separate
// Criteria for inclusion..

class ClassCriteria {
  unsigned int blob_set_id;
  std::map<unsigned int, Criteria> blob_criteria; // the key indicates the mapper id

 public:
  ClassCriteria(){
    blob_set_id = 0;
  }
  ClassCriteria(unsigned int bs_id){
    blob_set_id = bs_id;
  }
  // sets the corrected_id of the blob_set, and returns the correction code (0 for no correction, 1 for -1, etc).. 
  void insertCriteria(unsigned int m_id, Criteria criteria);
  unsigned int assess(QString& par, blob_set& bset); // returns 0 if ok, otherwise returns the failed ids..
};

// Blob_mt_mapper_collection takes ownership of the mappers.
// Note that this means that I have to use new and delete to create instances
// since I am too lazy to implement copy operators and reference counting.
class Blob_mt_mapper_collection {
  std::map<unsigned int, std::vector<blob_set> > blobs;
  std::map<unsigned int, std::vector<blob_set> > alternate_blobs;  // using the corrected blob_set_id (set by assess)
  std::map<unsigned int, ClassCriteria> class_criteria;
  std::map<unsigned int, std::vector<Blob_mt_mapper*> > mappers;  // only used to get dimensions, seems unnecessary
  std::map<unsigned int, BlobModelSet*> blobModels;

  Criteria readCriteria(QString& line, QStringList& params, unsigned int& super_id);

 public:
  Blob_mt_mapper_collection();
  ~Blob_mt_mapper_collection();

  void setMappers(std::map<unsigned int, std::vector<blob_set> >& blbs, std::map<unsigned int, std::vector<Blob_mt_mapper*> > maps);
  //bool setCriteria(std::map<unsigned int, ClassCriteria> criteria);  // Criteria must match.. 
  //bool setClassCriteria(unsigned int class_id, ClassCriteria criteria);
  //bool setCriteria(unsigned int class_id, unsigned int mapper_id, Criteria criteria);
  bool readCriteriaFromFile(QString& fileName);

  bool mapperDimensions(int& w, int& h, int& d);
  std::vector<blob_set> blobSets();
  std::vector<blob_set> blobSets(std::vector<unsigned int> superIds, bool use_corrected);
  std::vector<blob_set> blobSets(std::vector<QString> parNames, bool use_corrected);
  std::vector<blob_set> blobSets(std::vector<unsigned int> superIds, QString parName, bool use_corrected);
  std::vector<blob_set> blobSets(std::vector<unsigned int> superIds, std::vector<QString> parNames, bool use_corrected);
  // and a function that fills in the blanks in a reasonable manner.. 
  void resetModelFits();  // call before calling trainModels
  void trainModels(std::vector<QString> parNames, int xyr, int zr, bool use_corrected_ids);
  void setAlternateIds();  // this uses the results of the last instance of assess. Use with care.
};  



#endif
