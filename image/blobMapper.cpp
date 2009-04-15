#include "blobMapper.h"
#include <iostream>

using namespace std;

BlobMapper::BlobMapper(ImageAnalyser* ia)
{
    image = ia;
    image->dims(width, height, depth);
    vMask = new VolumeMask((unsigned long)width, (unsigned long)height, (unsigned long)depth);
}

BlobMapper::~BlobMapper()
{
    delete vMask;
    delete image;
}

// margin only refers to horizontal stuff.
set<blob*> BlobMapper::mapBlobs(float minEdge, unsigned int wi, int window, bool eatContained){
    vMask->zeroMask();
    blobs.clear();
    blobMap.clear();
    waveIndex = wi;
    // go through all voxels and make some little paths..
    for(int z=0; z < depth; ++z){
	cout << "Looking at section " << z << endl;
	for(int y=0; y < height ; ++y){
	    for(int x=0; x < width; ++x){
		if(vMask->mask(x, y, z))
		    continue;
		if(value(x, y, z) < minEdge)
		    continue;
		makeBlob(x, y, z, window);
	    }
	}
    }
    finaliseBlobs();
    // if eatContained then we eat contained and refinalise..
    if(eatContained){
	eatContainedBlobs();
	finaliseBlobs();
    }
    return(blobs);
}

void BlobMapper::makeBlob(int x, int y, int z, int w){
    // make a new blob.
    blob* b = new blob();
    
    b->points.push_back( linear(x, y, z) );
    b->values.push_back( value(x, y, z) );
    b->surface.push_back(true);
    vMask->setMask(true, x, y, z);
    blobMap.insert(make_pair( linear(x, y, z), b));
    blobs.insert(b);
    cout << "Made a new blob. " << blobs.size() << endl;
    extendBlob(x, y, z, b, w);
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
    
    if(!vMask->mask(nx, ny, nz)){
	b->points.push_back( linear(nx, ny, nz) );
	b->values.push_back( value(nx, ny, nz) );
	b->surface.push_back(false);
	vMask->setMask(true, nx, ny, nz);
	blobMap.insert(make_pair( linear(nx, ny, nz), b));
	//// and Recurse..
	extendBlob(nx, ny, nz, b, w);
	return;

    }
    // It is possible to come to a circle here. If this is the case we can just return.
    if(blobMap[linear(nx, ny, nz)] == b)
	return;
    // if we are here we need to joing this blob with another one.
    // Do this in another function.

    mergeBlobs(b, blobMap[ linear(nx, ny, nz) ]);

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
		    
		    if(!vMask->mask(x + dx, y + dy, z + dz))
			return(true);
		    if(blobMap[linear(x + dx, y + dy, z + dz) ] != b)
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


// oldBlob eats new blob.
void BlobMapper::mergeBlobs(blob* newBlob, blob* oldBlob){
    for(uint i=0; i < newBlob->points.size(); ++i){
	oldBlob->points.push_back(newBlob->points[i]);
	oldBlob->values.push_back(newBlob->values[i]);
	oldBlob->surface.push_back(newBlob->surface[i]);
	// and change the map positions..
	blobMap[ newBlob->points[i] ] = oldBlob;
    }
    // and then let's get rid of newBlob.
    blobs.erase(newBlob);
    delete(newBlob);
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
    
    if(!vMask->mask(x, y, z)){
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
    
    if(blobMap[linear(x, y, z)] != b){
//	cout << "merging blob" << endl;
	mergeBlobs(blobMap[linear(x, y, z)], b);
    }
    
    // check the 6 directions for non-member, non-surface..
    for(int d=-1; d < 2; d += 2){
	eatContents(b, vm, x + d, y, z);
	eatContents(b, vm, x, y + d, z);
	eatContents(b, vm, x, y, z + d);
    }
}

void BlobMapper::finaliseBlobs(){
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it){
	finaliseBlob(*it);
    }
}

void BlobMapper::finaliseBlob(blob* b){
    if(!b->points.size())
	return;
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
}
