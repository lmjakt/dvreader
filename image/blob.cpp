#include "blob.h"
#include <iostream>
#include <QRegExp>
#include <QString>

using namespace std;

// void blob::flatten(){
//   flatten(this);
//   deleteChildren();
// }

// void blob::flatten(blob* parentBlob){
//     for(uint i=0; i < blobs.size(); ++i){
// 	parentBlob->points.insert(parentBlob->points.end(), blobs[i]->points.begin(), blobs[i]->points.end());
// 	parentBlob->values.insert(parentBlob->values.end(), blobs[i]->values.begin(), blobs[i]->values.end());
// 	parentBlob->surface.insert(parentBlob->surface.end(), blobs[i]->surface.begin(), blobs[i]->surface.end());
// 	blobs[i]->flatten(parentBlob);
//     }
//     // do not delete within this function as it is recursive.
//     blobs.clear();
//     blobs.resize(0);
// }

void blob::size(uint& s){
    s += points.size();
//     for(uint i=0; i < blobs.size(); ++i)
// 	blobs[i]->size(s);
    
}


// stupid ugly hack.. no way to know if not successful.. ??
float getSingleBlobParameter(blob* b, QString parname)
{
  if(parname == "volume")
    return((float)b->points.size());
  if(parname == "sum")
    return(b->sum);
  if(parname == "max")
    return(b->max);
  if(parname == "min")
    return(b->min);
  if(parname == "aux")
    return(b->aux1);
  if(parname == "mean")
    return( b->sum / (float)b->points.size());
  return(0);
}

float getBlobParameter(blob* b, QString parname)
{
  QRegExp rx("(\\w+)([\\*\\+\\-\\/])(\\w+)");
  if(rx.indexIn(parname) == -1)
    return(getSingleBlobParameter(b, parname));
  float p1 = getSingleBlobParameter(b, rx.cap(1));
  float p2 = getSingleBlobParameter(b, rx.cap(3));
  char op = rx.cap(2).toAscii()[0];
  switch(op){
  case '+':
    return(p1 + p2);
  case '-':
    return(p1 - p2);
  case '*':
    return(p1 * p2);
  case '/':
    if(p2)
      return(p1 / p2);
    return(0);
  default:
    return(0);
  }
}

// void blob::childNo(unsigned int& c){
//     c += blobs.size();
//     for(uint i=0; i < blobs.size(); ++i)
// 	blobs[i]->childNo(c);
// }
