//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#ifndef SPOTWINDOW_H
#define SPOTWINDOW_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <qstring.h>
#include <qwidget.h>
#include <qcolor.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <values.h>
#include "../linGraph/plotWidget.h"
#include "channelWidget.h"
#include "modelWidget.h"
#include "nucleusWidget.h"
#include "setWidget.h"
#include "blurWidget.h"
#include "../dataStructs.h"
#include <string>

using namespace std;

// This class provides an interface for specifying paramaters and methods for finding spots.
// This class doesn't in fact do any spotfinding of its own as that is better handled by the
// class that sits on the actual data. However, this class allows the user to specify in what
// way to go looking for spots.

// This is essentially experimental. Since our spots are actually pretty clear from the backrgound (at least when we have
// at least two probes for each one) I'll start by attempting a simple directed method for looking for spots based on 
// looking for local maximal values. These initially this will be done separately for 2 d images, and then we'll extend this to the 
// the 3rd dimension.. at a later time..... (just join up local maxima). This might actually work reasonable easily.. 

struct norm_data {
    float mean;
    float standard_deviation;
    norm_data(){
	mean = 0;
	standard_deviation = 1;
    }
    norm_data(float m, float std){
	setValues(m, std);
    }
    void setValues(float m, float std){
	mean = m;
	standard_deviation = std;
	std = std < 0 ? -std : std;  // not possible to have negative standard deviation
	std = std == 0 ? MINFLOAT : std;   // and std deviation shouldn't be 0 either.
    }
};

class SpotWindow : public QWidget
{
    Q_OBJECT

    public :
	SpotWindow(int Width, int Height, int Depth, QWidget* parent=0, const char* name=0);
    
    void setLineValues(int slicePosition, int dim, std::vector<std::vector<float> > v, std::vector<int> m, int LMargin, int RMargin, float MinY=0, float MaxY=0);
    void setLineColors(std::vector<QColor> Colors);
    void setChannels(std::vector<QString> Channels);    // the names of the channels. (use string so that we can do merge channels as well)
    void setPeaks(std::map<int, linearPeaks> Peaks);
    void setNormalisations(std::vector<float*> areas, int width, int height);  // calculates the norm_data for each vawelength.
    void find_spots3D(string p_file);   // find all spots from p_file.. 

    private :
	void readSpotParameters(string p_file);

    PlotWidget* xPlot;   // plot a line across the x axis at some specified y position
    PlotWidget* yPlot;       // plot a line across the y axis at some specified x position
    std::vector<ChannelWidget*> channelWidgets;
    std::vector<norm_data> norm_values;   // if we normalise data .. 
    QVBoxLayout* channelBox;   // keep channelWidgets in here... 
    ModelWidget* modelWidget;
    NucleusWidget* nucleusWidget;
    NucleusWidget* contrastWidget;
    SetWidget* setWidget;   // bad name, but can't think of anything better.. 
    BlurWidget* blurWidget;  // and then a load of controls for other stuff.. but that's for later ...
    std::map<int, linearPeaks> linePeaks;
    std::map<int, std::vector<int> > slicePeaks(int slicePosition, int dim);   // dimension and slice Position.. 
    int xPosition;	
    
    int yPosition;
    int imageWidth, imageHeight, imageDepth;

//    void resizeEvent(QResizeEvent* e);
    
    private slots :
	void newPeakValue(int id, float pv);
    void newMaxEdgeValue(int id, float ev);
    void newScaleFactor(int id, float sf);
    void findSpots(int id, int wsize, float minPeakValue, float maxEdgeValue, float minCorrelation, int r, int g, int b);
    void findAllSpots(int id, int wsize, float minPeakValue, float maxEdgeValue, float minCorrelation, int r, int g, int b, int K, float bgm, bool exportFile);
    void findAllSpots3D(int id, int wsize, float minPeakValue, float maxEdgeValue, float minCorrelation, int r, int g, int b, int K, float bgm, bool exportFile);

    void saveSpotParameters();
    void readSpotParameters();

    signals :
	void findLocalMaxima(int, int, float, float);
    void findAllLocalMaxima(int, int, float, float, int, float, bool);  // the boolean indicates whether or not to export a file
    void findAllLocalMaxima3D(int, int, float, float, int, float, bool);  // the boolean indicates whether or not to export a file
    void increment_x_line(int);
    void increment_y_line(int);
    void setUseProjection(bool);
    void makeModel(int, int, int, int, int, int, set<int>);  // x, width, y, height, z, depth, channels (full thingy..)
    void recalculateSpotVolumes();        
    void findNuclearPerimeters(int, float);
    void findContrasts(int, float);     // not sure what the float represents at the moment, but it might be useful for something..
    void findSets(int, int, int, float); 
    void findSpotDensity(int, double, double);
    void blur(std::set<uint>, int, double, double);
};

#endif
