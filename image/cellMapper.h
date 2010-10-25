#ifndef CELLMAPPER_H
#define CELLMAPPER_H

// This class implements a sort of cell specific watershade algorithm.
// It uses 2 float* of identical dimensions, one containing intensities
// of something that marks the cell (eg autofluorescence) and something
// that marks nuclei.

// Unfortunately we'll probably end up needing to use several parameters
// for the mapping process. 

// A minimum intensity for a cell body, a minium intensity to be considered
// a nuclear position. These may be based on background levels, or some sort
// of distribution based number. (Based on random sampling perhaps.

// The process should look something like the following..
// If c(xyz) > minCellValue
//     expand(cluster)          // follow highest neighbour and neigbhour > present position
//        until 
//           n(xyz) > min 
//           or no neighbour higher than present.
//           or highest neighbour has been assigned to a nuclear position or edge (left/right/top/bottom)
// 
// That will give us lots of clusters, the problem then is how to merge into cells.
// Maybe first try to work out a 3 dimensional gaussian.. 

#include <QThread>



#endif
