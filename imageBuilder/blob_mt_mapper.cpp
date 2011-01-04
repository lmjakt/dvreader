#include "blob_mt_mapper.h"
#include <QSemaphore>
#include <string.h>
#include "imStack.h"
#include <sstream>
#include <iostream>
#include "../image/blob.h"
#include "../image/blobModel.h"
#include "../image/two_d_background.h"

using namespace std;

Blob_mt_mapper::Blob_mt_mapper(ImStack* imStack, unsigned int mapper_id, bool free_memory)
{
  map_id = mapper_id;
  stack = imStack;
  qsem = 0;
  destroy_memory = free_memory;
  bg_xm = bg_ym = 0;
  bg_q = 0.0;
  blobModel = 0;
  stack->pos(pos.x, pos.y, pos.z);
  stack->dims(pos.w, pos.h, pos.d);
  unsigned int stack_size = stack->w() * stack->h() * stack->d();
  blobMap = new blob*[stack_size];
  mask = new bool[stack_size];
  // the blobMap and the mask get set in mapBlobs, so no need to do here.
  minEdge = 0;
  minPeak = 1.0;
  stack_channel = 0;
  stack_width = stack->w();
  stack_height = stack->h();
  stack_depth = stack->d();
  //Left = 1; Right = 2; Bottom = 4; Top = 8;

}

Blob_mt_mapper::~Blob_mt_mapper()
{
  deleteBlobs();
  delete []mask;
  delete []blobMap;
}

void Blob_mt_mapper::setBgPar(uint xm, uint ym, float q)
{
  if(xm > pos.w)
    xm = pos.w;
  if(ym > pos.h)
    ym = pos.h;
  if(q < 0 || q > 1.0)
    q = 0.1;
  bg_xm = xm;
  bg_ym = ym;
  bg_q = q;
}

void Blob_mt_mapper::setBlobModel(BlobModel* b_model)
{
  blobModel = b_model;
}

void Blob_mt_mapper::mapBlobs(unsigned int wi, float minimumEdge, float minimumPeak, QSemaphore* sem)
{
  qsem = sem;
  if(wi >= stack->ch()){
    emit error("Blob_mt_mapper wi too large");
    return;
  }
  stack_channel = wi;
  if(bg_xm && bg_ym)
    subtract_background();
  minEdge = minimumEdge;
  minPeak = minimumPeak;
  unsigned int stack_size = stack->w() * stack->h() * stack->d();
  memset((void*)blobMap, 0, sizeof(blob*) * stack_size);
  memset((void*)mask, false, stack_size);
  deleteBlobs();
  start();
}

string Blob_mt_mapper::description()
{
  ostringstream os;
  os << stack_channel << "\t" << minEdge << "\t" << minPeak << "\t";
  if(isFinished()){
    os << blobs.size();
  }else{
    os << 0;
  }
  return(os.str());
}

void Blob_mt_mapper::position(int& x, int& y, int& z)
{
  x = pos.x;
  y = pos.y;
  z = pos.z;
}

void Blob_mt_mapper::dims(unsigned int& w, unsigned int& h, unsigned int& d)
{
  w = pos.w;
  h = pos.h;
  d = pos.d;
}

vector<blob*> Blob_mt_mapper::rblobs(){
  vector<blob*> bl;
  bl.reserve(blobs.size());
  for(multimap<unsigned int, blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it)
    bl.push_back((*it).second);
  return(bl);
}

void Blob_mt_mapper::run()
{
  if(qsem)
    qsem->acquire();
  float v;
  blob* tempBlob = new blob();
  for(int z=0; z < stack_depth; ++z){
    for(int y=0; y < stack_height; ++y){
      for(int x=0; x < stack_width; ++x){
	v = stack->lv(stack_channel, x, y, z);
	if(v < minEdge || mask[ z * stack_width * stack_height + y * stack_width + x])
	  continue;
	tempBlob = initBlob(tempBlob, x, y, z, v);
      }
    }
  }
  finaliseBlobs();
  if(qsem)
    qsem->release();
  if(destroy_memory)
    freeMemory();   // deletes the stack, the map and the mask, but not the blobs.
}

void Blob_mt_mapper::subtract_background()
{
  if(!bg_xm || !bg_ym || !stack)
    return;
  Two_D_Background tdb;
  for(uint z=0; z < stack->d(); ++z){
    float* data = stack->l_image(stack_channel, z);
    if(!tdb.setBackground(bg_q, bg_xm, bg_ym, (int)stack->w(), (int)stack->h(), data)){
      cerr << "Blob_mt_mapper::subtract_background failed to set background" << endl;
      continue;
    }
    for(int y=0; y < (int)stack->h(); ++y){
      float* dest = data + y * stack->w();
      for(int x=0; x < (int)stack->w(); ++x){
	(*dest) -= tdb.bg(x,y);
	// allow negative values. we don't care.
	++dest;
      }
    }
  }
}

blob* Blob_mt_mapper::initBlob(blob* b, uint x, uint y, uint z, float v)
{
  b->points.push_back( linear(x, y, z) );
  b->surface.push_back(true);
  b->max = b->min = v;
  b->peakPos = b->points[0];
  b->min_x = b->max_x = x;
  b->min_y = b->max_y = y;
  b->min_z = b->max_z = z;
  
  blobMap[ linear(x, y, z) ] = b;
  mask[ linear(x, y, z) ] = true;

  extendBlob(x, y, z, b);
  
  if(!b->points.size())
    return(b);
  if(stack->lv(stack_channel, b->peakPos) >= minPeak){
    temp_blobs.push_back(b);
    blob* tempBlob = new blob();
    return(tempBlob);
  }
  // the blob failed it's requirements. The voxels should remain masked, but
  // the blobMap should point to 0
  for(uint i=0; i < b->points.size(); ++i)
    blobMap[ b->points[i] ] = 0;
  b->points.resize(0);
  b->surface.resize(0);
  return(b);
}

void Blob_mt_mapper::extendBlob(int x, int y, int z, blob* b)
{
  float v1 = stack->lv(stack_channel, x, y, z);
  float maxV = v1;
  int mdx, mdy, mdz;
  mdx = mdy = mdz = 0;
  int dxb = x > 0 ? -1 : 0;
  int dxe = x < (stack_width - 1) ? 1 : 0;
  int dyb = y > 0 ? -1 : 0;
  int dye = y < (stack_height -1) ? 1 : 0;
  int dzb = z > 0 ? -1 : 0;
  int dze = z < (stack_depth -1) ? 1 : 0;
  
  for(int dx=dxb; dx <= dxe; ++dx){
    for(int dy=dyb; dy <= dye; ++dy){
      for(int dz=dzb; dz <= dze; ++dz){
	if(!dx && !dy && !dz)
	  continue;
	if(stack->lv(stack_channel, x + dx, y + dy, z + dz) >= maxV){
	  maxV = stack->lv(stack_channel, x + dx, y + dy, z + dz);
	  mdx = x+dx; mdy = y + dy; mdz = z + dz;
	}
      }
    }
  }
  if( !(mdx || mdy || mdz) ){
    return;
  }
  
  off_set new_off = (mdz) * stack_width * stack_height + (mdy) * stack_width + (mdx);
  //  cout << "new off is " << new_off << "  " << mdx << "," << mdy << "," << mdz << "  : " << stack_width << "*" << stack_height << "*" << stack_depth << endl;
  if(blobMap[ new_off ] == b)
    return;
  
  blob* oldBlob = blobMap[ new_off ];
  if(!oldBlob){
    blobMap[ new_off ] = b;
    mask[ new_off ] = true;
    b->peakPos = new_off;
    b->points.push_back( new_off );
    b->surface.push_back(false);
    b->max = maxV;
    b->min_x = mdx < b->min_x ? mdx : b->min_x;
    b->max_x = mdx > b->max_x ? mdx : b->max_x;
    b->min_y = mdy < b->min_y ? mdy : b->min_y;
    b->max_y = mdy > b->max_y ? mdy : b->max_y;
    b->min_z = mdz < b->min_z ? mdz : b->min_z;
    b->max_z = mdz > b->max_z ? mdz : b->max_z;
    extendBlob(mdx, mdy, mdz, b);
    return;
  }
  mergeBlobs(b, oldBlob);
  return;
}

void Blob_mt_mapper::mergeBlobs(blob* tempBlob, blob* permBlob)
{
  //  cout << "mergeBlobs" << tempBlob->points.size() << " to " << permBlob->points.size() << endl;
  permBlob->points.reserve(permBlob->points.size() + tempBlob->points.size());
  permBlob->surface.reserve(permBlob->points.size());
  for(uint i=0; i < tempBlob->points.size(); ++i){
    permBlob->points.push_back(tempBlob->points[i]);
    permBlob->surface.push_back(tempBlob->surface[i]);
    blobMap[ tempBlob->points[i] ] = permBlob;
  }
  // in the mapping procedure, permBlob should always have the highest peak value,
  // but we might want to use this function for some other purpos
  if(permBlob->max < tempBlob->max){
    permBlob->max = tempBlob->max;
    permBlob->peakPos = tempBlob->peakPos;
  }
  permBlob->min_x = permBlob->min_x < tempBlob->min_x ? permBlob->min_x : tempBlob->min_x;
  permBlob->max_x = permBlob->max_x > tempBlob->max_x ? permBlob->max_x : tempBlob->max_x;
  permBlob->min_y = permBlob->min_y < tempBlob->min_y ? permBlob->min_y : tempBlob->min_y;
  permBlob->max_y = permBlob->max_y > tempBlob->max_y ? permBlob->max_y : tempBlob->max_y;
  permBlob->min_z = permBlob->min_z < tempBlob->min_z ? permBlob->min_z : tempBlob->min_z;
  permBlob->max_z = permBlob->max_z > tempBlob->max_z ? permBlob->max_z : tempBlob->max_z;
  
  tempBlob->points.resize(0);
  tempBlob->surface.resize(0);
}

// tight means that the blob has to be tightly covered
bool Blob_mt_mapper::isSurface(int x, int y, int z, blob* b, bool tight)
{
  // edge equals surface..
  if(x <= 0 || y <= 0 || z <= 0 ||
     x >= stack_width-1 || y >= stack_width -1 || z >= stack_depth-1)
    return(true);
  
  if(!b)
    return(false);

  int dxb = x > 0 ? -1 : 0;
  int dxe = x < (stack_width - 1) ? 1 : 0;
  int dyb = y > 0 ? -1 : 0;
  int dye = y < (stack_height -1) ? 1 : 0;
  int dzb = z > 0 ? -1 : 0;
  int dze = z < (stack_depth -1) ? 1 : 0;
  blob* nBlob;
  if(tight){
    for(int dx=dxb; dx <= dxe; ++dx){
      for(int dy=dyb; dy <= dye; ++dy){
	for(int dz=dzb; dz <= dze; ++dz){
	  if( !(dx || dy || dz) )
	    continue;
	  nBlob = blobMap[ (dz + z) * stack_width * stack_height + (dy + y) * stack_width + (dx + x) ];
	  if(nBlob != b)
	    return(true);
	}
      }
    }
    return(false);
  }
  
  for(int d=-1; d <= 1; d += 2){
    if( (d + x) >= 0 && (d + x) < stack_width){
      if(b != blobMap[(x + d) + y * stack_width + z * stack_width * stack_height])
	return(true);
    }
    if( (d + y) >= 0 && (d + y) < stack_height){
      if(b != blobMap[x +  (y + d) * stack_width + z * stack_width * stack_height])
	return(true);
    }
    if( (d + z) >= 0 && (d + z) < stack_depth){
      if(b != blobMap[x + y * stack_width + (z + d) * stack_width * stack_height])
	return(true);
    }
  }
  return(false);
}

void Blob_mt_mapper::finaliseBlobs()
{
  // default colour is white.
  int r=255; int g=255; int b=255;
  for(vector<blob*>::iterator it=temp_blobs.begin(); it != temp_blobs.end(); ++it){
    (*it)->r=r; (*it)->g=g; (*it)->b=b;
    finaliseBlob((*it));
    unsigned int npos = 0;
    if((*it)->min_x == 0)
      npos |= Left;
    if((uint)(*it)->max_x == (pos.w - 1))
      npos |= Right;
    if((*it)->min_y == 0)
      npos |= Bottom;
    if((uint)(*it)->max_y == (pos.h - 1))
      npos |= Top;
    blobs.insert( make_pair(npos, (*it)));
    if(blobModel)
      incrementBlobModel((*it));
  }
  // Resize temp_blobs to 0 so we don't have more than one reference to the blobs
  temp_blobs.resize(0);
}

// This, and the isSurface function should be rewritten to avoid needing to
// call toVol (as peakPos should point directly to the appropriate position
// in blobMap (and the neighbouring positions are +/- 1, width, width * height
// but perhaps using x, y, z is more readable.
void Blob_mt_mapper::finaliseBlob(blob* b)
{
  b->sum = 0;
  int x, y, z;
  for(uint i=0; i < b->points.size(); ++i){
    toVol(b->points[i], x, y, z);
    b->surface[i] = isSurface(x, y, z, b);
    b->sum += stack->lv(stack_channel, b->points[i]);
  }
}

void Blob_mt_mapper::incrementBlobModel(blob* b)
{
  int x, y, z;
  ////////// a smarter way to do this is to find the weighted position of the peak
  ///////// using the 27 central voxels. (Basically interpolating the peak.) Otherwise
  // there is no point in increasing the resolution of the model. 
  toVol(b->peakPos, x, y, z);
  float fx, fy, fz;
  determineBlobCenter(b, fx, fy, fz);
  cout << "Blob_mt_mapper blob peak : " << x << "," << y << "," << z << "  mean peak pos: "
       << fx << "," << fy << "," << fz << endl;
  blobModel->lockMutex();
  blobModel->setPeak(fx, fy, fz, stack->lv(stack_channel, b->peakPos));
  for(uint i=0; i < b->points.size(); ++i){
    toVol(b->points[i], x, y, z);
    blobModel->addPoint(x, y, z, stack->lv(stack_channel, b->points[i]));
  }
  blobModel->unlockMutex();
}

void Blob_mt_mapper::deleteBlobs()
{
  for(multimap<unsigned int, blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it)
    delete (*it).second;
  blobs.clear();
}

void Blob_mt_mapper::freeMemory(){
  delete []blobMap;  // does not delete the blobs.
  delete []mask;
  delete stack;
  blobMap = 0;
  mask = 0;
  stack = 0;
}

// The below uses a simple weighted mean to find the center of the blob
// however, use a maximum of +/- 2 from the peak position, if these are available
void Blob_mt_mapper::determineBlobCenter(blob* b, float& fx, float& fy, float& fz)
{
  int r = 2;
  int px, py, pz;  // the peak position
  toVol(b->peakPos, px, py, pz); 
  int xb = b->min_x <= (px - r) ? px - r : b->min_x;
  int xe = b->max_x >= (px + r) ? px + r : b->max_x;
  int yb = b->min_y <= (py - r) ? py - r : b->min_y;
  int ye = b->max_y >= (py + r) ? py + r : b->max_y;
  int zb = b->min_z <= (pz - r) ? pz - r : b->min_z;
  int ze = b->max_z >= (pz + r) ? pz + r : b->max_z;

  float x_sum = 0;
  float y_sum = 0;
  float z_sum = 0;
  float sum = 0;
  for(int x=xb; x <= xe; ++x){
    for(int y=yb; y <= ye; ++y){
      for(int z=zb; z <= ze; ++z){
	float v = stack->lv(stack_channel, x, y, z);
	x_sum += float(x) * v;
	y_sum += float(y) * v;
	z_sum += float(z) * v;
	sum += v;
      }
    }
  }
  fx = x_sum / sum;
  fy = y_sum / sum;
  fz = z_sum / sum;
}
