#include "CellCollection.h"
#include <fstream>
#include <iostream>

using namespace std;

CellCollection::CellCollection()
{
  current_cell = -1;
}

CellCollection::~CellCollection()
{
  set<blob_set*>::iterator it;
  for(it = blobs.begin(); it != blobs.end(); ++it)
    delete( *it );
  for(it = unallocated_blobs.begin(); it != unallocated_blobs.end(); ++it)
    delete( *it );
  for(it = conflicting_blobs.begin(); it != conflicting_blobs.end(); ++it)
    delete( *it );
}

void CellCollection::addCell(Perimeter& cellP, Perimeter& nucP)
{
  Cell2 cell(cellP, nucP);
  cells.push_back(cell);
}

void CellCollection::addBlobs(vector<blob_set>& bs)
{
  for(unsigned int i=0; i < bs.size(); ++i)
    addBlob(bs[i]);
}

void CellCollection::addBlob(blob_set& bs)
{
  blob_set* bsptr = new blob_set(bs);
  addBlob(bsptr);
}

unsigned int CellCollection::cellNumber()
{
  return(cells.size());
}

int CellCollection::currentCell()
{
  return(current_cell);
}

// Note that blob_set points in cell may not remain valid
bool CellCollection::cell(unsigned int i, Cell2& cell)
{
  if(i < cells.size()){
    cell = cells[i];
    return(true);
  }
  return(false);
}

vector<blob_set> CellCollection::cellBlobs(unsigned int i)
{
  vector<blob_set> bl;
  if(i < cells.size()){
    set<blob_set*> blset = cells[i].blobs();
    for(set<blob_set*>::iterator it=blset.begin(); it !=blset.end(); ++it)
      bl.push_back(*(*it));  // dereferences and makes a copy
  }
  return(bl);
}

void CellCollection::setCurrentCell(int ccell)
{
  current_cell = (int)ccell;
}

bool CellCollection::modifyCellPerimeter(unsigned int i, Perimeter& cellP)
{
  if(i >= cells.size())
    return(false);
  cells[i].setCellPerimeter(cellP);
  return(true);
}

Perimeter CellCollection::cellPerimeter(unsigned int i)
{
  if(i < cells.size())
    return(cells[i].cellPerimeter());
  Perimeter p;
  return(p);
}

Perimeter CellCollection::nucleusPerimeter(unsigned int i)
{
  if(i < cells.size())
    return(cells[i].nucleusPerimeter());
  Perimeter p;
  return(p);
}
// this is very slow, but easy to code.
void CellCollection::reassignBlobs()
{
  set<blob_set*> allBlobs;
  allBlobs.insert(blobs.begin(), blobs.end());
  allBlobs.insert(unallocated_blobs.begin(), unallocated_blobs.end());
  allBlobs.insert(conflicting_blobs.begin(), conflicting_blobs.end());
  blobs.clear();
  unallocated_blobs.clear();
  conflicting_blobs.clear();
  for(set<blob_set*>::iterator it=allBlobs.begin(); it != allBlobs.end(); ++it)
    addBlob(*it);
}

bool CellCollection::writeTextSummary(QString fname)
{
  ofstream out(fname.toAscii().constData());
  if(!out){
    cerr << "CellCollection unable to write open file "
	 << fname.toAscii().constData() << "  for writing" << endl;
    return(false);
  }
  out << "Total " << cells.size() << "Cells\n";
  for(unsigned int i=0; i < cells.size(); ++i){
    out << i + 1 << "\t";
    cells[i].writeTextSummary(out);
  }
  out.close();
  return(true);
}

void CellCollection::addBlob(blob_set* bsptr)
{
  int cell_i = -1;
  for(uint i=0; i < cells.size(); ++i){
    if( cells[i].contains(bsptr) ){
      if(cell_i != -1){
	conflicting_blobs.insert(bsptr);
	return;
      }
      cell_i = (int)i;
    }
  }
  if(cell_i == -1){
    unallocated_blobs.insert(bsptr);
    return;
  }
  cells[cell_i].addBlob(bsptr);
  blobs.insert(bsptr);
}
