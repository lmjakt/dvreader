#include "blobMapper.h"
#include <iostream>

using namespace std;

BlobMapper::BlobMapper(ImageAnalyser* ia)
{
    image = ia;
    image->dims(width, height, depth);
//    vMask = new VolumeMask((unsigned long)width, (unsigned long)height, (unsigned long)depth);
    blobMap = new VolumeMap(width, height, depth);
}

BlobMapper::~BlobMapper()
{
//    delete vMask;
    delete image;
    delete blobMap;
}

// margin only refers to horizontal stuff.
set<blob*> BlobMapper::mapBlobs(float minEdge, unsigned int wi, int window, bool eatContained, bool eat_neighbors){
//    vMask->zeroMask();
    blobs.clear();
    blobMap->clear();
    waveIndex = wi;
    // go through all voxels and make some little paths..
    for(int z=0; z < depth; ++z){
	cout << "Looking at section " << z << endl;
	for(int y=0; y < height ; ++y){
	    for(int x=0; x < width; ++x){
		if(blobMap->masked(x, y, z))
		    continue;
		if(value(x, y, z) < minEdge)
		    continue;
//		cout << "calling makeBlob " << x << "," << y << "," << z  << endl;
		makeBlob(x, y, z, window);
	    }
	}
    }
    finaliseBlobs();
    // if eatContained then we eat contained and refinalise..
    if(eatContained){
	cout << "Eating contained blobs" << endl;
	eatContainedBlobs();
	finaliseBlobs();
    }
    if(eat_neighbors){
	cout << "Eating neighbours" << endl;
	eatNeighbors();
	finaliseBlobs();
	cout << "finaliseBlobs returned" << endl;
	cout << "giving us a blob size of " << blobs.size() << endl;
    }
    int count = 0;
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it){
	cout << "blob " << ++count << "  of " << blobs.size() << endl;
	cout << "blob after finalising with size of : " << (*it)->points.size() << endl;
    }
    cout << "and just about to return the blobs to the parent" << endl;
    return(blobs);
}

void BlobMapper::makeBlob(int x, int y, int z, int w){
    // make a new blob.
    blob* b = new blob();
    
    b->points.push_back( linear(x, y, z) );
    b->values.push_back( value(x, y, z) );
    b->surface.push_back(true);
    //vMask->setMask(true, x, y, z);
//    cout << "inserting new blob at : " << x << "," << y << "," << z << endl;
    blobMap->insert(x, y, z, b);
    blobs.insert(b);
//    cout << "Made a new blob. " << blobs.size() << endl;
    extendBlob(x, y, z, b, w);
//    cout << "and blob extended" << endl;
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
//	cout << "oldBlob is null adding to this one" << endl;
	b->points.push_back( linear(nx, ny, nz) );
	b->values.push_back( value(nx, ny, nz) );
	b->surface.push_back(false);
	if(!blobMap->insert(nx, ny, nz, b)){
	    cerr << "unable to insert into map at : " << nx << "," << ny << "," << nz << endl;
	    exit(1);
	}
//	vMask->setMask(true, nx, ny, nz);
//	blobMap.insert(make_pair( linear(nx, ny, nz), b));
	//// and Recurse..
	extendBlob(nx, ny, nz, b, w);
	return;

    }
//    cout << "old blob has size " << oldBlob->points.size() << endl;
    // It is possible to come to a circle here. If this is the case we can just return.
    if(oldBlob == b)
	return;
    // if we are here we need to join this blob with another one.
    // Do this in another function.
    
//    cout << "calling mergeBlobs" << endl;

    mergeBlobs(b, oldBlob);

//    cout << "merge returned" << endl;
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
		    //if(!vMask->mask(x + dx, y + dy, z + dz))
		    if(!nblob || nblob != b)
			return(true);
		}
	    }
	}
	return(false);
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
    oldBlob->blobs.push_back(newBlob);
    for(uint i=0; i < newBlob->points.size(); ++i){
//	oldBlob->points.push_back(newBlob->points[i]);
//	oldBlob->values.push_back(newBlob->values[i]);
//	oldBlob->surface.push_back(newBlob->surface[i]);
	// and change the map positions..
	blobMap->insert(newBlob->points[i], oldBlob);
    }
//    cout << "inserted new blob points into old blob" << endl;
    // and then let's get rid of newBlob.
    blobs.erase(newBlob);
//    cout << "erased new blob from blobs" << endl;
 
//   delete(newBlob);

//    cout << "deleted new blob" << endl;
}

void BlobMapper::eatContainedBlobs(){
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it){
//	cout << "Eating contained blobs" << endl;
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
//	    cout << "Set local mask at: " << x << "," << y << "," << z << endl;
	}
    }
	
    // Find the first non-surface member, and flood fill the surface from there using a recursive function.
    for(uint i=0; i < b->points.size(); ++i){
	if(!(b->surface[i])){
	    toVol(b->points[i], x, y, z);
	    //cout << "calling eatContents at : " << x << "," << y << "," << z << " blob size is : " << b->points.size() << endl;
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
	    cout << i << " : " << tx << "," << ty << "," << tz << "\t:" << b->surface[i] << endl;
	}
	vm->printMask();
	exit(1);
	return;
    }
    
    blob* nblob = blobMap->value(x, y, z);
    if(nblob != b){
//	cout << "merging blob" << endl;
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
    cout << "Eat Neighbours calling mergeBlobs, blob# is " << blobs.size() << endl;
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
    int count = 0;
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it){
	++count;
	cout << "about to call finalise blob on : " << count << "'th blob of " << blobs.size()  << endl;
	cout << "size of blob is " << (*it)->points.size() << endl;
	if(!fake)
	    finaliseBlob(*it);
    }
    cout << "end of finalise blobs?" << endl;
}

void BlobMapper::finaliseBlob(blob* b){
    if(!b->points.size())
	return;
    // To flatten it, first work out the size and then call the flatten function.
    unsigned int blobSize = 0;
    cout << "calling size" << endl;
    b->size(blobSize);
    cout << "Blob size with contained blobs is now : " << blobSize << endl;
    b->points.reserve(blobSize);
    b->values.reserve(blobSize);
    b->surface.reserve(blobSize);
    cout << "calling flatten" << endl;
    b->flatten(b);
    cout << "flattened went nice " << endl;

    
    int x, y, z;
//	cout << "finalise size " << b->points.size() << " : " << b->values.size() << " : " << b->surface.size() << endl;
    toVol(b->points[0], x, y, z);
//	cout << "\t\t" << b->points[0] << " maps to " << x << "," << y << "," << z << endl;
    
    b->max = b->values[0];
    b->min = b->values[0];
    
    b->peakPos = b->points[0];
    b->min_x = b->max_x = x;
    b->min_y = b->max_y = y;
    b->min_z = b->max_z = z;
    // and then go through and check everything..
    for(uint i=0; i < b->points.size(); ++i){
	toVol(b->points[i], x, y, z);
	b->surface[i] = isSurface(x, y, z, b);
	if(b->max < b->values[i]){
	    b->max = b->values[i];
	    b->peakPos = b->points[i];
	}
	if(b->min > b->values[i])
	    b->min = b->values[i];
	
	if(b->min_x > x)
	    b->min_x = x;
	if(b->max_x < x)
	    b->max_x = x;
	if(b->min_y > y)
	    b->min_y = y;
	if(b->max_y < y)
	    b->max_y = y;
	if(b->min_z > z)
	    b->min_z = z;
	if(b->max_z < z)
	    b->max_z = z;
    }
    cout << "end of finaliseBlob" << endl;
}

