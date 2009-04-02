#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <string.h>    // for memcopy
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include "colorMapper.h"
#include "dataStructs.h"
#include "kcluster/kClusterProcess.h"



class imageData {
    /// takes a reference to an ifstream pointing at a scratch file containing the floats
    /// in the data form that we tend to like, i.e. very similar to rgba (float). This is used to get
    /// the data for a given frame, by caclculating the approppriate offsets within the scratch file
    float* data; 
    float* currentFrameData;   // Basically the current frame, but includes data for merged channels..(important for distributions)
    float* frameBuffer;        // read from scratch file. Does not include the merged channels.
    //std::ifstream* in;               // points to all the data.. 
    //groupObject** objects;
    float* frame;  // the current frame.. in rgb format depending on the given scaleFactor and biasFactor and waveColors..
    float* x_z_frame;   // this is much more difficult to deal with, and might be very slow..
    float* x_z_FrameData;   // this will hold the values for all the channels including the merged ones, not actually necessary, but needed for the colormapper
    float* x_z_FrameBuffer;  // pack the appropriate data into this one. One value for each channel
//    float* y_z_frame;
//    float* y_z_FrameData;
//    float* y_z_FrameBuffer;
    float* colors; // 3 * number of wavelength in rgb format. basically the colour represented by the given factor.. 
    float* biasFactors;
    float* scaleFactors;
    bool* additive;    // whether the resulting values should be added or subtracted.. 
    bool* displayChannel;   // whether or not we display a channel.. 
    int* xOffsets;
    int* yOffsets;
    int* zOffsets;   // one for each wavelength.. but no more.. 
    int* wavelengths;
    int x, y, z, w;    // the dimensions of the image... 
    int globalMargin;   // the margin of the data set.. 
    float x_scale, y_scale, z_scale;   // the distance (in microns) of 1 pixel in the given dimension.. 
    int mergedWaves;
    int* mergedSizes;
    int** mergedDefs;   // which waves to merge... or something like that...
    float* mergedColors;   // 3 * the number of merged things..
    bool useComponentFactors;  // use the component factors for the merged channels (default to false)
    std::vector<ColorMapper*> colorMappers;   // maps values to colours... 
    ColorMapper* sliceMapper;                  // map colour for x_z slices. Just use one as the data is small.. 
    
    std::map<int, threeDPeaks> volumePeaks;    // peaks in three dimensions (one for each wave index.. )... 

    std::vector<voxel> findPeaks(float* line, int length, int pr, float minPeakValue, float maxEdgeValue, bool pts=false);  // dumb function, just returns the peak offsets
    std::vector<linearPeak> findPeaks(float* line, int length, int pr, float minPeakValue, float maxEdgeValue, int dim);  // also dumb. doesn't know how to define positions.. 
                                                                                                                 // so can't actually make proper linearPeak objects, but that is ok.
                                                                                                  // but better to isolate the peak finding function so that changes we make here propagate
    std::vector<int> findPeaks(std::vector<voxel>& line);   // simple peak finding algorithm, optimised for other stuff.. (basically just checks if there is more than one peak..
    std::vector<line> nuclearLines;                         // lines painting nuclei.. 
    std::vector<std::vector<twoDPoint> > nuclearPerimeters;      // the perimeters of the nuclei.. 

    std::vector<Nucleus> nuclei;
    // lets have a universal function for obtaining the colour data for a given frame number.
    float* readFrame(int frameNo, int waveIndex);  
    void readFrame(int frameNo, int waveIndex, int x_begin, int y_begin, int width, int height, float* dest);
    void paintVolumePeaks(threeDPeaks& peaks, float* dest, int frameNo);    // set colour of peak positions to appropriate value.. 
    void setDropVolumes(float bgm);    // go through 3dpeaks and assign the drop volumes so that we can calculate the stuff.. 
//    void setDropVolumes(int radius);    // go through 3dpeaks and assign the drop volumes so that we can calculate the stuff.. 
    void paintPerimeter(int margin, float* dest);
    void paintNuclearLines(float* dest);
    void findNuclearPerimeters();   // uses the nuclearLines defined.. (does a kind of merge of these ..).
    Nucleus findNuclearPerimeter(const std::set<line>& lines);
//    std::vector<twoDPoint> findNuclearPerimeter(const std::set<line>& lines);

    public :
	// then some functions... get the user to set things... 
	// and handle memory allocation...  
	~imageData(); 
    imageData(float* imData, int xn, int yn, int zn, int wn, int* wl, float x_s, float y_s, float y_z); //{
//    imageData(std::ifstream* scratch, int xn, int yn, int zn, int wn, int* wl, float x_s, float y_s, float y_z); //{
    
    void setColor(int index, float r, float g, float b);
    void setBiasAndScale(std::vector<float> bias, std::vector<float> scale);
    void setAdditive(std::vector<bool> addFactors);
    void setOffsets(int index, int xo, int yo, int zo);  // set the appropriate things.. 
    void addMergeChannel(int* mergeI, int s);
    void toggleUseComponentFactors(bool on);
    float* setImage(int frameNo);
    float* set_x_zFrame(unsigned int ypos);   
    // float* set_y_zFrame(unsigned int xpos);   // using a scratch file this is far too difficult to do
    float* frameData(int frameNo);
    float* currentRGBImage();
    int channelNo();
    std::vector<std::vector<float> > xLine(int ypos);   // return a vector of values for each channel for the given y position
    std::vector<std::vector<float> > yLine(int xpos);   // as for the above but for a line in the y field... 

    bool simpleLine(float* line, unsigned int dimension, unsigned int pos1, unsigned int pos2, unsigned int waveIndex);  /// assigns values to line (which should be appropriate size..).
    // create an interpolated line between two arbitrary points. The line has the distance between the two points * mult number of points.
    // It uses a virtual box to calculate a value for intermediate (i.e. pretty much all) points. 
    float* arbitraryLine(unsigned int wl, unsigned int x_begin, unsigned int y_begin, unsigned int z_begin, unsigned int x_end, unsigned int y_end, unsigned int z_end, unsigned int points);

    linearPeaks findLocalMaxima(int wl, int pr, float minPeakValue, float maxEdgeValue);
    linearPeaks findLocalMaxima(float* source, int pr, float minPeakValue, float maxEdgeValue);
    void findAllMaxima(int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm);   // the same as above but goes through all the slices.. 

    void findAllPeaks(int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm);   // the same as above but goes through all the slices.. 

    KClusterProcess* clusterDrops(int wi, int K);   // cluster drops for some waveIndex.. 
    std::vector<float> spotValues(int wl);  // return the values associated with all the spots for a given waveIndex.. 
    voxelVolume makeModel(int xBegin, int width, int yBegin, int height, int frameBegin, int depth, std::set<int> waveIndices);  //
    void setDropThresholds(int wi, float minT, float maxT);  // sets the thresholds for the appropriate thingy.. 
    void recalculateSpotVolumes();
    void findNuclearPerimeters(int wi, float minValue);   // simple algorithm for finding nuclei.. (uses only the current frame data.. ) 
    std::vector<Nucleus> currentNuclei();                 // get the current nuclei.. !! 
};


#endif
