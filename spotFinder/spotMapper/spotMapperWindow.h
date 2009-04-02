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

#ifndef SPOTMAPPERWINDOW_H
#define SPOTMAPPERWINDOW_H

#include "spotMapper.h"
#include "nearestNeighborMapper.h"
#include "spotPerimeterMapper.h"
#include "../../dataStructs.h"
#include "../perimeter.h"
#include "../perimeterWindow/perimeterPlotter.h"
#include "../cell.h"
#include <vector>
#include <QWidget>
#include <QSpinBox>
#include <QLabel>     // so we can update the label..
#include <QHBoxLayout>
#include <QCheckBox>

class DeltaViewer;  // so I can pass a pointer to the owner class

class SpotMapperWindow : public QWidget
{
    Q_OBJECT
	public :
	SpotMapperWindow(DeltaViewer* dv, int gw, int gh, int gd, int texSize, QWidget* parent=0);
    ~SpotMapperWindow(){
	if(backgroundImage)
	    delete backgroundImage;
	delete spotMapper;
    }
    // gw = globalWidth, gh = globalHeight, gd = globalDepth
    public slots :
    void setPerimeters(std::vector<PerimeterSet> perimeters, float wl, int sno);
    void setBlobs(std::map<int, threeDPeaks*> Blobs);  // these is the blob information.. 

    private slots :
	// first a set of slots for handling the drawing of background and stuff.. 
    void goLeft();              // go Left by one
    void goRight();
    void goUp();
    void goDown();              // a bit ugly to have them like this, .. 

    // and then something for handling the drawing of other stuff.. 
    void setBlob(int bNo, bool update=true);  // this is connected from one of the QSPinBoxes.. to set an initial blob for mapping
    void mapOneBlob();      // maps the current blob..
    void mapAllVisible();   // maps those blobs in the current view to things.. 
    void mapAllBlobs();     // map all blobs to nuclei and remember this somehow.. 

    void mapAllByNeighbor();
    void mapOneByNeighbor();

    void mapPerimeter();   // map the perimeter of the points associated with the currently selected nuclei.. 

    void makeCells();    // makes a cell for every selectedPerimeters (the nuclei) and then write something to file. (I should keep the cells, but?)..
    
    void waveLengthSelectionChanged();  // when we make a change to the wavelengths.. 
    
    void updateBlobs(int x, int y);     // work out what blobs should be shown, and then call some kind of update function x and y are the x and y_positions
    void updateBackground(int x, int y);
    void updateVisibility(int x, int y);
    void updatePerimeters(int x, int y);
//    void updatePlotter();               // tells the plotter to plot .. 
    void setPosLabel(int x, int y);
    void newMousePos(int x, int y);

    private :
	std::map<int, threeDPeaks*> blobs;
    std::vector<threeDPoint> visibleBlobs;  // visible blobs.. this is just used for experimentation.. 
    std::vector<threeDPoint> allBlobs;      // all blobs we might not need to keep these, but .. 
    std::vector<simple_drop*> selectedDrops; // so we can set the id easily when we map them .. 
    std::vector<PerimeterSet> perSets;      // the perimeters that we are using..
    std::vector<Perimeter> selectedPerimeters;  // the actual perimeters that we want to use.. (make a copy .. )
    std::vector<Cell> cells;                    // store cell information in these.. 
    int currentNuclearPerimeter;                // if one gets selected.. 
    float waveLength;
    int sectionNo;     // these just refer to the section and wavelength used to created the perimeters.. 
  
    // the plotting window
    PerimeterPlotter* plotter;
    DeltaViewer* deltaViewer;
    SpotMapper* spotMapper;
    NearestNeighborMapper* nnMapper;
    SpotPerimeterMapper* spotPerimeterMapper;
    // and some interface elements that we need to remember..
    QSpinBox* vertDeltaBox;
    QSpinBox* horDeltaBox;   // these are used to set the distance moved by the things
    QSpinBox* blobSelector;  // selects the current blob (for viewing and for other stuff). Is limited to the current visible blobs

//    QSpinBox* diffuserSelector; // this selects the term d in the following equation: 1 - x^2/(d + x^2) which determines how the thing is handled. 
    
    std::map<int, QCheckBox*> blobChecks;  // which blobs will be included in the mapping and selection and otherthings.
    // changing the blobChecks creates a problem in that it is not easy to know which blob was being looked at the time..
    // we can sort of do this using the == operator, but it is not guaranteed to work..
    QHBoxLayout* checksBox;   // the box into which we put the check boxes.. 
    // some stuff that tells us which location we are looking at..
    
    QLabel* perimeterLabel;
    QLabel* positionLabel;
    QLabel* selBlobLabel;
    QLabel* mouse_X_label;
    QLabel* mouse_Y_label;
    
    QLabel* selectedPerimeterLabel;
    // int x_pos, y_pos;   // the actual origin is given as the x_pos * horDeltaBox->value() .. and these are bounds checked in the appropriate area.
    int x_origin, y_origin;  // these should only be changed in the updateBlobs section, -so that we can check them to determine if the area has changed
    int globalWidth, globalHeight, globalDepth;  // the area that we are looking at the time..
    float* backgroundImage;   // this is a bit bad.. -- I need to remember to delete it in the destructor
    int textureSize;

};
	
#endif
