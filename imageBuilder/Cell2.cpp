#include "Cell2.h"
#include <iostream>
#include "../image/blob.h"

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

bool Cell2::nucleus_contains(blob_set* bs)
{
  int x, y, z;
  bs->mg_pos(x, y, z);
  return(nucleus.contains(x, y));
}

bool Cell2::addBlob(blob_set* bs)
{
  if(!contains(bs))
    return(false);
  blob_sets.insert(bs);
  return(true);
}

void Cell2::addBurstBlob(blob_set* bs)
{
  nuclear_bursts.insert(bs);
}

void Cell2::clearBlobs()
{
  blob_sets.clear();
  nuclear_bursts.clear();
}

void Cell2::clearBurstBlobs()
{
  nuclear_bursts.clear();
}

set<blob_set*> Cell2::blobs(){
  return(blob_sets);
}

set<blob_set*> Cell2::burst_blobs(){
  return(nuclear_bursts);
}

void Cell2::setNuclearSum(unsigned int wi, float n_sum)
{
  nuclear_sums[wi] = n_sum;
}

float Cell2::n_sum(unsigned int wi)
{
  if(!nuclear_sums.count(wi))
    return(-1);
  return(nuclear_sums[wi]);
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

void Cell2::writeTextSummary(ofstream& out)
{
  // Summarise some various bits and pieces
  unsigned int c_length = cell.length();
  unsigned int c_area = cell.area();
  unsigned int n_length = nucleus.length();
  unsigned int n_area = nucleus.area();
  out << cell.xmin() << "," << cell.ymin() << "\t" << c_length << "\t" << c_area
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
  // for nuclear bursts we want to give a little bit more information as the intensity is somewhat important
  out << "nuclear_bursts\n";  // one line for each burst, with information from each constituent blob
  for(std::set<blob_set*>::iterator it=nuclear_bursts.begin(); it != nuclear_bursts.end(); ++it){
    int x, y, z;
    (*it)->mg_pos(x, y, z);
    out << (*it)->correctedId();
    out << "\t" << x << "," << y << "," << z;
    std::vector<blob*> bs = (*it)->b();
    std::vector<unsigned int> ids = (*it)->ids();
    if(bs.size() != ids.size())
      std::cerr << "Cell2::writeTextSummary bs and ids are of different sizes, bs: " << bs.size() << "  ids: " << ids.size() << std::endl;
    
    for(unsigned int i=0; i < bs.size() && i < ids.size(); ++i)
      out << "\t" << ids[i] << "," << bs[i]->min << "," << bs[i]->max << "," << bs[i]->sum << "," << bs[i]->points.size();
    out << "\n";
  }
  out << "nuclear_sums\n";
  for(std::map<unsigned int, float>::iterator it=nuclear_sums.begin(); it != nuclear_sums.end(); ++it)
    out << (*it).first << "\t" << (*it).second << "\n";
  
}

bool Cell2::writePerimeters(ofstream& out)
{
  if(!out){
    cerr << "Cell2::writePerimeters out in bad state" << endl;
    return(false);
  }
  cell.write_to_file(out);
  nucleus.write_to_file(out);
  return(true);
}

bool Cell2::readPerimeters(ifstream& in)
{
  cout << "Read Perimeters" << endl;
  if(!in.good())
    return(false);
  if(!cell.read_from_file(in))
    return(false);
  if(!nucleus.read_from_file(in))
    return(false);
  return(true);
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

vector<blob_set*> Cell2::burst_blobs(set<unsigned int> blob_ids, bool use_corrected)
{
  vector<blob_set*> b;
  for(set<blob_set*>::iterator it=nuclear_bursts.begin(); it != nuclear_bursts.end(); ++it){
    unsigned int id = use_corrected ? (*it)->correctedId() : (*it)->id();
    if(blob_ids.count(id))
      b.push_back((*it));
  }
  return(b);
}
