#include "blobMapper.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

BlobMapper::BlobMapper(ImageData* ia)
{
    image = ia;
    image->dims(width, height, depth);
    cout << "Made a new BlobMapper and an image thingy with dims : " << width << "x" << height << "x" << depth << endl;
    blobMap = new VolumeMap(width, height, depth);
}

BlobMapper::~BlobMapper()
{
    delete image;
    delete blobMap;
}

// margin only refers to horizontal stuff.
set<blob*> BlobMapper::mapBlobs(float minEdge, unsigned int wi, int window, bool eatContained, bool eat_neighbors){
    blobs.clear();
    blobMap->clear();
    waveIndex = wi;
    blob* tempBlob = new blob();
    // go through all voxels and make some little paths..
    for(int z=0; z < depth; ++z){
	cout << "Looking at section " << z << endl;
	for(int y=0; y < height ; ++y){
	    for(int x=0; x < width; ++x){
		if(blobMap->masked(x, y, z))
		    continue;
		if(value(x, y, z) < minEdge)
		    continue;
		tempBlob = initBlob(tempBlob, x, y, z, window);
	    }
	}
	cout << "\tblob#\t" << blobs.size() << endl;
	cout << "\tmapSize\t" << blobMap->mapSize() << endl;
	cout << "\tmemSize\t" << blobMap->memSize() << endl;
	cout << "\n\tblobSize\t" << sizeof(blob) << endl;
	//	uint blobNo = 0;
	//for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it)
	//    (*it)->childNo(blobNo);
	//cout << "Total number of blobs defined : " << blobNo << endl;
    }
    finaliseBlobs();
    // if eatContained then we eat contained and refinalise..

    cout << "Got a total of : " << blobs.size() << endl;
    eatContainedBlobs();
    finaliseBlobs();
    cout << "Blobs after eating contained : " << blobs.size() << endl;
    eatNeighbors();
    finaliseBlobs();
    cout << "Blobs after eating neighbours 1: " << blobs.size() << endl;
    eatNeighbors();
    finaliseBlobs();
    cout << "Blobs after eating neighbours 2 : " << blobs.size() << endl;
    eatNeighbors();
    finaliseBlobs();
    cout << "Blobs after eating neighbours 3 : " << blobs.size() << endl;
    eatNeighbors();
    finaliseBlobs();
    cout << "Blobs after eating neighbours 4 : " << blobs.size() << endl;

    eatContainedBlobs();
    finaliseBlobs();
    cout << "Blobs after final eating of contained : " << blobs.size() << endl;

    eatNeighbors();
    finaliseBlobs();
    cout << "Blobs after eating neighbours 5 : " << blobs.size() << endl;

 //    if(eatContained){
// 	cout << "Eating contained blobs" << endl;
// 	eatContainedBlobs();
// 	finaliseBlobs();
//     }
//      if(eat_neighbors){
//  	cout << "Eating neighbours" << endl;
//  	eatNeighbors();
//  	finaliseBlobs();
// // 	eatNeighbors();
// // 	finaliseBlobs();
//  	cout << "finaliseBlobs returned" << endl;
//  	cout << "giving us a blob size of " << blobs.size() << endl;
//      }
     
    delete(tempBlob);

    if(image->interp_no() > 1){
	for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it)
	    uninterpolate((*it));
    }
    return(blobs);
}

blob* BlobMapper::initBlob(blob* b, int x, int y, int z, int w){
    // don't make a new blob. Use the one provided.
    
    b->points.push_back( linear(x, y, z) );
    //b->values.push_back( value(x, y, z) );
    b->surface.push_back(true);
    b->max = b->min = value(x, y, z);
    b->peakPos = b->points[0];
    b->min_x = b->max_x = x;
    b->min_y = b->max_y = y;
    b->min_z = b->max_z = z;

    extendBlob(x, y, z, b, w);
    // if b is merged , it will have a size of 0, and we can just return it for reuse.
    // otherwise we'll need to add it's points into the relevant maps and make a new
    // temporary blob

    if(!b->points.size())
	return(b);
    blobs.insert(b);
    for(uint i=0; i < b->points.size(); ++i)
	blobMap->insert(b->points[i], b);
    blob* tempBlob = new blob();
    return(tempBlob);

}

void BlobMapper::extendBlob(int x, int y, int z, blob* b, int w){
    float v = value(x, y, z);
    float mv = v;
    int mdx, mdy, mdz;
    mdx = mdy = mdz = 0;
    for(int dx=-w; dx <= w; ++dx){
	if( (dx + x) < 0 || (dx + x) >= width)
	    continue;
	for(int dy=-w; dy <= w; ++dy){
	    if( (dy + y) < 0 || (dy + y) >= height)
		continue;
	    for(int dz=-w; dz <= w; ++dz){
		if( (dz + z) < 0 || (dz + z) >= depth)
		    continue;
		if( !(dx || dy || dz) )
		    continue;
		if(value(dx + x, dy + y, dz + z) >= mv){
		    mdx = dx; mdy = dy; mdz = dz;
		    mv = value(dx + x, dy + y, dz + z);
		}
	    }
	}
    }
    if( !(mdx || mdy || mdz) ){      // this is a local peak.
	return;
    }
    int nx = x + mdx;
    int ny = y + mdy;
    int nz = z + mdz;
    
    blob* oldBlob = blobMap->value(nx, ny, nz);
    if(!oldBlob){
	b->points.push_back( linear(nx, ny, nz) );
	b->surface.push_back(false);
	if(!blobMap->insert(nx, ny, nz, b)){
	    cerr << "unable to insert into map at : " << nx << "," << ny << "," << nz << endl;
	    exit(1);
	}
	// update mins and maxes.. since we go upwards, just update mv
	b->max = mv;
	b->peakPos = linear(nx, ny, nz);
	b->min_x = nx < b->min_x ? nx : b->min_x;
	b->max_x = nx > b->max_x ? nx : b->max_x;
	b->min_y = ny < b->min_y ? ny : b->min_y;
	b->max_y = ny > b->max_y ? ny : b->max_y;
	b->min_z = nz < b->min_z ? nz : b->min_z;
	b->max_z = nz > b->max_z ? nz : b->max_z;
	//// and Recurse..
	extendBlob(nx, ny, nz, b, w);
	return;

    }
    // It is possible to come to a circle here. If this is the case we can just return.
    if(oldBlob == b)
	return;

    addPointsToBlob(b, oldBlob);
    return;
}

bool BlobMapper::isSurface(int x, int y, int z, blob* b, bool tight){
    // if on edge then consider as a surface
    if(x <= 0 || y <= 0 || z <= 0 ||
       x >= width-1 || y >= height-1 || z >= depth-1)
	return(true);

    if(tight){
	for(int dx=-1; dx <= 1; ++dx){
	    if( (dx + x) < 0 || (dx + x) >= width)
		continue;
	    for(int dy=-1; dy <= 1; ++dy){
		if( (dy + y) < 0 || (dy + y) >= height)
		    continue;
		for(int dz=-1; dz <= 1; ++dz){
		    if( (dz + z) < 0 || (dz + z) >= depth)
			continue;
		    if( (!(dx || dy || dz)) || (dx && dy && dz) )
			continue;
		    blob* nblob = blobMap->value(x + dx, y + dy, z + dz);
		    if(!nblob || nblob != b)
			return(true);
		}
	    }
	}
	return(false);
    }
    if(blobMap->value(x, y, z) != b){
	cerr << "BlobMapper::isSurface blobMap giving wrong blob " << blobMap->value(x, y, z) << "  instead of " << b << endl;
	exit(1);
    }
    for(int d=-1; d <= 1; d += 2){
	if( (d + x) >= 0 && (d + x) < width){
	    if(!sameBlob(x + d, y, z, b))
		return(true);
	}
	if( (d + y) >= 0 && (d + y) < height){
	    if(!sameBlob(x, y + d, z, b))
		return(true);
	}
	if( (d + z) >= 0 && (d + z) < depth){
	    if(!sameBlob(x, y, z + d, b))
		return(true);
	}
    }
    return(false);
} 


////////////// There may be an issue with the mergeBlobs function, in that it removes elements from 
/////////////  a set (blobs) whilst we are iterating over that set. However, as long as we don't ask
/////////////  it to remove the set that is pointed to by the iterator it should be ok. So we have
////////////   to be very careful when using this function.

// oldBlob eats new blob.
void BlobMapper::mergeBlobs(blob* newBlob, blob* oldBlob){
  if(oldBlob == newBlob){
    cerr << "mergeBlobs asked to merge identical blobs. This is really bad" << endl;
    exit(1);
  }
  addPointsToBlob(newBlob, oldBlob);
  blobs.erase(newBlob);
  delete(newBlob);
}

void BlobMapper::addPointsToBlob(blob* tempBlob, blob* permBlob){
    permBlob->points.reserve(permBlob->points.size() + tempBlob->points.size());
    //permBlob->values.reserve(permBlob->points.size() + tempBlob->points.size());
    permBlob->surface.reserve(permBlob->points.size() + tempBlob->points.size());

    for(uint i=0; i < tempBlob->points.size(); ++i){
	blobMap->insert(tempBlob->points[i], permBlob);
	permBlob->points.push_back( tempBlob->points[i]);
	//permBlob->values.push_back( tempBlob->values[i]);
	permBlob->surface.push_back( tempBlob->surface[i]);  // Though this is rather stupid..
    }
    
    // update mins and maxes..
    // Although in the initial mapping procedure permBlob will always have the highest value, this
    // isn't necessarily true in the blob eating procedures so we'll need to check this.
    if(permBlob->max < tempBlob->max){
	permBlob->max = tempBlob->max;
	permBlob->peakPos = tempBlob->peakPos;
    }
    // and then the position maxes and mins.. 
    permBlob->min_x = min(permBlob->min_x, tempBlob->min_x);
    permBlob->max_x = max(permBlob->max_x, tempBlob->max_x);
    permBlob->min_y = min(permBlob->min_y, tempBlob->min_y);
    permBlob->max_y = max(permBlob->max_y, tempBlob->max_y);
    permBlob->min_z = min(permBlob->min_z, tempBlob->min_z);
    permBlob->max_z = max(permBlob->max_z, tempBlob->max_z);
    
    tempBlob->points.resize(0);
    tempBlob->surface.resize(0);
    // Don't delete as we want to reuse tempBlob;
}

void BlobMapper::eatContainedBlobs(){
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it){
	eatContainedBlobs(*it);
    }
}

void BlobMapper::eatContainedBlobs(blob* b){
    // First of all make a temporary mask that we can try to use to limit the
    // amount of recursion.
    VolumeMask* vm = new VolumeMask((unsigned long)(1 + b->max_x - b->min_x), (unsigned long)(1 + b->max_y - b->min_y), (unsigned long)(1 + b->max_z - b->min_z) );
    int x, y, z;
    for(uint i=0; i < b->points.size(); ++i){
	if(b->surface[i]){
	    toVol(b->points[i], x, y, z);
	    vm->setMask(true, x - b->min_x, y - b->min_y, z - b->min_z);
	}
    }
	
    // Find the first non-surface member, and flood fill the surface from there using a recursive function.
    for(uint i=0; i < b->points.size(); ++i){
	if(!(b->surface[i])){
	    toVol(b->points[i], x, y, z);
	    eatContents(b, vm, x, y, z);
	    break;
	}
    }
    // And delete the temporary mask.
    delete vm;
}


void BlobMapper::eatContents(blob* b, VolumeMask* vm, int x, int y, int z){
    int lx = x - b->min_x;
    int ly = y - b->min_y;
    int lz = z - b->min_z;
    if(vm->mask(lx, ly, lz))
	return;
    vm->setMask(true, lx, ly, lz);
    
    if(!blobMap->value(x, y, z)){
	cerr << "eatContents: " << x << "," << y << "," << z << " --> " << lx << "," << ly << "," << lz << endl;
	cerr << "Undefined voxel. We are not supposed to end up here" << endl;
	cerr << "blob info " << b->min_x << "-" << b->max_x << " : " << b->min_y << "-" << b->max_y << " : " << b->min_z << "-" << b->max_z << endl;
	for(uint i=0; i < b->points.size(); ++i){
	    int tx, ty, tz;
	    toVol(b->points[i], tx, ty, tz);
	    //cout << i << " : " << tx << "," << ty << "," << tz << "\t:" << b->surface[i] << endl;
	}
	vm->printMask();
	exit(1);
	return;
    }
    
    blob* nblob = blobMap->value(x, y, z);
    if(nblob != b){
	mergeBlobs(nblob, b);
    }
    
    // check the 6 directions for non-member, non-surface..
    for(int d=-1; d < 2; d += 2){
	eatContents(b, vm, x + d, y, z);
	eatContents(b, vm, x, y + d, z);
	eatContents(b, vm, x, y, z + d);
    }
}

void BlobMapper::eatNeighbors(){
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it)
	eatNeighbors(*it);
}

void BlobMapper::eatNeighbors(blob* b){
    map<blob*, NeighborInfo> neighbors;
    int x, y, z;
    // go through surface and find things..
    for(uint i=0; i < b->points.size(); ++i){
	if(b->surface[i]){
	    vector<off_set> nei = findNeighbors(b, b->points[i]);
	    for(uint j=0; j < nei.size(); ++j){
		blob* bb = blobMap->value(nei[j]);
		toVol(nei[j], x, y, z);
		neighbors[bb].addPoint( value(x, y, z) );
	    }
	}
    }
    // and then we simply find which neighbor has the highest sum, (we might want to
    // use the highest mean, or lowest min, or something like that. but let's start
    // simply.
    if(!neighbors.size())
	return;
    
    map<blob*, NeighborInfo>::iterator it = neighbors.begin();
    blob* maxBlob = it->first;
    float maxSum = it->second.sum;
    for(it=neighbors.begin(); it != neighbors.end(); ++it){
	if( maxSum < it->second.sum ){
	    maxBlob = it->first;
	    maxSum = it->second.sum;
	}
    }
    // the best neighbor is maxBlob, so just eat maxBlob
    if(maxBlob == b){
	cerr << "Eat neighbours asked to eat itself. This is bad, so will die" << endl;
	exit(1);
    }
    mergeBlobs(maxBlob, b);
}

vector<off_set> BlobMapper::findNeighbors(blob* b, off_set p){
    int x, y, z;
    toVol(p, x, y, z);
    vector<off_set> neighbors;
    // search only in the reasonable directions
    for(int d=-1; d < 2; d += 2){
	if( differentBlob(x+d, y, z, b) )
	    neighbors.push_back(linear(x+d, y, z));
	if( differentBlob(x, y+d, z, b) )
	    neighbors.push_back(linear(x, y+d, z));
	if( differentBlob(x, y, z+d, b) )
	    neighbors.push_back(linear(x, y, z+d));
    }
    return(neighbors);
}


void BlobMapper::finaliseBlobs(bool fake){
    cout << "Finalising blobs.." << endl;
    int r = 50;
    int g = 255;
    int b = 0;
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it){
	if(!fake)
	    finaliseBlob(*it);
	(*it)->r = r % 256;
	(*it)->g = g % 256;
	(*it)->b = b % 256;
	r += 50;
	g += 128;
	b += 200;
    }
    cout << "end of finalise blobs?" << endl;
}

void BlobMapper::finaliseBlob(blob* b){
    if(!b->points.size())
	return;
    
    int x, y, z;

    // and then go through and determine the surface
    for(uint i=0; i < b->points.size(); ++i){
	toVol(b->points[i], x, y, z);
	b->surface[i] = isSurface(x, y, z, b);
    }
}

void BlobMapper::uninterpolate(blob* b){
    uint ip = image->interp_no();
    if(ip == 1)
	return;
    int x, y, z;
    b->min_x *= ip;
    b->min_y *= ip;
    b->min_z *= ip;
    b->max_x *= ip;
    b->max_y *= ip;
    b->max_z *= ip;
    b->peakPos *= ip;    
//    cout << "\tmax and mins : " << b->min_x << "->" << b->max_x << "  :  " << b->min_y << "->" << b->max_y << "  :  " << b->min_z << "->" << b->max_z << endl;

    for(uint i=0; i < b->points.size(); ++i){
	toVol(b->points[i], x, y, z);
	//cout << "mapping : " << b->points[i] << "  " << x << "," << y << "," << z ;
	off_set np = (z * ip) * (width * height * ip * ip) + (y * ip) * (width * ip) + x * ip;
	//cout << " --> " << np << "  : " << np % (width * ip) << ","
	//   << (np % (width * height * ip * ip)) / (width * ip) << ","
	//    << np / (width * height * ip * ip) << endl;
	b->points[i] = np;
	
	
    }
}
