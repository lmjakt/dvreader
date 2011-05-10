#include "CellParCollector.h"
#include "CellCollection.h"
#include "Cell2.h"
#include "../image/blob.h"
#include "blob_set.h"
#include <set>

using namespace std;

CellParCollector::CellParCollector(CellCollection* collection)
{
  cell_collection = collection;
}

CellParCollector::~CellParCollector()
{
}

cell_pars CellParCollector::params(vector<unsigned int> cell_ids, unsigned int set_id,
				   unsigned int map_id, QString xpar, QString ypar)
{
  cell_pars pars;
  Cell2 tempCell;
  vector<Cell2> cells;
  set<unsigned int> set_ids;
  set_ids.insert(set_id);

  vector< vector<float> > yv;
  vector< vector<float> > xv;
  for(unsigned int i=0; i < cell_ids.size(); ++i){
    if( cell_collection->cell(cell_ids[i], tempCell) ){
      cells.push_back(tempCell);
      vector<blob_set*> bs = tempCell.blobs(set_ids, false); // bool = use_corrected_id
      xv.push_back(getBlobPars(bs, map_id, xpar));
      yv.push_back(getBlobPars(bs, map_id, ypar));
    }
  }
  if(!cells.size())
     return(pars);
  pars.cells = cells;
  pars.x = xv;
  pars.y = yv;
  return(pars);
}

vector<float> CellParCollector::getBlobPars(vector<blob_set*>& bs, unsigned int m_id, QString pname)
{
  vector<float> v;
  for(unsigned int i=0; i < bs.size(); ++i){
    blob* b = bs[i]->blob_with_id(m_id);
    if(!b) return(v);
    v.push_back( getBlobParameter(b, pname) );  // failure gives all 0s
  }
  return(v);
}
      
