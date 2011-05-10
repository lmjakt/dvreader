#ifndef CELLPARCOLLECTOR_H
#define CELLPARCOLLECTOR_H

#include <QString>
#include <vector>

class blob_set;
class CellCollection;
class Cell2;

struct cell_pars {
  std::vector<Cell2> cells;
  std::vector< std::vector<float> > x;
  std::vector< std::vector<float> > y;
};

class CellParCollector
{
 public:
  CellParCollector(CellCollection* collection);
  ~CellParCollector();
  
  cell_pars params(std::vector<unsigned int> cell_ids, unsigned int set_id, 
		   unsigned int map_id, QString xpar, QString ypar);

 private:
  CellCollection* cell_collection;
  std::vector<float> getBlobPars(std::vector<blob_set*>& bs, 
				 unsigned int m_id, QString pname);
};

#endif
