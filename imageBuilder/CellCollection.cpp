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
  deleteBlobs();
}

void CellCollection::addCell(Perimeter& cellP, Perimeter& nucP)
{
  Cell2 cell(cellP, nucP);
  cells.push_back(cell);
}

// memory leak if we don't delete.
void CellCollection::setBlobs(vector<blob_set> bs)
{
  clearCellBlobs();
  deleteBlobs();      // untested could cause seg fault
  // blobs.clear();
  // unallocated_blobs.clear();
  // conflicting_blobs.clear();  // this is done by deleteBlobs()
  addBlobs(bs);
}

void CellCollection::addBlobs(vector<blob_set>& bs)
{
  for(unsigned int i=0; i < bs.size(); ++i)
    addBlob(bs[i]);
}

void CellCollection::setBurstingBlobs(std::vector<blob_set> bs)
{
  clearBurstBlobs();
  for(unsigned int i=0; i < bs.size(); ++i)
    addBurstingBlob( new blob_set(bs[i]) );
}

void CellCollection::setNuclearSum(unsigned int cell_id, unsigned int wi, float sum)
{
  if(cell_id >= cells.size())
    return;
  cells[cell_id].setNuclearSum(wi, sum);
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
  clearCellBlobs();
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
    out << "Cell no: " << i + 1 << "\n";
    cells[i].writeTextSummary(out);
  }
  out.close();
  return(true);
}

bool CellCollection::writeCells(QString fname)
{
  ofstream out(fname.toAscii().constData());
  if(!out){
    cerr << "CellCollection::writeCells unable to open file "
	 << fname.toAscii().constData() << "  for writing" << endl;
    return(false);
  }
  int id = cell_file_id;
  out.write((const char*)&id, sizeof(int));
  unsigned int n = cells.size();
  out.write((const char*)&n, sizeof(unsigned int));
  for(unsigned int i=0; i < cells.size(); ++i){
    if(!cells[i].writePerimeters(out))
      return(false);
  }
  return(true);
}

bool CellCollection::readCells(QString fname)
{
  cout << "CellCollection readCells : " << endl;
  ifstream in(fname.toAscii().constData());
  if(!in.good()){
    cerr << "CellCollection::readCells unable to open file "
	 << fname.toAscii().constData() << " for reading" << endl;
    return(false);
  }
  cout << "before reading anything .. " << endl;
  cout << "in.good() is " << in.good() << endl;
  int id = 0;
  in.read((char*)&id, sizeof(int));
  cout << "read id " << id << endl;
  if(id != cell_file_id){
    cerr << "CellCollection::readCells file opened, but incorrect cell_file_id: expected "
	 << cell_file_id << "  got: " << id << endl;
    return(false);
  }
  unsigned int cell_no;
  in.read((char*)&cell_no, sizeof(unsigned int));
  cout << "read cell_no: " << cell_no << endl;
  if(!in.good()){
    cerr << "CellCollection unable to obtain number of cells from file: " << endl;
    return(false);
  }
  cout << "resizing cells to : " << cell_no << endl;
  cells.resize(cell_no);
  for(unsigned int i=0; i < cells.size(); ++i){
    cout << "Calling cells[i] to read " << i << endl;
    if(!cells[i].readPerimeters(in)){
      cerr << "Unable to read perimeter for cell " << i << " of " << cells.size() << endl;
      return(false);
    }
  }
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

void CellCollection::addBurstingBlob(blob_set* bsptr)
{
  int cell_i = -1;
  for(uint i=0; i < cells.size(); ++i){
    if( cells[i].nucleus_contains(bsptr) ){
      if(cell_i != -1){
	std::cerr << "CellCollection::addBurstingBlob blob_set assigned to different cell" << std::endl;
	return;
      }
      cell_i = (int)i;
    }
  }
  if(cell_i == -1){
    unallocated_burst_blobs.insert(bsptr);
    return;
  }
  cells[cell_i].addBurstBlob(bsptr);
  burst_blobs.insert(bsptr);
}

void CellCollection::clearCellBlobs()
{
  for(uint i=0; i < cells.size(); ++i)
    cells[i].clearBlobs();
}


void CellCollection::clearBurstBlobs()
{
  for(uint i=0; i < cells.size(); ++i)
    cells[i].clearBurstBlobs();
  set<blob_set*>::iterator it;
  for(it = burst_blobs.begin(); it != burst_blobs.end(); ++it)
    delete(*it);
  for(it = unallocated_burst_blobs.begin(); it != unallocated_burst_blobs.end();
      ++it)
    delete(*it);
  burst_blobs.clear();
  unallocated_burst_blobs.clear();
}

// burst blobs are deleted by the clearBurstBlobs function.
void CellCollection::deleteBlobs()
{
  set<blob_set*>::iterator it;
  for(it = blobs.begin(); it != blobs.end(); ++it)
    delete( *it );
  for(it = unallocated_blobs.begin(); it != unallocated_blobs.end(); ++it)
    delete( *it );
  for(it = conflicting_blobs.begin(); it != conflicting_blobs.end(); ++it)
    delete( *it );

  // for(it = burst_blobs.begin(); it != conflicting_blobs.end(); ++it)
  //   delete( *it );
  // for(it = unallocated_burst_blobs.begin(); it != conflicting_blobs.end(); ++it)
  //   delete( *it );
  
  blobs.clear();
  unallocated_blobs.clear();
  conflicting_blobs.clear();
  // burst_blobs.clear();
  // unallocated_burst_blobs.clear();
}
