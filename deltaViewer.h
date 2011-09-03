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

#ifndef DELTAVIEWER_H
#define DELTAVIEWER_H

#include <QWidget>

#include <qtimer.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <q3buttongroup.h>
#include <qstring.h>
#include <QPoint>
//Added by qt3to4:
#include <QVBoxLayout>
#include "dvReader.h"
#include "jpgView/jpgView.h"
#include "opengl/glImage.h"
#include "distchooser/distChooser.h"
#include "distchooser/tabWidget.h"
#include "spotFinder/spotWindow.h"
#include "spotFinder/spotsWidget.h"
#include "spotFinder/perimeterWindow/perimeterWindow.h"
#include "spotFinder/spotMapper/spotMapperWindow.h"
#include "pointViewer/pointViewWidget.h"
#include "flatViewer/flatView.h"
//#include "linGraph/plotWidget.h"
#include "linGraph/distPlotter.h"
//#include "colorChooser.h"
#include "parameterChooser.h"
//#include "dataStructs.h"
#include "datastructs/channelOffset.h"
#include <vector>
#include <map>
#include <string>
#include "kcluster/kClusterProcess.h"
#include "kcluster/clusterWindow.h"
#include "panels/fileSet.h"
#include "panels/overlapWindow.h"
#include "image/imageAnalyser.h"
//#include "image/blobMapper.h"
#include "image/blobMapperWidget.h"
#include "image/blobMapperWidgetManager.h"
#include "image/backgroundWindow.h"
#include "image/rectangle.h"

class OverlapEditorWindow;
class ImageBuilderWidget;
class ValueLabels;
class ColorChooser;

using namespace std;

class DeltaViewer : public QWidget 
{
  Q_OBJECT
    
    public :
  DeltaViewer(std::map<std::string, std::string> opt_commands, int xyMargin, const char* ifName=0,  QWidget* parent=0, const char* name=0);
  ~DeltaViewer(){
    delete imageAnalyser;
    delete reader;
    delete glViewer;
    delete projection;
    delete objectSums;
    delete spotChooserTabs;
  }
  bool readToRGB(float* dest, int xb, int yb, unsigned int tw, unsigned int th, unsigned int slice_no);
  bool readToRGBPro(float* dest, int xb, int yb, unsigned int tw, unsigned int th);
  void int_color(int i, float& r, float& g, float& b);
  
 private :
  void setHome();
  void setRanges(QString text);  // take a word or a line or something and set the parameters.. 
  QString readRanges();
  bool setVisible(int& x, int& y, int& width, int& height);
  std::vector<color_map> readColors(std::string fname);
  std::vector<channel_info> collect_channel_info();
  
  private slots :
  void nextImage();
  void previousImage();
  void firstImage();
  void lastImage();
  void setImage(int slice);
  void paintCoverage(float maxCount);
  //void setProjection();
  //bool readProjection(ifstream& in);  // read from a file..
  void showOverlapEditor();
  void newStackSelected(float x, float y);
  void adjustStackPosition(float x, float y, QPoint p);
  void updateFileSetInfo();
  void paintProjection();
  void exportProjection();
  QColor wiColor(int wi);
  void paintPeaks(float* area, int px, int py, int w, int h);   // this checks if there are peaks to be painted from the spotsWidgets .. 
  //void paintBlobs(float* area, int xo, int yo, int z, int w, int h);
  void paintBlobs(float* area, int xo, int yo, int z, int w, int h, bool isProjection=false);
  void paintBlobs(float* area, int xo, int yo, int z, int w, int h, 
		  BlobMapperWidget* blb, bool isProjection);
  void paint(float* area, std::vector<std::vector<o_set> >& points, Rectangle rect, int z, color_map cm);
  
  void exportPeaks();  // write the peaks to a file in a reasonable manner.. 
  void paintNuclei(float* area, int px, int py, int w, int h);  // and this checks if there are any nuclei to be painted.. (but a bit more tricky)
  void paintParameterData(float* area, int px, int py, int w, int h);
  void setSliceNo(int n);
  void displayImage();
  void displaySlices();
  void startTimer();
  void stopTimer();
  //  void changeMag(int m);
  void calculateDistribution();
  void updateDistToggled(bool on);   // show if hidden.. 
  void useComponentsToggled(bool on);
  void changeRanges(float mn, float mx);
  void changeDropThresholds(float minT, float maxT);
  void setAdditive(bool b);   // but b is roundly ingored... 
  void copyRanges();    // copy all the ranges to the clipboard,
  void pasteRanges();   // paste all ranges to the distChoosers.. 
  void saveRanges();    // save ranges to a file.. 
  void readRangesFromFile();
  void readRangesFromFile(QString fileName);

  void setWaveColors(int wi, float r, float g, float b);
  void setWaveColors();   // basically just for init.. 
  //void findObjects(int wl);
  //  void clearObjects();
  
  void addMergedChannel();
  
  void newMagnification(float mag);
  void newOffsets(int xo, int yo);  // new x and y offsets from the glImage.. 
  void newMousePosition(int xp, int yp); // the current mouse position.. but not the signal..
  void newProMousePosition(int xp, int yp);
  
  void setSlices(int xpos, int ypos);
  void increment_x_z_slice(int delta);
  //void increment_y_z_slice(int delta);
  void increment_x_line(int delta);
  void increment_y_line(int delta);
  void setSpotsUseProjection(bool useProjection);
  
  // for changing channel offsets..
  void offSetChanged(int id);
  void channelOffsetIdChanged(int id);
  void setLinePlotterColors();
  void plotLines();
  void findLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue);
  void findAllLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue, int clusterK, float bgm, bool exportFile, bool use3D);
  void findAllLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue, int clusterK, float bgm, bool exportFile);
  void findAllLocalMaxima3D(int wl, int radius, float minPeakValue, float maxEdgeValue, int clusterK, float bgm, bool exportFile);
  void find_spots(string p_file);  // read parameters from p_file and find a load of spots and then export them
  void findBlobs(std::string& params);
  void determineSpotDensities(int r, double sigma, double order);  // use a gaussian blur model to give densities.. 
  void blur(std::set<uint> channels, int r, double sigma, double order); // make blurred intensity map for the indicated things
  
  void makeModel(int xBegin, int Width, int yBegin, int Height, int zBegin, int Depth, set<int> channels);
  void recalculateSpotVolumes();
  void findNuclearPerimeters(int wi, float minValue);
  void findContrasts(int wi, float value);   // not sure what the value represents, but.. 
  void mapBlobs(int wi, float minEdge);
  void mapBlobs(int wi, float minEdge, int xw, int yw, int zw, float pcnt);
  void makeBlobModel(int xy_radius, int z_radius);

  void mapCavities(int wi, int xr, int yr, int zr, float P, float DP);
  void findSets(int wi, int minSize, int maxSize, float minValue);
  
  //  void setSpotsToSpotMapper();
  
  void newLine(int x1, int xy1, int x2, int y2);
  
  void peakColorsChanged();
  void peakRepTypeChanged();   // just wrappers that call paintProjection for now anyway.. 
  void nuclearColorsChanged();
  
  //  void peakColorsChanged(int wl, float r, float g, float b);
  //  void peakRepTypeChanged(DropRepresentation dr);   // just wrappers that call paintProjection for now anyway.. 
  //  void nuclearColorsChanged(int wl, float r, float g, float b);
  
  
  void paramDataRangeChanged();
  void paramDataColorChanged();
  void paramDataRangeChanged(float lowt, float hight);
  void paramDataColorChanged(int wl, float r, float g, float b);
  
  void setBackgroundPars(std::map<fluorInfo, backgroundPars> bgPars);
  void setChooserMaxValue(float mv);

 private :
  /// Move this later, bull
  BlobMapperWidgetManager* blobManager;
  BackgroundWindow* backgroundWindow;   
  ImageBuilderWidget* imageBuilder;
  vector<vector<o_set> > aux_points;   // for quick checks of various functions.
  float maxLevel;
  //      set<BlobMapperWidget*> blobs;
  
  QString fName;
  DVReader* reader;
  FileSet* fileSet;
  ImageAnalyser* imageAnalyser;
  //  std::vector<raw_data*> projection_data;   // data for a projection ..   we need one projection for each panel..
  
  //  std::vector<float> projection_means;
  //std::vector<float> projection_stds;  // use these as a substitute for the real ones. They'll be a bit higher, but maybe that's ok.
  
  GLImage* glViewer;
  GLImage* projection;
  GLImage* x_zView;
  OverlapWindow* overlapWindow;   // for looking at overlaps.. 
  OverlapEditorWindow* olapEditor;
  PerimeterWindow* perimeterWindow;  // for looking at predictions of perimeters.. 
  SpotMapperWindow* spotMapperWindow;  // for mapping spots to stuff.. 
  
  map<int, color_map> colormap;   // using the indices.. 
  vector<color_map> colorSet;     // an extended set of colours.. 
  int textureSize;
  int texColNo, texRowNo;   // the size and arrangement of textures in the glViewer.. 
  unsigned int currentSliceNo;
  vector<float> scales;
  vector<float> biases;    // how we interpret stuff.. 
  vector<bool> additive;   // additive or subtractive things.. 
  view_area currentView;   // the area that we are currently interestet in.. 
  view_area completeArea;  // the complete area covered by the thingy.. in the stuff.
  
  SpotWindow* spotWindow;
  map<int, SpotsWidget*> spotsWidgets;  // little widgets containing information about 3D spots..
  map<ColorChooser*, vector<Nucleus> > nuclearModels;   // predictions of models.. (so they can be drawn on the thingy).
  map<ParameterChooser*, parameterData> parameterSets;  // some kind of parameters that we want to draw.. 
  bool spotsUseProjection;
  PointViewWidget* modelViewer;   // view models of selected regions.. 
  ClusterWindow* dropClusterWindow;  // clusters of individidual drops.. 
  //  FlatView* flatView;             // a flatttened view of things.. 
  //  PlotWidget* linePlotter;
  //GLImage* y_zView;
  // we'll need to know which slices we are currently holding so that we can update the
  // slice viewers if we change the way the colors are interpreted..
  int xPosition, yPosition;
  
  TabWidget* chooserTabs;
  std::vector<DistChooser*> choosers;  // one for each wavelength..
  std::vector<ChannelOffset> waveOffsets;
  float chooserMaxValue;
  DistChooser* objectSums;
  std::vector<ColorChooser*> colorChoosers;  // although maybe a map would be appropriate for these two.. oh well. think
                                             // about it later.. 
  
  // stuff for dealing with spots that have been found.. 
  TabWidget* spotChooserTabs;
  std::map<int, DistChooser*> spotDistributions;  // update when stuff happens.. 
  
  QTimer* timer;
  bool animate;
  //QSpinBox* magbox;  // setting the magnification.. 
  QCheckBox* updateDist;
  QCheckBox* updateVisible;
  raw_data* raw;       // use this for setting the distributions.. 
  QCheckBox* useComponents;
  float lastMaximumValue;  // for distribution..
  
  // some labels which display information about the image..
  Q3ButtonGroup* channelOffsets;  // sets the channels... 
  QLabel* ch_xoffset;
  QLabel* ch_yoffset;
  QLabel* ch_zoffset;

  QLabel* xOffset;
  QLabel* yOffset;
  QLabel* magnification;
  QLabel* sectionNo;
  QLabel* mouseX;
  QLabel* mouseY;
  QSpinBox* sectionSpin;
  QLabel* proMouseX;
  QLabel* proMouseY;
  
  // and a widget that gives some numbers (connect to show intensities of things)
  ValueLabels* sliceSignals;

  // so we can add stuff..
  QVBoxLayout* colorBox;
  
  // peaks and stuff..
  std::map<int, linearPeaks> peaks;    // where the int represents the channel used in the peak.. 
  //float* completeProjection(unsigned int waveIndex);  // return a float representation of the projection  (0 if not successful).
  
};

#endif
