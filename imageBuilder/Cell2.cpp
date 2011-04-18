#include "Cell2.h"

using namespace std;

Cell2::Cell2()
{
}

Cell2::Cell2(Perimeter& c, Perimeter& n)
{
  cell = c;
  nucleus = n;
}

Cell2::~Cell2()
{
}

bool Cell2::contains(blob_set* bs)
{
  int x, y, z;
  bs->mg_pos(x, y, z);
  return(cell.contains(x, y));
}

bool Cell2::addBlob(blob_set* bs)
{
  if(!contains(bs))
    return(false);
  blob_sets.insert(bs);
  return(true);
}

set<blob_set*> Cell2::blobs(){
  return(blob_sets);
}

void Cell2::setCellPerimeter(Perimeter& cp)
{
  cell = cp;
}

Perimeter Cell2::cellPerimeter()
{
  return(cell);
}

Perimeter Cell2::nucleusPerimeter()
{
  return(nucleus);
}

void Cell2::writeTextSummary(ofstream& out){
  // Summarise some various bits and pieces
  int x, y;
  cell.pos(0, x, y);
  unsigned int c_length = cell.length();
  unsigned int c_area = cell.area();
  unsigned int n_length = nucleus.length();
  unsigned int n_area = nucleus.area();
  out << x << "," << y << "\t" << c_length << "\t" << c_area
      << "\t" << n_length << "\t" << n_area << "\t" << blob_sets.size() << "\n";
  map<unsigned int, int> blob_counts;
  // let's collect corrected_ids, but we better check the code to make
  // sure that this is ok.
  for(set<blob_set*>::iterator it=blob_sets.begin(); it != blob_sets.end(); ++it){
    if(!blob_counts.count((*it)->correctedId()))
      blob_counts[ (*it)->correctedId() ] = 0;
    blob_counts[ (*it)->correctedId() ]++;
  }
  for(map<unsigned int, int>::iterator it=blob_counts.begin(); it != blob_counts.end(); ++it)
    out << (*it).first << "\t" << (*it).second << "\n";
}

vector<blob_set*> Cell2::blobs(set<unsigned int> blob_ids, bool use_corrected)
{
  vector<blob_set*> b;
  for(set<blob_set*>::iterator it=blob_sets.begin(); it != blob_sets.end(); ++it){
    unsigned int id = use_corrected ? (*it)->correctedId() : (*it)->id();
    if(blob_ids.count(id))
      b.push_back((*it));
  }
  return(b);
}
