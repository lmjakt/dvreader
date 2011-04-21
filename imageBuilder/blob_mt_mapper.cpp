#include "blob_mt_mapper.h"
#include <QSemaphore>
#include <string.h>
#include "imStack.h"
#include <sstream>
#include <iostream>
#include "../image/blob.h"
#include "../image/blobModel.h"
#include "../image/two_d_background.h"
#include "../panels/fileSet.h"
#include "BlobMerger.h"

using namespace std;


// the following constructor is deprecated and should be replaced by one that takes
// a stack_info and FileSet argument, and then sets the data itself when needed..

Blob_mt_mapper::Blob_mt_mapper(stack_info s_info, FileSet* fset, unsigned int mapper_id, bool free_memory)
{
  fileSet = fset;
  pos = s_info;
  map_id = mapper_id;
  qsem = 0;
  destroy_memory = free_memory;
  bg_xm = bg_ym = 0; 
  bg_q = 0.0;
  blobModel = 0;
  stack = 0;
  blobMap = 0;
  mask = 0;
  minEdge = 0;
  minPeak = 1.0;
  stack_channel = 0;
  // stack_width, stack_height and stack_depth are still used by a couple of inline functions (linear and toVol)
  // as I suspect that might be faster. ?? 
  stack_width = pos.w; //stack->w();   // references to these should be replaced with pos.w and so on.. 
  stack_height = pos.h; //stack->h();
  stack_depth = pos.d; //stack->d();
}

Blob_mt_mapper::Blob_mt_mapper(ImStack* imStack, unsigned int mapper_id, bool free_memory)
{
  fileSet = 0;
  map_id = mapper_id;
  stack = imStack;
  qsem = 0;
  destroy_memory = free_memory;
  bg_xm = bg_ym = 0;
  bg_q = 0.0;
  blobModel = 0;
  pos = stack->info();
  //  stack->pos(pos.x, pos.y, pos.z);
  //stack->dims(pos.w, pos.h, pos.d);
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
  qsem->acquire();  // sem is released at the end of run. acquired here to limit memory usage.
  setImageStack();
  if(!stack){
    emit error("Blob_mt_mapper::mapBlobs failed to obtain a stack giving up");
    qsem->release();
    return;
  }
  if(wi >= stack->ch()){
    emit error("Blob_mt_mapper wi too large");
    qsem->release();
    return;
  }
  stack_channel = wi;
  if(bg_xm && bg_ym)
    subtract_background();
  minEdge = minimumEdge;
  minPeak = minimumPeak;
  // blobMap and mask are set up in the setBlobMap.. 
  // unsigned int stack_size = stack->w() * stack->h() * stack->d();
  // memset((void*)blobMap, 0, sizeof(blob*) * stack_size);
  // memset((void*)mask, false, stack_size);
  deleteBlobs();
  setBlobMap();
  cout << "calling start" << endl;
  start();
}

void Blob_mt_mapper::addToBlobModel(BlobModel* bmodel, vector<blob*>& b)
{
  if(!stack)
    setImageStack();
  for(unsigned int i=0; i < b.size(); ++i){
    if(b[i]->min_x >= 0 && b[i]->max_x < stack_width &&
       b[i]->min_y >= 0 && b[i]->max_y < stack_height &&
       b[i]->min_z >= 0 && b[i]->max_z < stack_depth)
      incrementBlobModel(b[i], bmodel);
  }
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

vector<float> Blob_mt_mapper::blob_model_correlations()
{
  vector<float> fake_values(blobs.size(), 0);
  if(!blobModel)
    return(fake_values);
  vector<blob*> local_blobs;
  temp_blobs.reserve(blobs.size());
  for(map<unsigned int, blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it)
    local_blobs.push_back((*it).second);
  return( blob_model_correlations(blobModel, local_blobs) );
}

// all 0 if no model specified.. 
// This function is in the mapper object rather than in the model object
// because it needs access to the stack. The stack data may be deleted,
// but there is a function to recreate it (setImageStack());
vector<float> Blob_mt_mapper::blob_model_correlations(BlobModel* bmodel, vector<blob*>& local_blobs)
{  
  vector<float> correlations(local_blobs.size(), 0);
  if(!bmodel)
    return(correlations);
  // use a resolution multiplier of 1.0 for the model.
  int s_range, z_radius;
  float* model = bmodel->model(s_range, z_radius, 1.0);
  // width of model = 1 + z_radius * 2, height = 1 + s_range;
  int m_width = 1 + z_radius * 2;
  int m_height = 1 + s_range;
  // we would like to normalise the blob model at this point. As this 
  z_normalise(model, m_width * m_height);
  
  // we need to check whether or not we have the actual stack data;
  // if we have to create it then we should also delete if afterwards..
  bool keepStack = stack;
  if(!stack){
    cout << "Calling setImageStack" << endl;
    setImageStack();
    cout << "setImageStack returned" << endl;
  }
  if(!stack){
    cerr << "Blob_mt_mapper unable to obtain image stack. let's die here" << endl;
    exit(1);
  }
  float* stack_data = stack->stack(stack_channel);
  if(!stack_data){
    cerr << "Blob_mt_mapper unable to obtain stack data. dying" << endl;
    exit(1);
  }
  int i = 0;
  float* blob_values = new float[ 2 * m_width * m_height * m_height ];  // 2 * to overcome rounding errors.. 
  cout << "Location : " << pos.x << "," << pos.y << "," << pos.z << endl;
  for(vector<blob*>::iterator it=local_blobs.begin(); it != local_blobs.end(); ++it){
    if(local_blobs.size() != correlations.size()){
      cerr << "1 exiting local " << local_blobs.size() << "\t" << correlations.size() << endl;
      exit(1);
    }
    float bx, by, bz;
    blob* b = (*it);
    determineBlobCenter(b, bx, by, bz);  // the center of the blob will be set to these values..
    // now we need to normalise the data points
    unsigned int blob_value_no = 0;
    vector<float> xy_offset;
    vector<float> z_offset;
    int x, y, z;
    if(local_blobs.size() != correlations.size()){
      cerr << "2 exiting local " << local_blobs.size() << "\t" << correlations.size() << endl;
      exit(1);
    }
    for(uint j=0; j < b->points.size(); ++j){
      toVol(b->points[j], x, y, z);
      // check to make sure that the point is within the model. If so, add it to the 
      // blob_values and increment blob_value_no.
      if( (int)roundf( e_dist(bx, by, (float)x, (float)y) ) < m_height
	  &&
	  (int)roundf( fabs((float)z - bz) ) <= z_radius ){
	blob_values[blob_value_no] = stack_data[ b->points[j] ];
	++blob_value_no;
	z_offset.push_back( (float)z - bz );
	xy_offset.push_back( e_dist(bx, by, (float)x, (float)y ));
	//	if(blob_value_no >= (m_width * m_height * m_height))
	//  cerr << "blob_value_no is getting too large, bugger : " << blob_value_no << " >= " << m_width * m_height * m_height << endl;
      }
    }
    if(local_blobs.size() != correlations.size()){
      cerr << "3 exiting local " << local_blobs.size() << "\t" << correlations.size() << endl;
      exit(1);
    }
    z_normalise(blob_values, blob_value_no);
    cout << "blob_value_no : " << blob_value_no << endl;
    for(uint j=0; j < blob_value_no; ++j){
      int mx = int(roundf(z_offset[j])) + z_radius;
      int my = int(roundf(xy_offset[j]));
      if(mx >= m_width || my >= m_height || mx < 0 || my < 0){
	cerr << "Blob_mt_mapper, model correlation mx or my to large : " << mx << ", " << my << endl;
	cout << j << " : " << bx << "," << by << "," << bz << " - " 
	     << x << "," << y << "," << z << "  ==> " << xy_offset[j] << "," << z_offset[j] << endl;
	cout << "and mx, my : " << mx << "," << my << endl;
	continue;
      }
      correlations[i] += blob_values[j] * model[ my * m_width + mx ];
    }
    correlations[i] /= (float)blob_value_no;
    b->aux1 = correlations[i];
    ++i;
  }
  // delete lots of things.. 
  delete []model;
  delete []blob_values;
  if(!keepStack)
    freeMemory();
    //    delete stack;
  cout << "correlations size : " << correlations.size() << endl;
  return correlations;
}

vector<blob_set> Blob_mt_mapper::blob_sets(std::vector<Blob_mt_mapper*> mappers)
{
  vector<blob_set> bsets;
  // first check to make sure that the mappers have unique mapper ids, and that all are
  // 2 ^ n (how to check that?)
  map<unsigned int, Blob_mt_mapper*> mapKey;
  unsigned int key_sum = 0;
  for(unsigned int i=0; i < mappers.size(); ++i){
    if(mappers[i]->pos != pos){
      cerr << "Blob_mt_mapper::blob_sets mappers cover different regions" << endl;
      return(bsets);
    }
    if(!isPowerOfTwo(mappers[i]->map_id)){
      cerr << "Mapper in position " << i << " has an illegal map_id " << mappers[i]->map_id << endl;
      return(bsets);
    }
    if(mapKey.count(mappers[i]->map_id)){
      cerr << "Mapper in position " << i << " has duplicate map_id " << endl;
      return(bsets);
    }
    mapKey[ mappers[i]->map_id ] = mappers[i];
    key_sum += mappers[i]->map_id;
  }
  
  BlobMerger blobMerger;
  return( blobMerger.mergeBlobs(mappers, 1) );

  //////// All the below code is implicitly removed, by the above return statement.. ///// 
  ////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////

  unsigned int stack_size = pos.w * pos.h * pos.d;
  unsigned int* id_map = new unsigned int[stack_size];
  unsigned int* peak_id_map = new unsigned int[stack_size];
  memset((void*)id_map, 0, sizeof(unsigned int) * stack_size);
  memset((void*)peak_id_map, 0, sizeof(unsigned int) * stack_size);
  // then simply assign the overlapping ids onto this map usig binary or \=
  for(vector<Blob_mt_mapper*>::iterator mit=mappers.begin(); mit != mappers.end(); ++mit){
    unsigned int m_id = (*mit)->map_id;
    for(multimap<unsigned int, blob*>::iterator bit=(*mit)->blobs.begin(); bit != (*mit)->blobs.end(); ++bit){
      peak_id_map[ (*bit).second->peakPos ] |= m_id;
      for(vector<off_set>::iterator pit = (*bit).second->points.begin(); pit != (*bit).second->points.end(); ++pit){
	id_map[ *pit ] |= m_id;
      }
    }
  }
  // Then all we need to do is some sort of magic to deconvolve the resulting mixtures.. 
  // I can't think of an efficient way of doing this, 
  map<unsigned int, vector<unsigned int> > classes;
  for(unsigned int i=0; i < stack_size; ++i){
    if(peak_id_map[i])
      classes[ id_map[i] ].push_back(i);
  }
  // then reverse iterate across such that we take the highest (containing many points)
  set<blob*> mappedBlobs;  // use to make sure we don't double count anything.
  for(map<unsigned int, vector<unsigned int> >::reverse_iterator rit=classes.rbegin();
      rit != classes.rend(); ++rit){
    for(vector<unsigned int>::iterator it=(*rit).second.begin(); it != (*rit).second.end(); ++it){
      vector<Blob_mt_mapper*> maps = unmix_id((*rit).first, mapKey);
      if(maps.size()){
	bsets.push_back(blob_set(pos.x, pos.y, pos.z));
      }else{
	cerr << "Blob_mt_mapper::blob_sets unmix_id " << (*rit).first << "  didn't obtain any mappers" << endl;
	exit(1);
      }
      for(unsigned int i=0; i < maps.size(); ++i){
	blob* b = maps[i]->blob_at((*it));
	if(!b){
	  cerr << "Blob_mt_mapper::blob_sets didn't obtain blob from " << maps[i] << endl;
	  exit(1);
	}
	bsets.back().push(b, maps[i]->map_id, maps[i]);
      }
    }
  }
  // let's call free memory on all of the mappers used..
  for(unsigned int i=0; i < mappers.size(); ++i)
    mappers[i]->freeMemory();
  delete []id_map;
  delete []peak_id_map;
  return(bsets);
}

/// setImageStack must not be called from within run as the fileSet structure that it depends on
/// is not thread safe
// note that it does not reset blobMap or mask. Not sure if this is good or bad.
void Blob_mt_mapper::setImageStack()
{
  if(stack)
    return;
  stack = fileSet->imageStack(pos, true);
  if(!stack){
    cerr << "Blob_mt_mapper unable to obtain an imageStack. Bugger. " << endl;
    return;
  }
  // unsigned int stack_size = stack->w() * stack->h() * stack->d();
  // if(!blobMap)
  //   blobMap = new blob*[stack_size];
  // if(!mask)
  //   mask = new bool[stack_size];
  // memset((void*)blobMap, 0, sizeof(blob*) * stack_size);
  // memset((void*)mask, 0, sizeof(bool) * stack_size);
}

void Blob_mt_mapper::freeImageStack()
{
  if(!stack)
    return;
  delete stack;
  stack = 0;
}

void Blob_mt_mapper::setBlobMap()
{
  if(blobMap)
    delete []blobMap;
  if(mask)
    delete []mask;
  blobMap = 0;
  mask = 0;
  unsigned int stack_size = pos.d * pos.h * pos.w;
  if(!stack_size)
    return;
  blobMap = new blob*[stack_size];
  mask = new bool[stack_size];
  memset((void*)blobMap, 0, sizeof(blob*) * stack_size);
  memset((void*)mask, 0, sizeof(bool) * stack_size);
  
  for(map<unsigned int, blob*>::iterator it = blobs.begin(); it != blobs.end(); ++it){
    for(vector<off_set>::iterator vit = it->second->points.begin(); vit != it->second->points.end(); ++vit){
      mask[ *vit ] = true;
      blobMap[ *vit ] = (*it).second;
    }
  }
  
}

void Blob_mt_mapper::run()
{
  //  if(qsem)
  // qsem->acquire();
  cout << "Beg of Blob_mt_mapper run() " << pos.x << "," << pos.y << endl;
  float v;
  blob* tempBlob = new blob();
  for(uint z=0; z < pos.d; ++z){
    for(uint y=0; y < pos.h; ++y){
      for(uint x=0; x < pos.w; ++x){
	v = stack->lv(stack_channel, x, y, z);
	if(v < minEdge || mask[ z * pos.w * pos.h + y * pos.w + x])
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
  cout << "End of Blob_mt_mapper run() " << pos.x << "," << pos.y << endl;
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
  int dxe = x < ((int)pos.w - 1) ? 1 : 0;
  int dyb = y > 0 ? -1 : 0;
  int dye = y < ((int)pos.h -1) ? 1 : 0;
  int dzb = z > 0 ? -1 : 0;
  int dze = z < ((int)pos.d -1) ? 1 : 0;
  
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
  
  off_set new_off = (mdz) * pos.w * pos.h + (mdy) * pos.w + (mdx);
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
     x >= (int)pos.w-1 || y >= (int)pos.h -1 || z >= (int)pos.d-1)
    return(true);
  
  if(!b)
    return(false);

  int dxb = x > 0 ? -1 : 0;
  int dxe = x < ((int)pos.w - 1) ? 1 : 0;
  int dyb = y > 0 ? -1 : 0;
  int dye = y < ((int)pos.h -1) ? 1 : 0;
  int dzb = z > 0 ? -1 : 0;
  int dze = z < ((int)pos.d -1) ? 1 : 0;
  blob* nBlob;
  if(tight){
    for(int dx=dxb; dx <= dxe; ++dx){
      for(int dy=dyb; dy <= dye; ++dy){
	for(int dz=dzb; dz <= dze; ++dz){
	  if( !(dx || dy || dz) )
	    continue;
	  nBlob = blobMap[ (dz + z) * pos.w * pos.h + (dy + y) * pos.w + (dx + x) ];
	  if(nBlob != b)
	    return(true);
	}
      }
    }
    return(false);
  }
  
  for(int d=-1; d <= 1; d += 2){
    if( (d + x) >= 0 && (d + x) < (int)pos.w){
      if(b != blobMap[(x + d) + y * pos.w + z * pos.w * pos.h])
	return(true);
    }
    if( (d + y) >= 0 && (d + y) < (int)pos.h){
      if(b != blobMap[x +  (y + d) * pos.w + z * pos.w * pos.h])
	return(true);
    }
    if( (d + z) >= 0 && (d + z) < (int)pos.d){
      if(b != blobMap[x + y * pos.w + (z + d) * pos.w * pos.h])
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
  if(!blobModel)
    return;
  incrementBlobModel(b, blobModel);
}

void Blob_mt_mapper::incrementBlobModel(blob* b, BlobModel* model)
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
  model->lockMutex();
  model->setPeak(fx, fy, fz, stack->lv(stack_channel, b->peakPos));
  for(uint i=0; i < b->points.size(); ++i){
    toVol(b->points[i], x, y, z);
    model->addPoint(x, y, z, stack->lv(stack_channel, b->points[i]));
  }
  model->unlockMutex();
}

void Blob_mt_mapper::deleteBlobs()
{
  for(multimap<unsigned int, blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it)
    delete (*it).second;
  blobs.clear();
}

// // creates a blobMap if not present. (needs to be used carefully to avoid memory leak)
// // assumes that all blob_mt_mappers refer to the same imageStack area.
// // Use carefully.. 
// void Blob_mt_mapper::overlappingBlobs(blob_set& bset)
// {
//   if(!blobMap)
//     setBlobMap();
  
// }

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

void Blob_mt_mapper::z_normalise(float* v, unsigned int length)
{
  float mean = 0;
  float std = 0;
  for(unsigned int i=0; i < length; ++i)
    mean += v[i];
  mean /= (float)length;
  for(unsigned int i=0; i < length; ++i)
    std += ((mean - v[i]) * (mean - v[i]));
  std = sqrt( std / (float)length );
  for(unsigned int i=0; i < length; ++i)
    v[i] = ( v[i] - mean ) / std;
}

blob* Blob_mt_mapper::blob_at(off_set pos)
{
  if(!blobMap)
    setBlobMap();

  return( blobMap[ pos ] );
}
