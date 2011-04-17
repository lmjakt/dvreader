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
