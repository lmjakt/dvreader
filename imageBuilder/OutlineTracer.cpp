#include "OutlineTracer.h"

using namespace std;

OutlineTracer::OutlineTracer(unsigned char* msk, int x, int y, unsigned int m_width, unsigned int m_height, unsigned int g_width, bool destructive)
{
  mask = msk;
  mask_width = m_width;
  mask_height = m_height;
  mask_x = x;
  mask_y = y;
  global_width = g_width;
  deleteMask = destructive;
}

OutlineTracer::OutlineTracer(unsigned char* msk, unsigned int m_width, unsigned int m_height, bool destructive)
{
  mask = msk;
  mask_width = m_width;
  mask_height = m_height;
  mask_x = 0;
  mask_y = 0;
  global_width = m_width;
  deleteMask = destructive;
}

OutlineTracer::~OutlineTracer()
{
  if(deleteMask)
    delete []mask;
}

vector<int> OutlineTracer::traceOutline(unsigned char border)
{
  vector<int> points;
  // These arrays contain coordinates that allow a clockwise rotation around
  // the current point; with the starting point decided by the previous entry position
  // It's horrible to read and understand the flow.
  int xoffsets[] = {-1, 0, 1, 1, 1, 0, -1, -1};
  int yoffsets[] = {1, 1, 1, 0, -1, -1, -1, 0};     // this gives us a clockwise spin around a central position.. 
  int offset_offsets[] = {5, 6, 7, 0, 1, 2, 3, 4};  //    
  int offsetPos = 0;
  
  int x, y;
  x = y = -1;
  for(int i=0; i < (int)(mask_width * mask_height); ++i){
    if(mask[i] & border){
      x = i % mask_width;
      y = i / mask_width;
      break;
    }
  }
  if(x == -1 || y == -1)
    return(points);

  bool tracing = true;
  int mask_origin = y * mask_width + x;
  points.push_back( global_width * (y + mask_y) + x + mask_x );
  while(tracing){
    for(int i=0; i < 8; ++i){
      int op = (offsetPos + i) % 8;
      int nx = x + xoffsets[op];
      int ny = y + yoffsets[op];
      if( ny * (int)mask_width + nx == mask_origin ){
	//points.push_back( globalWidth * (y + mask_y) + x + mask_x ); // use nx and ny below this should not be needed
	tracing = false;
	break;
      }
      if(nx < 0 || nx >= (int)mask_width)
	continue;
      if(ny < 0 || ny >= (int)mask_height)
	continue;
      if(mask[ ny * mask_width + nx ] & border){
	points.push_back( global_width * (ny + mask_y) + nx + mask_x );  //SURELY THIS SHOULD BE nx and ny not x and y???
	offsetPos = offset_offsets[op];
	x = nx;
	y = ny;
	break;
      }
    }
    if(points.size() < 2)    // in the case of a single point this can happen.
      break;
  }
  return(points);
}
