//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
	return(y);
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    string infoFile = fileName;
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software#include "globalVariables.h"

    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#include "deltaViewer.h"
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qimage.h>
#include <qcolor.h>
#include <qstring.h>
#include <qclipboard.h>
#include <qapplication.h>  // to get the clipboard ?? 
#include <QTextStream>
//#include <qiodevice.h>    // for the io stream modes .. 
#include <q3filedialog.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIODevice>
#include <QRegExp>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <globalVariables.h>
#include <colorChooser.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <q3filedialog.h>
#include <stdlib.h>
#include "button/arrowButton.h"
#include "kcluster/kClusterProcess.h"
#include "tiff/tiffReader.h"    // this is one screwed up class so use with care !! 
#include "dataStructs.h"
#include "stat/stat.h"
#include "image/blobMapper.h"   // probably move this to somewhere else.. 
#include "image/imageData.h"
#include "image/coordConverter.h"
#include "cavity/cavityMapper.h"
#include "imageBuilder/imageBuilderWidget.h"
#include "panels/overlapEditorWindow.h"
#include "customWidgets/valueLabels.h"

////////// temporary for checking what it looks like.. 
#include "cavity/cavityBall.h"
#include "image/background.h"

//#include "pointViewer/pointViewWidget.h"


const int redrawWait = 5;
const int maxMag = 20;
const int minMag = 0;   // not actual magnifications, but just numbers.. 
const float maxLevel = 4096;    // max for a 12 bit unsigned camera.. (but after deconvolution this changes..)


void ChannelOffset::changeOffset(int offsetDirection){
  switch(offsetDirection){
  case XPLUS :
    xo++;
    break;
  case XMINUS :
    xo--;
    xo = xo < 0 ? 0 : xo;
    break;
  case YPLUS :
    yo++;
    break;
  case YMINUS :
    yo--;
    yo = yo < 0 ? 0 : yo;
    break;
  case ZPLUS :
    zo++;
    break;
  case ZMINUS :
    zo--;
    //zo = zo < 0 ? 0 : zo;
    break;
  case XYRESET :
    xo = yo = 0;
    break;
  case ZRESET :
    zo = 0;
    break;
  default :
    cerr << "unknown offset id : " << endl;
  }
}

DeltaViewer::DeltaViewer(map<string, string> opt_commands, int xyMargin, const char* ifName, QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  lastMaximumValue = -1;
  animate = false;
  
  if(!ifName){
      cout << "using a file dialog to get a file name" << endl;
      fName = Q3FileDialog::getOpenFileName();
      if(fName.isNull()){
	  cerr << "No file name given" << endl;
	  exit(1);
      }
      ifName = fName.latin1();
  }else{
      fName = ifName;
  }
  cout << "trying to open file " << ifName << endl;
  reader = new DVReader(ifName, maxLevel, xyMargin);
  reader->printInfo();
  fileSet = reader->fileData();   // then use fileData to do everything.. 
  imageAnalyser = new ImageAnalyser(fileSet);
  // at which point we can make and show the overlapviewer..
  overlapWindow = new OverlapWindow(fileSet->overlaps());
  olapEditor = new OverlapEditorWindow();
  olapEditor->resize(1024, 1024);
  olapEditor->setInfo(fileSet->panelInfo());
  connect(olapEditor, SIGNAL(newFrameSelected(float, float)), 
	  this, SLOT(newStackSelected(float, float)));
  connect(olapEditor, SIGNAL(adjustFramePos(float, float, QPoint)),
	  this, SLOT(adjustStackPosition(float, float, QPoint)));
  connect(olapEditor, SIGNAL(updateFileSetInfo()),
	  this, SLOT(updateFileSetInfo()));
  if(fileSet->overlaps().size())
      overlapWindow->show();
  
  perimeterWindow = new PerimeterWindow(this);
  perimeterWindow->resize(600, 800);


  raw = 0;
  // initialise the view_area to be the complete viewing area.. 
  currentView.setArea(fileSet->xpos(), fileSet->ypos(), fileSet->width(), fileSet->height(), 0, 0, fileSet->pwidth(), fileSet->pheight());
  completeArea.setArea(fileSet->xpos(), fileSet->ypos(), fileSet->width(), fileSet->height(), 0, 0, fileSet->pwidth(), fileSet->pheight());

  float w, h, d;
  reader->dimensions(w, h, d);    // the dimensions of the thingy we are looking at.
  cout << "Dimensions of the image are : " << w << " * " << h << " * " << d << "  um" << endl;


  textureSize = 1024;      // the size of the textures that we are going to be painting to.. 
  texColNo = fileSet->pwidth() / textureSize;   
  texRowNo = fileSet->pheight() / textureSize;
  // Add one if not an even fit.
  texColNo = (fileSet->pwidth() % textureSize) ? 1 + texColNo : texColNo;
  texRowNo = (fileSet->pheight() % textureSize) ? 1 + texRowNo : texRowNo;

  spotMapperWindow = new SpotMapperWindow(this, fileSet->pwidth(), fileSet->pheight(), fileSet->sectionNo(), textureSize);
  spotMapperWindow->resize(1100, 1100);
  //  spotMapperWindow->setFont(widgetFont);
  connect(perimeterWindow, SIGNAL(perimetersFinalised(std::vector<PerimeterSet>, float, int)), spotMapperWindow, SLOT(setPerimeters(std::vector<PerimeterSet>, float, int)) );
  
  glViewer = new GLImage(texColNo, texRowNo, textureSize);   // total of 12 panels.. 
  glViewer->resize(textureSize, textureSize);         // which is just the simple thing to do.. 

  //glViewer->setCaption("Main");
  glViewer->setCaption(ifName);
  connect(glViewer, SIGNAL(nextImage()), this, SLOT(nextImage()) );
  connect(glViewer, SIGNAL(previousImage()), this, SLOT(previousImage()) );
  connect(glViewer, SIGNAL(firstImage()), this, SLOT(firstImage()) );
  connect(glViewer, SIGNAL(lastImage()), this, SLOT(lastImage()) );
  connect(glViewer, SIGNAL(newLine(int, int, int, int)), this, SLOT(newLine(int, int, int, int)) );
  glViewer->show();   // which is very simple interface, nothing in particular there.. 

  projection = new GLImage(texColNo, texRowNo, textureSize);   
  projection->resize(textureSize, textureSize);         // which is just the simple thing to do.. 
  QString p_name(ifName);
  p_name.append(" Projection");
  projection->setCaption(p_name);
  projection->show();

  // a thingy for plotting stuff...
  spotWindow = new SpotWindow(fileSet->pwidth(), fileSet->pheight(), fileSet->sectionNo());
  spotWindow->setFont(font());
  spotsUseProjection = false;

  connect(spotWindow, SIGNAL(findLocalMaxima(int, int, float, float)), this, SLOT(findLocalMaxima(int, int, float, float)) );
  connect(spotWindow, SIGNAL(findAllLocalMaxima(int, int, float, float, int, float, bool)), 
	  this, SLOT(findAllLocalMaxima(int, int, float, float, int, float, bool)) );
  connect(spotWindow, SIGNAL(findAllLocalMaxima3D(int, int, float, float, int, float, bool)), 
	  this, SLOT(findAllLocalMaxima3D(int, int, float, float, int, float, bool)) );
  connect(spotWindow, SIGNAL(increment_x_line(int)), this, SLOT(increment_x_line(int)) );
  connect(spotWindow, SIGNAL(increment_y_line(int)), this, SLOT(increment_y_line(int)) );
  connect(spotWindow, SIGNAL(makeModel(int, int, int, int, int, int, set<int>)),	  
	  this, SLOT(makeModel(int, int, int, int, int, int, set<int>)) );
  connect(spotWindow, SIGNAL(recalculateSpotVolumes()), this, SLOT(recalculateSpotVolumes()) );
  connect(spotWindow, SIGNAL(findNuclearPerimeters(int, float)), this, SLOT(findNuclearPerimeters(int, float)) );
  connect(spotWindow, SIGNAL(findContrasts(int, float)), this, SLOT(findContrasts(int, float)) );
  connect(spotWindow, SIGNAL(mapBlobs(int, float)), this, SLOT(mapBlobs(int, float)) );
  connect(spotWindow, SIGNAL(mapBlobs(int, float, int, int, int, float)),
	  this, SLOT(mapBlobs(int, float, int, int, int, float)) );
  connect(spotWindow, SIGNAL(mapCavities(int, int, int, int, float, float)),
	  this, SLOT(mapCavities(int, int, int, int, float, float)) );
  connect(spotWindow, SIGNAL(findSets(int, int, int, float)), this, SLOT(findSets(int, int, int, float)) );
  connect(spotWindow, SIGNAL(setUseProjection(bool)), this, SLOT(setSpotsUseProjection(bool)) );
  connect(spotWindow, SIGNAL(findSpotDensity(int, double, double)), this, SLOT(determineSpotDensities(int, double, double)) );
  connect(spotWindow, SIGNAL(blur(std::set<uint>, int, double, double)), this, SLOT(blur(std::set<uint>, int, double, double)) );
  //spotWindow->resize(1000, 800);

  // and let's have a window for flat views and stuff..
//  flatView = new FlatView(reader->width(), reader->height());
//  flatView->show();

  // ok now that we have a thing and stuff for finding spots we need to have something that can display spots from different thingys
  // and also a set of channel widgets for this sort of thing as well.. 
  spotChooserTabs = new TabWidget();
  QString tabsName = ifName;
  tabsName.append(" Spots");
  spotChooserTabs->setCaption(tabsName);
  spotChooserTabs->resize(400, 200);


////////////////////////////////////////////////////////commented out for testing purposes.. 
  // make a viewer for displaying x_z slices ...and one for y_z slices...

//   x_zView = new GLImage(reader->width(), reader->sliceNo(), d/w);
//   x_zView->resize(reader->width(), (int)((float)reader->width() * d/w));
//   x_zView->setCaption("X-Z Slice");
//   connect(x_zView, SIGNAL(incrementImage(int)), this, SLOT(increment_x_z_slice(int)) );

///////////////////////////////////////////////////////////////////////

//   y_zView = new GLImage(reader->height(), reader->sliceNo(), d/h);
//   y_zView->resize(reader->height(), (int)((float)reader->height() * d/h));
//   y_zView->setCaption("Y-Z Slice");
//   connect(y_zView, SIGNAL(incrementImage(int)), this, SLOT(increment_y_z_slice(int)) );

  xPosition = yPosition = 0;

  // we'll set these according to the output of the newPos signal from glViewer
  // as by calling the appropriate thingies... ok.. this is a bit scary, but there you
  // go. 
  
  connect(glViewer, SIGNAL(newPos(int, int)), this, SLOT(setSlices(int, int)) );
  connect(projection, SIGNAL(newPos(int, int)), this, SLOT(setSlices(int, int)) );

  // Make a label and a button to display next frame..
  QLabel* label = new QLabel("Delta Viewer", this, "label");
  // Make a label for the file name.. 
  QString fullName(ifName);
  int pathEnd = fullName.findRev('/');
  QString path, fileName;
  if(pathEnd != -1){
      fileName = fullName.right(fullName.length() - pathEnd - 1);
      path = fullName.left(pathEnd);
  }else{
      fileName = fullName;
      path = "";
  }
  QLabel* fileLabel = new QLabel(fileName, this, "fileName");
  QLabel* pathLabel = new QLabel(path, this, "pathLabel");
  setCaption("main");
  //  setCaption(fileName);  // good for using thingy.. 
      

  QPushButton* nextButton = new QPushButton(">", this, "nextButton");
  connect(nextButton, SIGNAL(clicked()), this, SLOT(nextImage()) );

  QPushButton* previousButton = new QPushButton("<", this, "previousButton");
  connect(previousButton, SIGNAL(clicked()), this, SLOT(previousImage()) );

  timer = new QTimer(this, "timer");
  connect(timer, SIGNAL(timeout()), this, SLOT(nextImage()) );

  QPushButton* startAnimation = new QPushButton(">>", this, "startAnimation");
  QPushButton* stopAnimation = new QPushButton("||", this, "stopAnimation");
  connect(startAnimation, SIGNAL(clicked()), this, SLOT(startTimer()));
  connect(stopAnimation, SIGNAL(clicked()), this, SLOT(stopTimer()) );

  nextButton->setMaximumWidth(30);
  previousButton->setMaximumWidth(30);
  startAnimation->setMaximumWidth(30);
  stopAnimation->setMaximumWidth(30);


  // A couple of buttons for showing windows..
  QPushButton* showViewButton = new QPushButton("view", this);
  QPushButton* showProjectionButton = new QPushButton("pro", this);
  connect(showViewButton, SIGNAL(clicked()), glViewer, SLOT(show()) );
  connect(showProjectionButton, SIGNAL(clicked()), projection, SLOT(show()) );
  

  // labels for stuff from image viewer..
  xOffset = new QLabel("0", this, "xOffset");
  yOffset = new QLabel("0", this, "yOffset");
  xOffset->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  yOffset->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  QLabel* offsetsLabel = new QLabel("Offsets", this, "offsetsLabel");
  offsetsLabel->setAlignment(Qt::AlignVCenter);
  QPushButton* resetOffsetButton = new QPushButton("R", this, "resetOffsetButton");
  connect(resetOffsetButton, SIGNAL(clicked()), glViewer, SLOT(resetOffsets()) );

  magnification = new QLabel("1.0", this, "magnification");
  magnification->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  QLabel* magnificationLabel = new QLabel("Magnification", this, "magnificationLabel");
  magnificationLabel->setAlignment(Qt::AlignVCenter);
  QPushButton* resetMagButton = new QPushButton("R", this, "resetMagButton");
  connect(resetMagButton, SIGNAL(clicked()), glViewer, SLOT(resetMagnification()) );

  // a label to show the current section no..
  sectionSpin = new QSpinBox(0, fileSet->sectionNo(), 1, this, "sectionSpin");
  sectionSpin->setWrapping(true);
  connect(sectionSpin, SIGNAL(valueChanged(int)), this, SLOT(setImage(int)) );
  sectionNo = new QLabel("Section", this, "sectionNo");
  sectionNo->setAlignment(Qt::DockLeft|Qt::DockBottom);

  // labels for the mouse position
  mouseX = new QLabel("----", this);
  mouseY = new QLabel("----", this);
  mouseX->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  mouseY->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

  proMouseX = new QLabel("----", this);
  proMouseY = new QLabel("----", this);
  proMouseX->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  proMouseY->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

  connect(glViewer, SIGNAL(mousePos(int, int)), this, SLOT(newMousePosition(int, int)) );
  connect(projection, SIGNAL(mousePos(int, int)), this, SLOT(newProMousePosition(int, int)) );
  // and set up the connections..
  connect(glViewer, SIGNAL(offSetsSet(int, int)), this, SLOT(newOffsets(int, int)) );
  connect(glViewer, SIGNAL(magnificationSet(float)), this, SLOT(newMagnification(float)) );

  updateDist = new QCheckBox("Update Dist", this, "updateDist");
  connect(updateDist, SIGNAL(toggled(bool)), this, SLOT(updateDistToggled(bool)) );

  // Use components hijacked for background subtraction.. 
  useComponents = new QCheckBox("Background subtract", this);
  connect(useComponents, SIGNAL(toggled(bool)), this, SLOT(useComponentsToggled(bool)) );

  objectSums = new DistChooser("Object Intensities", 100);
  objectSums->resize(300, 200);

  // make a tabwidget to hold the choosers..
  chooserTabs = new TabWidget();  // don't show.. 
  connect(chooserTabs, SIGNAL(copyRanges()), this, SLOT(copyRanges()) );
  connect(chooserTabs, SIGNAL(saveRanges()), this, SLOT(saveRanges()) );
  connect(chooserTabs, SIGNAL(pasteRanges()), this, SLOT(pasteRanges()) );
  connect(chooserTabs, SIGNAL(readRangesFromFile()), this, SLOT(readRangesFromFile()) );
  chooserTabs->resize(400, 200);
  tabsName = ifName;
  tabsName.append(" Channels");
  chooserTabs->setCaption(tabsName);
  chooserMaxValue = 1.0; // defaults to 1.0.
  
  // ok, a little magic to chose appropriate colours. basically try to make the bluest colours for
  // the shortest wawelength..

  /// a vector of color_map that is maintained
  colorSet.push_back(color_map(0, 0, 1.0));
  colorSet.push_back(color_map(0, 1.0, 0));
  colorSet.push_back(color_map(1.0, 0, 0));
  colorSet.push_back(color_map(0, 1.0, 1.0));
  colorSet.push_back(color_map(1.0, 0, 1.0));
  colorSet.push_back(color_map(1.0, 0, 0.0));
  colorSet.push_back(color_map(0.3, 0.5, 0.8));
  colorSet.push_back(color_map(0.56, 1.0, 0.9));
  colorSet.push_back(color_map(0.88, 0.72, 1.0));
  colorSet.push_back(color_map(1.0, 0.3, 0.88));
  colorSet.push_back(color_map(1.0, 0.66, 0.0));  

  /// override if -c specified by the user
  /// which might do strange things with strange files.
  if(opt_commands.count("colorFile"))
    colorSet = readColors(opt_commands["colorFile"]);
  
  for(uint i=0; i < fileSet->channelNo(); i++){
    colormap.insert(make_pair(fileSet->channel(i), colorSet[i % colorSet.size()]));  // the default..
  }

  channelOffsets = new Q3ButtonGroup(1, Qt::Horizontal, "channel", this);
  // make one DistChooser for each channel ..
  vector<QColor> channelColors;
  for(unsigned int i=0; i < fileSet->channelNo(); i++){
    ostringstream ss;
    ostringstream s2;
    fluorInfo finfo = fileSet->channelInfo(i);
    ss << fileSet->channel(i);
    s2 << fileSet->channel(i); 

    QRadioButton* rbutton = new QRadioButton(ss.str().c_str(), channelOffsets);    // ??? 

    waveOffsets.push_back(ChannelOffset(fileSet->channel(i)));
    choosers.push_back(new DistChooser(s2.str().c_str(), 100));
    s2 << " (&" << (char)(97 + i % 25) << ")";
    chooserTabs->addTab(choosers.back(), s2.str().c_str());

    connect(choosers[i], SIGNAL(newRanges(float, float)), this, SLOT(changeRanges(float, float)) );
    ostringstream ls;
    ls << finfo.excitation << " -> " << finfo.emission << " : " << finfo.exposure;
    colorChoosers.push_back(new ColorChooser(ls.str().c_str(), i, fileSet->channel(i), colormap[fileSet->channel(i)].qcolor(), this));
    colorChoosers[i]->setContentsMargins(1, 1, 1, 1);
    connect(colorChoosers[i], SIGNAL(colorChanged(int, float, float, float)), this, SLOT(setWaveColors(int, float, float, float)) );
    connect(colorChoosers[i], SIGNAL(checkSubtractions(bool)), this, SLOT(setAdditive(bool)) );

    scales.push_back(1.0);
    biases.push_back(0.0);  // the simplest way.. 
    channelColors.push_back(colormap[fileSet->channel(i)].qcolor());
  }
  setWaveColors();
  sliceSignals = new ValueLabels(channelColors, this);

  QLabel* exportLabel = new QLabel("Export", this);
  QPushButton* mergeChannelButton = new QPushButton("Projection", this, "mergeChannelButton");
  connect(mergeChannelButton, SIGNAL(clicked()), this, SLOT(exportProjection()) );

  QPushButton* exportSpotsButton = new QPushButton("Spots", this);
  connect(exportSpotsButton, SIGNAL(clicked()), this, SLOT(exportPeaks()) );

  QPushButton* adjustOverlapsButton = new QPushButton("Adjust Overlaps", this);
  connect(adjustOverlapsButton, SIGNAL(clicked()), olapEditor, SLOT(show()) );

  // input individual offsets... 
  
  Q3ButtonGroup* offSetButtons = new Q3ButtonGroup();
  connect(offSetButtons, SIGNAL(clicked(int)), this, SLOT(offSetChanged(int)) );
  ArrowButton* x_plus = new ArrowButton(90, this);
  offSetButtons->insert(x_plus, XMINUS);
  ArrowButton* x_minus = new ArrowButton(270, this);
  offSetButtons->insert(x_minus, XPLUS);
  ArrowButton* y_plus = new ArrowButton(0, this);
  offSetButtons->insert(y_plus, YMINUS);
  ArrowButton* y_minus = new ArrowButton(180, this);
  offSetButtons->insert(y_minus, YPLUS);
  ArrowButton* z_plus = new ArrowButton(0, this);
  offSetButtons->insert(z_plus, ZMINUS);
  ArrowButton* z_minus = new ArrowButton(180, this);
  offSetButtons->insert(z_minus, ZPLUS);
  // 
  QPushButton* x_y_resetButton = new QPushButton("x/y", this);
  offSetButtons->insert(x_y_resetButton, XYRESET);
  QPushButton* z_resetButton = new QPushButton("z", this);
  offSetButtons->insert(z_resetButton, ZRESET);
  
  x_y_resetButton->setFixedSize(35, 35);
  z_resetButton->setFixedSize(35, 35);

  // add a thingy for viewing spots ..
  modelViewer = new PointViewWidget();
  modelViewer->generateNullModel(20);
  modelViewer->resize(512, 512);
//  modelViewer->show();
  dropClusterWindow = new ClusterWindow();
  dropClusterWindow->resize(1024, 400);

//  connect(dropClusterWindow, SIGNAL(drawClusters(vector<Cluster>&)), flatView, SLOT(setClusterDrops(vector<Cluster>&)) );
  // and lets see what happens.. 

  ///////////  Initialise blobManager to 0, then if we need it make it and 
  blobManager=0;
  backgroundWindow = 0;

  // Make an imagebuilderwidget and stick it at the bottom..


  imageBuilder = new ImageBuilderWidget(fileSet, collect_channel_info());
  imageBuilder->resize(500, 500);
  connect(imageBuilder, SIGNAL(showCoverage(float)), 
	  this, SLOT(paintCoverage(float)) );
  connect(imageBuilder, SIGNAL(setChooserMax(float)), this, SLOT(setChooserMaxValue(float)) );
  connect(imageBuilder, SIGNAL(getBlobModel(int, int)), this, SLOT(makeBlobModel(int, int)));

  QPushButton* imageBuilderButton = new QPushButton("image builder", this);
  connect(imageBuilderButton, SIGNAL(clicked()), imageBuilder, SLOT(show()) );

	  
  QVBoxLayout* box = new QVBoxLayout(this);
  box->setSpacing(1);
  box->setMargin(1);
  box->addWidget(label);
  box->addWidget(fileLabel);
  box->addWidget(pathLabel);
  QHBoxLayout* playButtonBox = new QHBoxLayout();
  box->addLayout(playButtonBox);

  playButtonBox->addStretch();
  playButtonBox->addWidget(previousButton);
  playButtonBox->addWidget(stopAnimation);
  playButtonBox->addWidget(nextButton);
  playButtonBox->addWidget(startAnimation);

  QGridLayout* sectionBox = new QGridLayout(1, 4);
  box->addLayout(sectionBox);
  sectionBox->addWidget(sectionNo, 0, 0, 1, 1, Qt::AlignBottom|Qt::AlignLeft);
  sectionBox->addWidget(sectionSpin, 0, 3);
  sectionBox->setColumnStretch(0, 1);

  QHBoxLayout* positionBox = new QHBoxLayout();
  box->addLayout(positionBox);
  QLabel* mouseLabel = new QLabel("Position");
  positionBox->addWidget(mouseLabel, Qt::AlignBottom|Qt::AlignLeft);
  positionBox->addStretch();
  positionBox->addWidget(mouseX);
  positionBox->addWidget(mouseY);
  positionBox->addStretch();
  positionBox->addWidget(proMouseX);
  positionBox->addWidget(proMouseY);
  int minWidth = mouseX->sizeHint().width() * 2;
  mouseX->setMinimumWidth(minWidth);
  mouseY->setMinimumWidth(minWidth);
  proMouseX->setMinimumWidth(minWidth);
  proMouseY->setMinimumWidth(minWidth);

  box->addWidget(sliceSignals);

  QHBoxLayout* showBox = new QHBoxLayout();
  box->addLayout(showBox);
  showBox->addStretch();
  showBox->addWidget(showViewButton);
  showBox->addWidget(showProjectionButton);

  QGridLayout* imageInfoGrid = new QGridLayout(box, 2, 4);
  imageInfoGrid->addWidget(offsetsLabel, 0, 0);
  imageInfoGrid->addWidget(xOffset, 0, 1);
  imageInfoGrid->addWidget(yOffset, 0,2);
  imageInfoGrid->addWidget(resetOffsetButton, 0, 3);
  imageInfoGrid->addWidget(magnificationLabel, 1, 0);
  imageInfoGrid->addMultiCellWidget(magnification, 1, 1, 1, 2);
  imageInfoGrid->addWidget(resetMagButton, 1, 3);
  // might want to set stretch factors and stuff, but migth work fine.. 

  colorBox = new QVBoxLayout(box);
  colorBox->setSpacing(1);
  colorBox->setContentsMargins(1, 1, 1, 1);
  for(uint i=0; i < colorChoosers.size(); i++){
    colorBox->addWidget(colorChoosers[i]);
  }
  QHBoxLayout* exportBox = new QHBoxLayout();
  exportBox->setSpacing(0);
  exportBox->setMargin(0);
  box->addLayout(exportBox);
  exportBox->addWidget(exportLabel);
  exportBox->addWidget(mergeChannelButton);
  exportBox->addWidget(exportSpotsButton);
  QHBoxLayout* olapBox = new QHBoxLayout();
  box->addLayout(olapBox);
  olapBox->addStretch();
  adjustOverlapsButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  olapBox->addWidget(adjustOverlapsButton, Qt::AlignRight);
  QGridLayout* offSetGrid = new QGridLayout(4, 5);
  QHBoxLayout* offSetBox = new QHBoxLayout();
  box->addLayout(offSetBox);
  offSetBox->addLayout(offSetGrid);
  offSetGrid->addWidget(y_plus, 0, 1);
  offSetGrid->addWidget(x_minus, 1, 0);
  offSetGrid->addWidget(x_plus, 1, 2);
  offSetGrid->addWidget(y_minus, 2, 1);
  offSetGrid->addWidget(z_plus, 0, 4);
  offSetGrid->addWidget(z_minus, 2, 4);
  offSetGrid->addWidget(x_y_resetButton, 1, 1);
  offSetGrid->addWidget(z_resetButton, 1, 4);
  offSetGrid->setRowStretch(3, 3);
  offSetBox->addWidget(channelOffsets);
  box->addWidget(updateDist);
  box->addWidget(useComponents);

  box->addWidget(imageBuilderButton);

  box->addStretch();
  
  // and lets set the projection..
  setProjection();

  // And if we have any optional commands. then do those and exit (this allows us to script the thing)
  if(opt_commands.size()){
      if(opt_commands.count("find_spots")){
	  cout << "This program has been requested to count spots using parameters in file " << opt_commands["find_spots"] << endl;
	  find_spots(opt_commands["find_spots"]);
      }
      if(opt_commands.count("find_blobs"))
	  findBlobs(opt_commands["find_blobs"]);
      if(opt_commands.count("die") && opt_commands["die"] == "yes")
	exit(0);
      if(opt_commands.count("rangeFile"))
	readRangesFromFile(QString(opt_commands["rangeFile"].c_str()));
  }

  currentSliceNo = fileSet->sectionNo() / 2;
  setImage(currentSliceNo);

  glViewer->show();
  projection->show();

  //  Temporary, remove along with includes of background and cavityBall
  // ImageData* idata = new ImageData(fileSet, 1);
  // map<o_set, CavityBall*> cavityMap; 
  // CavityBall* cBall = new CavityBall(idata, new Background(idata, 8, 8, 4, 20), cavityMap,
  // 				     (unsigned int)1, 8, 8, 6, 632, 322, 21,
  // 				     0.08, 100.0);
  // aux_points = cBall->points();
}

void DeltaViewer::offSetChanged(int id){
    int n = channelOffsets->selectedId();
    if(n == -1){
	return;
    }
    cout << "offSetsChanged n is : " << n << endl;
    if((uint)n >= waveOffsets.size()){
	cerr << "offSetChanged n is too large .. " << endl;
	return;
    }
    waveOffsets[n].changeOffset(id);
    cout << "offsetf for : " << waveOffsets[n].w() << " : " << waveOffsets[n].x() << "\t" << waveOffsets[n].y() << "\t" << waveOffsets[n].z() << endl;

//    reader->setOffsets(n, waveOffsets[n].x(), waveOffsets[n].y(), waveOffsets[n].z());
//    reader->setImageData();
//    displayImage(); // slices ?  not yet.. 
//    displaySlices();

}

void DeltaViewer::setWaveColors(){
    for(uint i=0; i < colorChoosers.size(); i++){
	float r, g, b;
	int wl = colorChoosers[i]->wlength();
	colorChoosers[i]->color(&r, &g, &b);
	colormap.insert(make_pair(wl, color_map(r, g, b)));
    }
    // but don't update image.. as this is used mostly only for the init..
    setLinePlotterColors();
    paintProjection();
}

void DeltaViewer::setWaveColors(int wi, float r, float g, float b){
    int wl = colorChoosers[wi]->wlength();
    colormap[wl] = color_map(r, g, b);
    sliceSignals->setColor((unsigned int)wi, QColor( (int)(r * 255), (int)(g * 255), int(b * 255) ) );
    displayImage();
    displaySlices();
    setLinePlotterColors();
    paintProjection();
}

void DeltaViewer::setLinePlotterColors(){
    vector<QColor> cols;
    vector<QString> channels;
    for(uint i=0; i < colorChoosers.size(); i++){
	cols.push_back(colorChoosers[i]->color());
	channels.push_back(colorChoosers[i]->displayLabel());
    }
    spotWindow->setLineColors(cols);
    spotWindow->setChannels(channels);
}


void DeltaViewer::addMergedChannel(){
    // for this we need to tell the reader to merge channels, 
    // set up a new color chooser, and a new dist chooser so that
    // we can manipulate the colours and things..
    int* mch = new int[colorChoosers.size()];
    int m = 0;
    QString labelString;
    QString numString;
    for(uint i=0; i < colorChoosers.size(); i++){
	if(colorChoosers[i]->includeInMerger()){
	    mch[m] = i;
	    m++;
	    numString.setNum(colorChoosers[i]->wlength());
	    if(m > 1){
		labelString.append(" * ");
	    }
	    labelString.append(numString);
	}
    }
    if(m < 2){
	cerr << "no sense in merging less than two channels.. bybye" << endl;
    }

    colorChoosers.push_back(new ColorChooser(labelString, colorChoosers.size(), 0, QColor(255, 255, 255), this));
    colorBox->addWidget(colorChoosers.back());
    colorChoosers.back()->show();   // which will put it in the wrong place fix later.. 
    connect(colorChoosers.back(), SIGNAL(colorChanged(int, float, float, float)), this, SLOT(setWaveColors(int, float, float, float)) );
    connect(colorChoosers.back(), SIGNAL(checkSubtractions(bool)), this, SLOT(setAdditive(bool)) );

    int n = choosers.size();
    char c = (char)(97 + n % 25);
    ostringstream ss;
    ss << labelString.latin1();
    
    choosers.push_back(new DistChooser(ss.str().c_str(), 100));
    ss << " (&" << c << ")";
    chooserTabs->addTab(choosers.back(), ss.str().c_str());
    connect(choosers.back(), SIGNAL(newRanges(float, float)), this, SLOT(changeRanges(float, float)) );
    displayImage();   // hmmmm... this is bound to give us some problem.. eh. 
    setLinePlotterColors();
}

void DeltaViewer::nextImage(){
    setImage(++currentSliceNo);
}

void DeltaViewer::previousImage(){
    setImage(--currentSliceNo);
}

void DeltaViewer::firstImage(){
  setImage(0);
}

void DeltaViewer::lastImage(){
  setImage(fileSet->sectionNo() - 1);
}

void DeltaViewer::setImage(int slice){
    if(slice >= fileSet->sectionNo())
	slice = 0;
    if(slice < 0)
      slice = fileSet->sectionNo() - 1;

    currentSliceNo = slice;

    sectionSpin->blockSignals(true);
    sectionSpin->setValue(slice);
    sectionSpin->blockSignals(false);

    // This depends on the order of the colorChoosers and hence is not particularly a good thing
    vector<channel_info> chinfo;
    for(uint i=0; i < colorChoosers.size(); ++i){
      if(i >= scales.size() || i >= biases.size()){
	cerr << "DeltaViewer::setImage colorChoosers.size is larger than scales.size or biases.size()" << endl;
	exit(1);
      }
      float r, g, b;
      colorChoosers[i]->color(&r, &g, &b);
      chinfo.push_back(channel_info( color_map(r, g, b), maxLevel, biases[i], scales[i],
				     colorChoosers[i]->includeInMerger(), colorChoosers[i]->subtractColor()) );
    }

    // but more importantly do something to actually update the image.. 
    float* data = new float[textureSize * textureSize * 3];

    if(raw){
	delete raw;
    }
    raw = 0;


    // This necessary since overlapping panels can give too many numbers..
    int col_no, row_no, panelWidth, panelHeight;
    fileSet->stackDimensions(col_no, row_no, panelWidth, panelHeight);
    if(updateDist->isOn())
      raw = new raw_data(fileSet->channelNo(), col_no * row_no * panelWidth * panelHeight);
    //    if(updateDist->isOn())
    //	raw = new raw_data(fileSet->channelNo(), currentView.pw * currentView.ph);
    

    int rowCounter = 0;
    int colCounter = 0;
    int textureCounter = 0;
    for(int yb = currentView.py; yb < currentView.py + currentView.ph; yb += textureSize){
	colCounter = 0;
	int th = yb + textureSize < currentView.py + currentView.ph ? textureSize : (currentView.py + currentView.ph) - yb;

	for(int xb = currentView.px; xb < currentView.px + currentView.pw; xb += textureSize){
	    int tw = xb + textureSize < currentView.px + currentView.pw ? textureSize : (currentView.px + currentView.pw) - xb;
	    memset((void*)data, 0, textureSize * textureSize * 3 * sizeof(float));
	
	    if(fileSet->readToRGB(data, xb, yb, tw, th, currentSliceNo, chinfo, raw)){
		textureCounter++;
		paintBlobs(data, xb, yb, currentSliceNo, tw, th);
		paint(data, aux_points, Rectangle(xb, yb, tw, th), currentSliceNo, color_map(0.1, 0.1, 0.1) );
		glViewer->setImage(data, tw, th, colCounter, rowCounter);
	    }
	    colCounter++;
	}
	rowCounter++;
    }
    delete []data;
    glViewer->updateGL();

    if(raw){
	calculateDistribution();
    }
    
    // and update the lines if we have set them.. 
    if(xPosition || yPosition){
	plotLines();
    }
}

void DeltaViewer::paintCoverage(float maxCount){
  int w, h;
  float* image = fileSet->paintCoverage(w, h, maxCount);
  imageBuilder->setImage(image, w, h);
}

bool DeltaViewer::readToRGB(float* dest, int xb, int yb, unsigned int tw, unsigned int th, unsigned int slice_no){
    // this is a bit redundant with the above function and we should probably rewrite the above function to use this 
    // one..
    // This function is called from PerimeterWindow ??  
    if(slice_no >= (uint)fileSet->sectionNo()){
	return(false);
    }
    // this is damn ugly..
    vector<channel_info> chinfo;
    for(uint i=0; i < colorChoosers.size(); ++i){
      if(i >= scales.size() || i >= biases.size()){
	cerr << "DeltaViewer::setImage colorChoosers.size is larger than scales.size or biases.size()" << endl;
	exit(1);
      }
      float r, g, b;
      colorChoosers[i]->color(&r, &g, &b);
      chinfo.push_back(channel_info( color_map(r, g, b), maxLevel, biases[i], scales[i],
				     useComponents->isChecked(), colorChoosers[i]->subtractColor()) );
    }
    return(fileSet->readToRGB(dest, (uint)xb, (uint)yb, tw, th, slice_no, chinfo, 0) );
}

bool DeltaViewer::readToRGBPro(float* dest, int xb, int yb, unsigned int tw, unsigned int th){
    vector<color_map> cv;
    for(map<int, color_map>::iterator it = colormap.begin(); it != colormap.end(); it++){
	cv.push_back((*it).second);
    }
    return(fileSet->mip_projection(dest, xb, yb, tw, th, maxLevel, biases, scales, cv, 0));
}
	   
void DeltaViewer::int_color(int i, float& r, float& g, float& b){
  int max = 100;
  int m = 40;
  r = float( (i * m) % max ) / float(max);
  g = float( (i * m + 50) % max ) / float(max);
  b = float( (i * m + 75) % max ) / float(max);
}

void DeltaViewer::setProjection(){
    // first check if we have a projection file..

    //int margin = xyMargin;
    int margin = 0;

    int w = completeArea.pw - margin * 2;
    int h = completeArea.ph - margin * 2;

    float* fpro = new float[w * h];
    projection_means.resize(fileSet->channelNo());
    projection_stds.resize(fileSet->channelNo());
    for(int i=0; i < fileSet->channelNo(); ++i){
//	int wl = fileSet->channel((uint)i);
	memset((void*)fpro, 0, sizeof(float) * w * h);
	if(!fileSet->readToFloatPro(fpro, margin, w, margin, h, i)){
	    cerr << "Set projection unable to get floats for means and stds: " << endl;
	    projection_means[i] = 0;
	    projection_stds[i] = 1.0;
	    continue;
	}
	if(!mean_std(projection_means[i], projection_stds[i], fpro, w * h)){
	    projection_means[i] = 0;
	    projection_stds[i] = 1.0;
	    cerr << "Unable to get mean and standard deviation for projection values" << endl;
	}
    }
    delete []fpro;
    for(uint i=0; i < projection_means.size(); ++i){
	cout << "Channel : " << fileSet->channel(i) << "\t mean : " << projection_means[i] << "\tstd : " << projection_stds[i] << endl;
    }
    


    float maxLevel = 4096;   // fix this at some point..
    vector<color_map> cv;
    for(map<int, color_map>::iterator it = colormap.begin(); it != colormap.end(); it++){
      //	cout << "color map wave length = " << (*it).first << "  color " << (*it).second.r << "," << (*it).second.g << "," << (*it).second.b << endl;
	cv.push_back((*it).second);
    }

    // delete any known projection data..
    for(uint i=0; i < projection_data.size(); i++){
	delete projection_data[i];
    }
    projection_data.resize(0);

    float* data = new float[textureSize * textureSize * 3];
    cout << "data size is " << textureSize * textureSize * 3 << endl;
    int rowCounter = 0;
    int colCounter = 0;
    int textureCounter = 0;
    for(int yb = currentView.py; yb < currentView.py + currentView.ph; yb += textureSize){
	colCounter = 0;
	int th = yb + textureSize < currentView.py + currentView.ph ? textureSize : (currentView.py + currentView.ph) - yb;
	// which is a bit of an ugly way of putting it..
	float yf = fileSet->ypos() + float(yb) * currentView.yscale;
	float hf = th * currentView.yscale;
	for(int xb = currentView.px; xb < currentView.px + currentView.pw; xb += textureSize){
	    projection_data.push_back(new raw_data(fileSet->channelNo(), textureSize * textureSize));
	    int tw = xb + textureSize < currentView.px + currentView.pw ? textureSize : (currentView.px + currentView.pw) - xb;
	    // at this point...
	    // 1. convert the coordinates to float coordinates, since this is more convenient for handling file stacks
	    // 2. get the appropriate data from fileSet
	    // 3. set image in glviewer (specifying which thingy and so on.. 
	    //float xf = fileSet->xpos() + float(xb) * currentView.xscale;
	    //float wf = tw * currentView.xscale;
	    cout << "making projection for : " << colCounter << "," << rowCounter << " requesting " 
		 << "  pixels : " << xb << "," << yb << "   " << tw << "," << th << endl; 
	    memset((void*)data, 0, textureSize * textureSize * 3 * sizeof(float));
	    
	    if(fileSet->mip_projection(data, xb, yb, textureSize, textureSize, maxLevel, biases, scales, cv, projection_data.back())){
		textureCounter++;
		// and in this case let's copy data to the appropriate texture..
		projection->setImage(data, textureSize, textureSize, colCounter, rowCounter);
	    }
	    colCounter++;
	}
	rowCounter++;
    }
    delete []data;
//    if(projection->isVisible())
    projection->updateGL();

}

// // WARNING WARNING WARNING
// both the paintProjection() and completeProjection functions make assumptions about the structure of the raw_data structs making up the
// projection_data .. This is bad and dangerous and makes the code difficult to maintain.. but never mind.. 
//
// In particular the assumption is made that raw_data bits in the projection data are made up of squares with length and width of textureSize
// 
/////
void DeltaViewer::paintProjection(){
    if(!projection_data.size()){
	cerr << "paintProjection no raw data to work with" << endl;
	return;
    }
    
    vector<color_map> cv;
    for(map<int, color_map>::iterator it = colormap.begin(); it != colormap.end(); it++){
      //	cout << "color map wave length = " << (*it).first << "  color " << (*it).second.r << "," << (*it).second.g << "," << (*it).second.b << endl;
	cv.push_back((*it).second);
    }

    float* data = new float[textureSize * textureSize * 3];
    float* dptr;
    int rowCounter = 0;
    int colCounter = 0;

//    int textureCounter = 0;

    for(int yb = completeArea.py; yb < completeArea.py + completeArea.ph; yb += textureSize){
	colCounter = 0;

//	int th = yb + textureSize < completeArea.py + completeArea.ph ? textureSize : (completeArea.py + completeArea.ph) - yb;

	for(int xb = completeArea.px; xb < completeArea.px + completeArea.pw; xb += textureSize){
	  memset((void*)data, 0, sizeof(float) * 3 * textureSize * textureSize);
	  fileSet->mip_projection(data, xb, yb, textureSize, textureSize, maxLevel, biases, scales, cv);
	    // check that width is not too big.. 

//	    int tw = xb + textureSize < completeArea.px + completeArea.pw ? textureSize : (completeArea.px + completeArea.pw) - xb;

	    // at this point...
	    // 1. convert the coordinates to float coordinates, since this is more convenient for handling file stacks
	    // 2. get the appropriate data from fileSet
	    // 3. set image in glviewer (specifying which thingy and so on.. 

	    // memset((void*)data, 0, textureSize * textureSize * 3 * sizeof(float));
	    // for(int y=0; y < textureSize; y++){
	    // 	for(int x=0; x < textureSize; x++){
	    // 	    for(uint w=0; w < scales.size(); w++){   // the wavelenght index..
	    // 		float v = biases[w] + scales[w] * projection_data[rowCounter * texColNo + colCounter] ->values[w][y * textureSize + x];
	    // 		if(v > 0){
	    // 		    dptr = data + 3 * (y * textureSize + x);
	    // 		    dptr[0] += v * cv[w].r;
	    // 		    dptr[1] += v * cv[w].g;
	    // 		    dptr[2] += v * cv[w].b;
	    // 		}
	    // 	    }
	    // 	}
	    // }

	    paintPeaks(data, xb, yb, textureSize, textureSize);
	    paintBlobs(data, xb, yb, 0, textureSize, textureSize, true);
	    paintParameterData(data, xb, yb, textureSize, textureSize);
	    paintNuclei(data, xb, yb, textureSize, textureSize);
	    projection->setImage(data, textureSize, textureSize, colCounter, rowCounter);
	    colCounter++;
	}
	rowCounter++;
    }
    delete []data;
    projection->updateGL();
}

void DeltaViewer::exportProjection(){
    float* prdata = new float[completeArea.pw * completeArea.ph * 3];
    memset((void*)prdata, 0, sizeof(float) * completeArea.pw * completeArea.ph * 3);

    vector<color_map> cv;
    for(map<int, color_map>::iterator it = colormap.begin(); it != colormap.end(); it++){
      //	cout << "color map wave length = " << (*it).first << "  color " << (*it).second.r << "," << (*it).second.g << "," << (*it).second.b << endl;
	cv.push_back((*it).second);
    }
    
    if(!fileSet->mip_projection(prdata, 0, 0, completeArea.pw, completeArea.ph, maxLevel, biases, scales, cv) ){
	cerr << "DeltaViewer::exportProjection fileSet->mip_projection returned false giving up " << endl;
	delete prdata;
	return;
    }
    // and now do the other funky things..
    paintPeaks(prdata, 0, 0, completeArea.pw, completeArea.ph);
    paintParameterData(prdata, 0, 0, completeArea.pw, completeArea.ph);

    // and then somehow we need to turn this into an exportable file. I don't actually know of any particularly easy way of doing
    // this, though I have a library function somewhere.. 
    TiffReader tr;
    if(!tr.makeFromRGBFloat(prdata, completeArea.pw, completeArea.ph))
	cerr << "oh bugger, the tiff reader didn't much like me" << endl;
    tr.writeOut("projection.tif");
    // finally ..
    delete prdata;
}

QColor DeltaViewer::wiColor(int wi){
  QColor c(255, 255, 255);
  for(uint i=0; i < colorChoosers.size(); ++i){
    if(colorChoosers[i]->wIndex() == wi)
      return(colorChoosers[i]->color());
  }
  return(c);
}

// WARNING.. see above WARNING for paintProjection()
float* DeltaViewer::completeProjection(unsigned int waveIndex){
    if(waveIndex >= scales.size()){
	cerr << "DeltaViewer::completeProjection waveIndex is too large" << endl;
	return(0);
    }
    if(!projection_data.size()){
	cerr << "DeltaViewer::completeProjection no raw data to work with" << endl;
	return(0);
    }
    // assign the float area..
    float* pdata = new float[completeArea.pw * completeArea.ph];
    int rowCounter = 0;
    int colCounter = 0;

//    int textureCounter = 0;

    for(int yb = completeArea.py; yb < completeArea.py + completeArea.ph; yb += textureSize){
	colCounter = 0;
	int th = yb + textureSize < completeArea.py + completeArea.ph ? textureSize : (completeArea.py + completeArea.ph) - yb;
	for(int xb = completeArea.px; xb < completeArea.px + completeArea.pw; xb += textureSize){
	    int tw = xb + textureSize < completeArea.px + completeArea.pw ? textureSize : (completeArea.px + completeArea.pw) - xb;
	    // now copy one line at a time from projection_data[rowCounter * texColNo + colCounter]->values
	    // to the appropriate area of p_data...
	    for(int y=0; y < th; ++y){
		memcpy(pdata + (yb + y) * completeArea.pw + xb, projection_data[rowCounter * texColNo + colCounter]->values[waveIndex] + y * textureSize, tw * sizeof(float));
	    }
	    ++colCounter;
	}
	++rowCounter;
    }
    return(pdata);
}

void DeltaViewer::peakColorsChanged(){
    paintProjection();
}

void DeltaViewer::nuclearColorsChanged(){
    paintProjection();
}

void DeltaViewer::paramDataRangeChanged(){
    paintProjection();
}

void DeltaViewer::paramDataColorChanged(){
    paintProjection();
}

void DeltaViewer::paramDataRangeChanged(float lowt, float hight){
    paintProjection();
}

void DeltaViewer::paramDataColorChanged(int wl, float r, float g, float b){
    paintProjection();
}

void DeltaViewer::peakRepTypeChanged(){
    paintProjection();
}

void DeltaViewer::paintPeaks(float* area, int px, int py, int w, int h){
    // positions in the peaks point to global coordinates, so we need to know these as well..
    // we also need to know some useful parameters for painting the values.. 
    // which given the mess that we've made of various parameters and so on a little bit difficult..

//    int total_width = fileSet->pwidth();
//    int total_height = fileSet->pheight();
    cout << "painting Peaks .. " << endl;

    map<int, SpotsWidget*>::iterator it;
    for(it = spotsWidgets.begin(); it != spotsWidgets.end(); it++){
	// first check if this is a NO_REP type,
	SpotsWidget* sw = (*it).second;
	if(sw->dropType() == NO_REP){
	    cout << "Drop type is NO_REP so ignoring" << endl;
	    continue;
	}
	// then work out the bias, scale and colors that I want to use..
	int wi = sw->w_index();
	if(wi < 0 || wi >= fileSet->channelNo()){
	    cerr << "DeltaViewer::paintPeaks wave Index : " << wi << " is rather too large for the size of biases" << endl;
	    continue;
	}
	// then assign the interpretation variables.. that we'll use for the picture..
	float r, g, b, bias, scale;
	sw->color(r, g, b);
	bias = biases[wi];
	scale = scales[wi];   // this is so ugly we should have a struct for the interpretation variables (channelInfo or something)
	
	// The we just go through all the spots and modify the area in the appropriate manner...
	threeDPeaks* peaksInfo = sw->peaks();
	// and then..
	for(map<long, simple_drop>::iterator dit=peaksInfo->simpleDrops.begin(); dit != peaksInfo->simpleDrops.end(); dit++){
	    // and then we can do stuff with it.. ... 
	    simple_drop* drop = &(*dit).second;    // easier to write drop-> than (*it).second.
	    if(sw->dropType() == DOT){
		if(drop->id >= 0){
		    colorSet[drop->id % colorSet.size()].set_color(r, g, b);
		}else{
		    r = g = b = 1.0;
		}
	    }

	    // first determine if the drops position is within the current area ..
	    // Ideally we should determine overlap.. rather than within.. so..
	    if(drop->x - drop->radius >= px && drop->x + drop->radius <= px + w
	       &&
	       drop->y - drop->radius >= py && drop->y + drop->radius <= py + h)
	    {
		// then go through the central plane of the drop and assign the locations.. 
		int radius = drop->radius;
		int d = radius * 2 + 1;
		for(int dy=-drop->radius; dy <= drop->radius; ++dy){
		    // make sure dy+drop->y is within the thingy..
		    if( (dy + drop->y) >= py && (dy + drop->y) < py + h){
			for(int dx = -drop->radius; dx <= drop->radius; ++dx){
			    // and check the x location..
			    if( (dx + drop->x) >= px && (dx + drop->x) < px + w){
				// then assign the appropriate location of the area to the appropriate location
				// of the drop thingy.. 
				float v = bias + scale * drop->values[ radius * (d * d) + (radius + dy) * d + radius + dx];
				if(v > 0){
				    area[3 * (((dy + drop->y) - py) * w  + (dx + drop->x) - px)] +=  v * r;
				    area[3 * (((dy + drop->y) - py) * w  + (dx + drop->x) - px) + 1] +=  v * g;
				    area[3 * (((dy + drop->y) - py) * w  + (dx + drop->x) - px) + 2] +=  v * b;
				}
				if(dy == 0 && dx == 0 && sw->dropType() != DOT){
				    area[3 * (((dy + drop->y) - py) * w  + (dx + drop->x) - px)] +=  1.0;
				    area[3 * (((dy + drop->y) - py) * w  + (dx + drop->x) - px) + 1] +=  1.0;
				    area[3 * (((dy + drop->y) - py) * w  + (dx + drop->x) - px) + 2] +=  1.0;
				}
			    }
			}
		    }
		}
	    }
	}
    }

}

void DeltaViewer::paintBlobs(float* area, int xo, int yo, int z, int w, int h, bool isProjection){
    if(!blobManager)
	return;
    set<BlobMapperWidget*> blobs = blobManager->blobMapperWidgets();
    for(set<BlobMapperWidget*>::iterator it = blobs.begin(); it != blobs.end(); ++it)
      paintBlobs(area, xo, yo, z, w, h, (*it), isProjection);
}

void DeltaViewer::paintBlobs(float* area, int xo, int yo, int z, int w, int h, 
			     BlobMapperWidget* blb, bool isProjection){
    // There is no indication here as to what is the waveIndex represented by the blobs.
    // This is because we'll modify this function to use a reference to some kind of widget that
    // we can use to select colours and the like a bit later on. For now, I just want to leave as
    // is.
    if(blb->blobRep() == RepIcon::NO_REP)
	return;
    RepIcon::BlobRepresentation br = blb->blobRep();


    set<blob*>& blobs = blb->blobs();


    CoordConverter cc(fileSet->pwidth(),fileSet->pheight(), fileSet->sectionNo());
    float r, g, blue;
    blb->color(r, g, blue);
    //r = blue = 1.0;
    int blobCounter = 0;
    for(set<blob*>::iterator it=blobs.begin(); it != blobs.end(); ++it){
      int_color(++blobCounter, r, g, blue);
	blob* b = (*it);
	if( !(blb->filterBlob(b)) || !(b->active) )
	  continue;
	// check if it overlaps in any of the dimensions..
	if( !( b->min_x <= xo + w && b->max_x >= xo ) )
	    continue;
	if( !( b->min_y <= yo + h && b->max_y >= yo) )
	    continue;
	if(!isProjection &&  !(b->min_z <= z && b->max_z >= z) )
	    continue;
	// Then simply go through the  points and see whether or not
	int b_x, b_y, b_z;  // the point coordinates
	if(br == RepIcon::PEAK){
	  cc.vol(b->peakPos, b_x, b_y, b_z);
	  if(isProjection || b_z == z){
	    if(b_y >= yo && b_y <= yo+h
	       &&
	       b_x >= xo && b_x <= xo + w){
	      int area_offset = 3 * ((b_y - yo) * w + b_x - xo);
	      area[ area_offset ] += r;
	      area[ area_offset + 1] += g;
	      area[ area_offset + 2] += blue;
	    }
	  }
	  continue;
	}

	for(uint i=0; i < b->points.size(); ++i){
	    if(!b->surface[i] && br != RepIcon::FILLED)
		continue;
	    cc.vol(b->points[i], b_x, b_y, b_z);
	    if(!isProjection && b_z != z)
		continue;
	    if( !( b_y >= yo && b_y <= yo + h) )
		continue;
	    if( !(b_x >= xo && b_x <= xo + w) )
		continue;
	    int area_offset = 3 * ((b_y - yo) * w + b_x - xo);
	    area[ area_offset ] += r;
	    area[ area_offset + 1] += g;
	    area[ area_offset + 2] += blue;
	}
    }
}

void DeltaViewer::paint(float* area, std::vector<std::vector<o_set> >& points, Rectangle rect, int z, color_map cm){
  CoordConverter cc(fileSet->pwidth(), fileSet->pheight(), fileSet->sectionNo());
  
  int px, py, pz;
  int offset;
  float r, g, b;
  int counter = 0;
  for(vector<vector<o_set> >::iterator ht=points.begin(); ht != points.end(); ++ht){
    int_color(counter++, r, g, b);
    r *= 0.2; g *= 0.2; b *= 0.2;
    for(vector<o_set>::iterator it=(*ht).begin(); it != (*ht).end(); ++it){
      cc.vol(*it, px, py, pz);
      if(pz == z && rect.contains(px, py)){
	offset = 3 * ( (py - rect.y) * rect.width + (px - rect.x) );
	area[ offset ] += r;
	area[ offset + 1] += g;
	area[ offset + 2] += b;
      }
    }
  }
}

void DeltaViewer::exportPeaks(){
    // first check if we have some peaks worth exporting
    if(!spotsWidgets.size())
	return;
    // work out some useful name ..
    QString spotsFile = fName + ".spots";

    // Some useful data structures .
    map<simple_drop*, simple_drop_set*> dropMap;
    map<int, SpotsWidget*>::iterator ot;
    map<int, SpotsWidget*>::iterator it; // outer and inner iterators..
    for(ot = spotsWidgets.begin(); ot != spotsWidgets.end(); ot++){
	threeDPeaks* peakInfo1 = (*ot).second->peaks();
	int wl, r;
	float mpv, mev, r_f;
	ot->second->peakProperties(wl, r, mpv, mev);
	r_f = float(r);
	for(map<long, simple_drop>::iterator dot=peakInfo1->simpleDrops.begin(); dot != peakInfo1->simpleDrops.end(); dot++){
	    // and here first make an entry to the dropMap structure if none exits..
	    if(!dropMap.count(&(*dot).second)){
		dropMap.insert(make_pair(&dot->second, new simple_drop_set(&dot->second)));
	    }
	    // and then go through the other colours..
	    for(it = spotsWidgets.begin(); it != spotsWidgets.end(); it++){
		if(it->first == ot->first)
		    continue;
		threeDPeaks* peakInfo2 = (*it).second->peaks();
		for(map<long, simple_drop>::iterator dit=peakInfo2->simpleDrops.begin(); dit != peakInfo2->simpleDrops.end(); dit++){
		    // check if they have been defined.. if defined and the same, then we don't need to do anything.. 
		    if(dropMap.count(&dit->second) && dropMap[&dit->second] == dropMap[&dot->second])
			continue;
		    // check it the two drops overlap..
		    if(dot->second.peak_distance(dit->second) <= r_f){
//		    if(dot->second.overlaps(dit->second)){
			simple_drop_set* o_set = dropMap[&dot->second];
			// if dropMap has been defined for dit, we need to merge the maps and remove one of them.
			if(dropMap.count(&dit->second)){  // and we know it to be different.. 
			    simple_drop_set* i_set = dropMap[&dit->second];
			    o_set->drops.insert(i_set->drops.begin(), i_set->drops.end());
			    for(set<simple_drop*>::iterator sdit = i_set->drops.begin(); sdit != i_set->drops.end(); sdit++)
				dropMap[*sdit] = o_set;
			    dropMap.erase(&dit->second);
			    delete i_set;
			}else{
			    dropMap.insert(make_pair(&dit->second, o_set));
			    o_set->drops.insert(&dit->second);
			}
		    }
		}
	    }
	}
    }
    //// wheeew, That's ugly, and is going to be a bitch to reread and debug.
    set<simple_drop_set*> drop_sets;
    for(map<simple_drop*, simple_drop_set*>::iterator sit=dropMap.begin(); sit != dropMap.end(); sit++){
	drop_sets.insert(sit->second);
    }
    cout << "DeltaViewer::exportPeaks Total number of drops: " << dropMap.size() << "  total number of drop sets: " << drop_sets.size() << endl;
 
    // Then simply go through the simple_drop_sets and print out the information.. 
    // this should be fairly simple eh ?
    ofstream out(spotsFile.latin1());
    if(!out){
	cerr << "DeltaViewer::exportPeaks unable to open file ,, what a bummer" << endl;
    }else{
	// first write some information for the various thingys..
	for(it=spotsWidgets.begin(); it != spotsWidgets.end(); it++){
	    int wl, r;
	    float mpv, mev;
	    it->second->peakProperties(wl, r, mpv, mev);
	    out << "#" << wl << "\t" << r << "\t" << mpv << "\t" << mev << endl;
	}
	for(set<simple_drop_set*>::iterator sit=drop_sets.begin(); sit != drop_sets.end(); sit++){
	    out << endl;
	    for(set<simple_drop*>::iterator dip = (*sit)->drops.begin(); dip != (*sit)->drops.end(); dip++){
		out << (*dip)->waveLength << "\t" << (*dip)->x << "\t" << (*dip)->y << "\t" << (*dip)->z << "\t"
		    << (*dip)->sumValue << "\t" << (*dip)->peakValue
		    << "\t" << (*dip)->xb << "\t" << (*dip)->xe << "\t" << (*dip)->yb << "\t" << (*dip)->ye << "\t"
		    << (*dip)->zb << "\t" << (*dip)->ze << endl;
	    }
	}
    }
    // and delete the stuff
    while(drop_sets.size()){
	delete *(drop_sets.begin());
	drop_sets.erase(drop_sets.begin());
    }
}

void DeltaViewer::determineSpotDensities(int r, double sigma, double order){
    // first check if we have any spots to play with..
    if(!spotsWidgets.size()){
	cerr << "determineSpotDensities : no spots to determine density from" << endl;
	return;
    }
    if(r < 0){
	cerr << "determineSpotDensities radius is less than 0 " << r << endl;
	return;
    }
    sigma = abs(sigma);
    order = abs(order);  // just to make sure, these don't make much sense if negative
    // gaussian blur to give us e density map.. 
    unsigned int w = completeArea.pw;
    unsigned int h = completeArea.ph;
 
    float* values = new float[w * h];
    memset((void*)values, 0, sizeof(float) * w * h);
    
    // and then go through all of the spots..
    map<int, SpotsWidget*>::iterator it;
    for(it = spotsWidgets.begin(); it != spotsWidgets.end(); it++){
	threeDPeaks* peaks = (*it).second->peaks();
	for(map<long, simple_drop>::iterator dit=peaks->simpleDrops.begin(); 
	    dit != peaks->simpleDrops.end(); dit++){
	    // just check to make sure that x and y are within the limits..
	    int x = (uint) (*dit).second.x;
	    int y = (uint) (*dit).second.y;
	    // then fill in from xb -> xe, and yb -> ye
	    int yb = (y - r) < 0 ? 0 : (y - r);
	    int ye = (y + r) <= (int)h ? (y + r) : (int)h - 1;
	    int xb = (x - r) < 0 ? 0 : (x - r);
	    int xe = (x + r) <= (int)w ? (x + r) : (int)w - 1;
//	    cout << "w : " << w << "  h : " << h << "   x : " << x << "   y : " << y << "   yb : " << yb << "  ye : " << ye << "  xb : " << xb << "  xe : " << xe << endl;
	    for(int dy=yb; dy <= ye; dy++){
		for(int dx=xb; dx <= xe; dx++){
		    // determine the distance from the core and then use the thingy
		    double d = sqrt((double) ((dy - y)*(dy - y) + (dx - x)*(dx - x)));
		    values[dy * w + dx] += pow(order, -(d * d)/sigma);
		}
	    }
	}
    }
    // and then make a parameter data set and stuff..
    parameterData pd(values, w, h);
    QString labelString = QString("Spot density %1 : %2 %3")
	.arg(r)
	.arg(order)
	.arg(sigma);
    ParameterChooser* chooser = new ParameterChooser(pd, labelString, 0, 0, QColor(255, 0, 255), this);
    connect(chooser, SIGNAL(colorChanged(int, float, float, float)), this, SLOT(paramDataColorChanged(int, float, float, float)) );
    connect(chooser, SIGNAL(newRanges(float, float)), this, SLOT(paramDataRangeChanged(float, float)) );
    parameterSets.insert(make_pair(chooser, pd));
    colorBox->addWidget(chooser);
    chooser->show();
    /// and I hope that's all I need to do.. 
}

void DeltaViewer::blur(std::set<uint> channels, int r, double sigma, double order){
    cout << "Deltaviwer blur : " << endl;
    unsigned int w = completeArea.pw;
    unsigned int h = completeArea.ph;
    float* values = new float[w * h];
    memset((void*)values, 0, sizeof(float) * w * h);
    // and then for each channel that we have get the data from the file sets,, not sure if I remember how to do this, but..
    float* buffer = new float[w * h];
    for(set<uint>::iterator it=channels.begin(); it != channels.end(); it++){
	cout << "Getting values for channeld : " << (*it) << endl;
	memset((void*)buffer, 0, sizeof(float) * w * h); // just in case not all of it gets used..
	fileSet->readToFloatPro(buffer, 0, w, 0, h, (*it));
	// and then increment the values by the values in this one..
	// we could do this using the offset and scale specified in order to 
	// equalise the impact of channels with different strengths, but for now I think I will just do it
	// using the raw numbers..
	for(uint i=0; i < (w * h); ++i){
	    values[i] += buffer[i]; // I wonder if there is a faster way of doing this ?
	}
	cout << "values incremented" << endl;
    }
    // and this point we have a data set that we wish to blur. It seems that maybe we should put the blur functionality into the 
    // imageAnalyser object, -certainly seems more appropriate than here.. 
    // but first lets delete the buffer..
    
    // rather than delete the buffer we'll use it as the final output of the blur operation.
    // since the imageAnalyser will take care of 0ing it, we don't have to worry about it.. that's goood
    imageAnalyser->blur(values, buffer, w, h, r, sigma, order);
    // and since we don't need the values here we can get rid of them and ..
    delete values;
    // let's make a new parameter data thingy..
    parameterData pd(buffer, w, h);
    QString labelString = QString("Density %1 : %2 %3")
	.arg(r)
	.arg(order)
	.arg(sigma);
    ParameterChooser* chooser = new ParameterChooser(pd, labelString, 0, 0, QColor(0, 255, 255), this);
    connect(chooser, SIGNAL(colorChanged(int, float, float, float)), this, SLOT(paramDataColorChanged(int, float, float, float)) );
    connect(chooser, SIGNAL(newRanges(float, float)), this, SLOT(paramDataRangeChanged(float, float)) );
    parameterSets.insert(make_pair(chooser, pd));
    colorBox->addWidget(chooser);
    chooser->show();
}

void DeltaViewer::paintNuclei(float* area, int px, int py, int w, int h){
    cout << "paint Nuclei" << endl;
    map<ColorChooser*, vector<Nucleus> >::iterator it;
    for(it = nuclearModels.begin(); it != nuclearModels.end(); it++){
	float r, g, b;
	(*it).first->color(&r, &g, &b);
	if(r + g + b > 0){   // colour is not set to black (black means it's off)
	    for(vector<Nucleus>::iterator nit=(*it).second.begin(); nit != (*it).second.end(); nit++){
		// first check whether or not this nucleus overlaps with the current view..
		if((*nit).min_x < px + w && (*nit).max_x > px
		   &&
		   (*nit).min_y < py + h && (*nit).max_y > py){   // then we have an overlap
		    // go through all the points and set the corresponding voxels to thingy.. 
		    for(vector<twoDPoint>::iterator tit=(*nit).perimeter.begin(); tit != (*nit).perimeter.end(); tit++){
			// and if the point is within the area..
			if((*tit).x >= px && (*tit).x < px + w
			   &&
			   (*tit).y >= py && (*tit).y < py + h){
			    /// work out which area point to put to.. (assuming that total area starts at 0.. (bad assumption..)
			    area[3 * (((*tit).y - py) * w + ((*tit).x - px))] = r;
			    area[3 * (((*tit).y - py) * w + ((*tit).x - px)) + 1] = g;
			    area[3 * (((*tit).y - py) * w + ((*tit).x - px)) + 2] = b;
			}
		    }
		}
	    }
	}
    }
}
	
void DeltaViewer::paintParameterData(float* area, int px, int py, int w, int h){
    cout << "Paint ParamterData .." << endl;
    map<ParameterChooser*, parameterData>::iterator it;
    for(it = parameterSets.begin(); it != parameterSets.end(); it++){
	// read values from paramter sets.. 
	// make sure that the paramter data covers the given area..
	ParameterChooser* pc = (*it).first;
	parameterData& pd = (*it).second;   // for convenience
	float bias = pc->biasValue();
	float scale = pc->scaleValue();
	float r, g, b;
	
	int dMaxX = pd.xOrigin + pd.width;
	int dMaxY = pd.yOrigin + pd.height;
	
	pc->color(&r, &g, &b);
	cout << "\tpaintParameterData : r, g, b : " << r << "," << g << "," << b << "  scale : " << scale << "  bias : " << bias 
	     << "  minValue " << pd.minValue << "  maxValue : " << pd.maxValue  << endl;
	if(r + g + b > 0){
	    //    if(pd.xOrigin <= px && pd.xOrigin + pd.width >= px + w && pd.yOrigin <= py && pd.yOrigin + pd.height >= py + h){
	    // this is cheap test as it says don't bother to deal with partial overlaps.. maybe we can deal with that later on..
	    // shouldn't be that difficult to fix later on.. 
	    for(int dy = 0; (dy < h) && (dy + py < dMaxY); ++dy){
		float* dest = area + dy * w * 3;
		for(int dx = 0; (dx < w) && (dx + px < dMaxX); ++dx){
		    // work out the value to use from the thing..
//			float v = scale * bias + pd.values[ (py + dy) * pd.width + (px + dx)] * scale;
		    float v = (pd.values[ (py + dy) * pd.width + (px + dx)] + bias) * scale;
		    if(v > 0){
			dest[0] += v * r;
			dest[1] += v * g;
			dest[2] += v * b;
		    }
		    dest += 3;
		}
	    }
	    //   }else{
	    //cout << "paintParameterData.. no overlap between data and area" << endl;
	    //}
	}
    }
}

bool DeltaViewer::readProjection(ifstream& in){
    int pw, ph, cno, tsize, panelNo;
    in.read((char*)&pw, sizeof(int));
    in.read((char*)&ph, sizeof(int));
    in.read((char*)&cno, sizeof(int));
    in.read((char*)&tsize, sizeof(int));
    in.read((char*)&panelNo, sizeof(int));
    if(pw != completeArea.pw || ph != completeArea.ph || cno != fileSet->channelNo() || tsize != textureSize || panelNo != texRowNo * texColNo){
	cerr << "DeltaViewer::readProjection one of the parameters is unsuitable " << pw << ", " << ph << ", " << cno << ", " << tsize << ", " << panelNo << endl;
	return(false);
    }
    // to make sure.. 
    for(uint i=0; i < projection_data.size(); i++){
	delete projection_data[i];
    }
    projection_data.resize(0);
    for(int i=0; i < panelNo; i++){
	projection_data.push_back(new raw_data(cno, textureSize * textureSize));
	for(int j=0; j < cno; j++){
	    in.read((char*)projection_data.back()->values[j], sizeof(float) * textureSize * textureSize);
	}
    }
    if(in.fail()){
	cerr << "DeltaViewer::readProjection we seem to have failed to read to one of the panels return false" << endl;
	return(false);
    }
    // if we are here just call paintProjection and return true..
    paintProjection();
    return(true);
}

void DeltaViewer::newStackSelected(float x, float y){
  cout << "Deltaviewer new Stack selected at " << x << "," << y << endl;
  olapEditor->setBorderImages( fileSet->borderInformation(x, y) );
}

void DeltaViewer::adjustStackPosition(float x, float y, QPoint p){
  fileSet->adjustStackPosition(x, y, p);
  setProjection();
  //  paintProjection();
  setImage(currentSliceNo);
}

void DeltaViewer::updateFileSetInfo(){
  cout << "Update File set info trying to do something useful" << endl;
  if(!fileSet->updateFileSetInfo())
    cerr << "Unable to update file Set info" << endl;
}

void DeltaViewer::setSliceNo(int n){
  QString numstring;
  numstring.setNum(n);
  numstring.prepend("Section ");
  sectionNo->setText(numstring);
}

void DeltaViewer::increment_x_z_slice(int delta){
    cout << "DeltaViewer::increment_x_z_slice  function currently unavailalbe, please code it up" << endl;
    delta = delta;
//     yPosition += delta;
//     cout << "setting yPosition to : " << yPosition << endl;
//     x_zView->setImage(reader->x_zSlice(yPosition), reader->width(), reader->sliceNo());
//     x_zView->updateGL();
//     plotLines();
}

void DeltaViewer::increment_x_line(int delta){
    yPosition += delta;
    plotLines();
}

void DeltaViewer::increment_y_line(int delta){
    xPosition += delta;
    plotLines();
}

void DeltaViewer::setSpotsUseProjection(bool useProjection){
    spotsUseProjection = useProjection;
    plotLines();
}

// void DeltaViewer::increment_y_z_slice(int delta){
//     xPosition += delta;
//     y_zView->setImage(reader->y_zSlice(xPosition), reader->height(), reader->sliceNo());
//     y_zView->updateGL();
// }

void DeltaViewer::setSlices(int xpos, int ypos){
    cout << "DeltaViewer::setSlices .. this function is currently unsupported in panel reading stuff" << endl;
    cout << "Ok, I should here be setting the data for the slices and perhaps redrawing them,, but " << endl;
    cout << "xpos is : " << xpos << "  and ypos is " << ypos << endl;
//     x_zView->setImage(reader->x_zSlice(ypos), reader->width(), reader->sliceNo());
//     if(!x_zView->isVisible()){
// 	x_zView->show();
//     }

    xPosition = xpos;
    yPosition = ypos;
    plotLines();
    
//     x_zView->updateGL();
//     plotLines();
}

void DeltaViewer::plotLines(){   // use xposition and yposition.. 
//    cout << "DeltaViewer::plotLines This function is not currently supported " << endl;

     if(yPosition < 0){ yPosition = 0; }
     if(xPosition < 0){ xPosition = 0; }
     vector<vector<float> > xLines;
     vector<vector<float> > yLines;
     if(spotsUseProjection){
	 xLines = imageAnalyser->mip_xline(yPosition);
	 yLines = imageAnalyser->mip_yline(xPosition);
     }else{
	 xLines = imageAnalyser->x_line(yPosition, currentSliceNo);
	 yLines = imageAnalyser->y_line(xPosition, currentSliceNo);
     }
     vector<int> xMarks(1);
     vector<int> yMarks(1);
     xMarks[0] = xPosition;   // to show the intersect point.. 
     yMarks[0] = yPosition;

     // This may not work, as I'm not sure that colors have been set at this point. But if not later they should be
     map<uint, vector<float> > auxLines;
     if(blobManager){
	 set<BlobMapperWidget*> bmappers = blobManager->blobMapperWidgets();
	 for(set<BlobMapperWidget*>::iterator it=bmappers.begin(); it != bmappers.end(); ++it){
	     auxLines[ (*it)->wave_index() ] = (*it)->x_background(yPosition, currentSliceNo, 1);
	 }
     }
     spotWindow->setAuxLines(1, auxLines);
     spotWindow->setLineValues(yPosition, 1, xLines, xMarks, 50, 50, 0, 1.0);
     spotWindow->setLineValues(xPosition, 2, yLines, yMarks, 50, 50, 0, 1.0);
     if(!spotWindow->isVisible()){
	 spotWindow->show();
     }
}

void DeltaViewer::displaySlices(){
    cout << "DeltaViewer::displaySlices this function is currently unsupported .. " << endl;
//     cout << "display Slices " << endl;
//     x_zView->setImage(reader->x_zSlice(yPosition), reader->width(), reader->sliceNo());
//     cout << "Images set " << endl;
//     x_zView->updateGL();
//     cout << "gl update " << endl;
}

void DeltaViewer::displayImage(){

//     timespec time;
//     int clockRes = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
//     long startTime = time.tv_nsec;

//    glViewer->setImage(reader->rgb_image(), reader->width(), reader->height());  // don't delete that data.. 

//     clockRes = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
//     long imageSetTime = time.tv_nsec;
    glViewer->updateGL();
 //    clockRes = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
//     long endTime = time.tv_nsec;
//     cout << "\ndisplay Image : imageSetTime\t" << imageSetTime - startTime << endl
// 	 << "               : updateTime\t" << endTime - imageSetTime << endl;

    // and let's calculate the distribution for this.. if its selected ..  
    if(updateDist->isOn()){
	calculateDistribution();
    }
    
    if(animate){
	timer->start(redrawWait, true);
    }
    if(spotWindow->isVisible()){
	plotLines();
    }
}

void DeltaViewer::calculateDistribution(){
    if(!raw){
	cerr << "DeltaViewer::calculateDistribution raw is 0" << endl;
	return;
    }
    vector<vector<float> > v(raw->channels);
    for(uint i=0; i < raw->channels; i++){
	v[i].resize(raw->positions[i]);
	for(uint j=0; j < raw->positions[i]; j++){
	    v[i][j] = raw->values[i][j];
	}

	choosers[i]->setData(v[i], 0, chooserMaxValue, false);

    }

}

void DeltaViewer::updateDistToggled(bool on){
  if(on){
    calculateDistribution();
    chooserTabs->show();
    chooserTabs->raise();   // so I can find them .. hmm.. 
  }
}

/// This function now handles the background issues. 
/// Not in the smartest way, but anyway..

void DeltaViewer::useComponentsToggled(bool on){
  //    cout << "useComponents currently unavailable" << endl;

  // Reverting to a 2D background subtraction for now as we are having
  // trouble with the 3D one. (Temporarily)
  
  // if(!backgroundWindow){
  //   backgroundWindow = new BackgroundWindow( fileSet->channelInfo() );
  //   connect(backgroundWindow, SIGNAL(setBackgroundPars(std::map<fluorInfo, backgroundPars>)),
  // 	    this, SLOT(setBackgroundPars(std::map<fluorInfo, backgroundPars>)) );
  //   backgroundWindow->show();
  //   return;
  // }

  on = on;
  setImage(currentSliceNo);
//     reader->useComponentFactors(on);
//     displayImage();
//     displaySlices();
}

void DeltaViewer::setBackgroundPars(std::map<fluorInfo, backgroundPars> bgPars){
  cout << "Calling some function in fileSet to set the background parameters" << endl;
  for(map<fluorInfo, backgroundPars>::iterator it=bgPars.begin(); it != bgPars.end(); it++){
    cout << "DeltaViewer pars are : " << it->second.x_m << "," << it->second.y_m << "," << it->second.z_m << " : " << it->second.pcntile << endl;
  }
  fileSet->setBackgroundParameters(bgPars);
  setImage(currentSliceNo);
}

void DeltaViewer::setChooserMaxValue(float mv){
  chooserMaxValue = mv;
  // how to cause an update.? 
  bool wasOff = !(updateDist->isChecked());
  updateDist->setChecked(true);
  setImage(currentSliceNo);
  if(wasOff)
    updateDist->setChecked(false);
  changeRanges(0, 1.0);  // note that changeRanges ignorese these parameters.
  
}

void DeltaViewer::newMagnification(float mag){
    // set the text of magnification appropriately.
    QString magString;
    magString.sprintf("%'.2f", mag);
    magnification->setText(magString);
}

void DeltaViewer::newOffsets(int xo, int yo){
    // integers so easy.
    QString xoff, yoff;
    xoff.setNum(xo);
    yoff.setNum(yo);
    xOffset->setText(xoff);
    yOffset->setText(yoff);
}

void DeltaViewer::newMousePosition(int xp, int yp){
    QString xs, ys;
    xs.setNum(xp);
    ys.setNum(yp);
    mouseX->setText(xs);
    mouseY->setText(ys);
    // The below doesn't work well, because imageAnalyser assumes that you
    // won't be looking at more than one wavelength at a time.
    // hence it ends up making a new cache everytime 4 times at every point
    // and since that means reading in four pages, that's really screwed.
    // lets see if this causes problems..
    float p;
    for(uint i=0; i < colorChoosers.size(); ++i){
      if( fileSet->readToFloat(&p, xp, yp, (unsigned int)currentSliceNo, 1, 1, 1, i) )
    	sliceSignals->setValue(i, p);
    }

}

void DeltaViewer::newProMousePosition(int xp, int yp){
    QString xs, ys;
    xs.setNum(xp);
    ys.setNum(yp);
    proMouseX->setText(xs);
    proMouseY->setText(ys);
}

void DeltaViewer::changeRanges(float mn, float mx){
  mn = mn; mx = mx; // since we don't know which distChooser brought us to this location, we might as 
  // well update all of the values. This is wasteful, but it is simpler..
  vector<float> bFactors(choosers.size());
  vector<float> sFactors(choosers.size());
  for(uint i=0; i < choosers.size(); i++){
    // basically we want to set a scale factor and a bias factor to bracket the thingy..
    // this gets a little bit complicated, but here goes..
    // for now assume that min is equal to about 0. This simplifies things, but...
    float min = choosers[i]->tMin();
    float max = choosers[i]->tMax();
    float Max = choosers[i]->vMax();  // which isn't actually necessarily true, but..
    cout << endl << "Max : " << Max << "\tnew min : " << min << "  max : " << max << endl;

    float scaleFactor = 1.0 / (max - min);
    //    float scaleFactor = Max / (max - min);
    //    float biasFactor = -scaleFactor * (min / Max);
    float biasFactor = -scaleFactor * (min);
    bFactors[i] = biasFactor;
    sFactors[i] = scaleFactor;
    cout << "setting scale to : " << scaleFactor << "  bias : " << biasFactor << endl << endl;
  }
  biases = bFactors;
  scales = sFactors;

//  reader->setBiasAndScale(bFactors, sFactors);
  displayImage();
  displaySlices();
  
  paintProjection();

}

void DeltaViewer::changeDropThresholds(float minT, float maxT){
    cout << "Drops and drop thresholds not yet supported" << endl;
    minT = maxT = 0;// since we don't know which wavelength this represents we are just going to
    // pass it on..
//     map<int, DistChooser*>::iterator it;
//     for(it = spotDistributions.begin(); it != spotDistributions.end(); it++){
// 	reader->setDropThresholds((*it).first, (*it).second->tMin(), (*it).second->tMax());
//     }
//     // and update the stuff..
//     reader->setImageData();
//     displayImage();
//     // no need to display Slices since these don't show the things yet.. but since it doesn't take that much time..
//     displaySlices();
}

void DeltaViewer::setAdditive(bool b){
    b = b;// but ignore b.. 
    vector<bool> factors(colorChoosers.size());
    for(uint i=0; i < colorChoosers.size(); i++){
	factors[i] = !(colorChoosers[i]->subtractColor());
    }
    // and then set .. thingy..
    additive = factors;

//     reader->setAdditive(factors);
    
//     // and update ..
//     reader->setImageData();
//     displayImage();
//     displaySlices(); 
}


QString DeltaViewer::readRanges(){
  QString ranges;
  QTextStream ts(&ranges, QIODevice::WriteOnly);
  for(uint i=0; i < choosers.size(); i++){
    // get the relevant information from the chooser and format it in some sort of reasonable way.
    // need to have some sort of identifier..
    DistChooser* dc = choosers[i];
    ts << dc->statname().c_str() << endl 
       << dc->dMin() << " " << dc->dMax() << " "
       << dc->tMin() << " " << dc->tMax() << endl;
  }
  return(ranges);
}


vector<channel_info> DeltaViewer::collect_channel_info(){
  vector<channel_info> chinfo;
  cout << "DeltaViewer::collect_channel_info() size of colorChoosers : " << colorChoosers.size() << endl;
  cout << "size of scales : " << scales.size() << endl;
  for(uint i=0; i < colorChoosers.size(); ++i){
    if(i >= scales.size() || i >= biases.size()){
      cerr << "DeltaViewer::setImage colorChoosers.size is larger than scales.size or biases.size()" << endl;
      exit(1);
    }
    float r, g, b;
    colorChoosers[i]->color(&r, &g, &b);
    cout << i << " color : " << r << "," << g << "," << b << endl;
    //      chinfo.push_back(channel_info( color_map(r, g, b), maxLevel, biases[i], scales[i],
    //				     useComponents->isChecked(), colorChoosers[i]->subtractColor()) );
    chinfo.push_back(channel_info( color_map(r, g, b), maxLevel, biases[i], scales[i],
				   colorChoosers[i]->includeInMerger(), colorChoosers[i]->subtractColor()) );
    cout << "pushed back" << endl;
  }
  return(chinfo);
}

void DeltaViewer::copyRanges(){
    cout << "copyRanges function " << endl;
    QString ranges = readRanges();
    QClipboard* cb = QApplication::clipboard();
    cb->setText(ranges, QClipboard::Clipboard);
}

void DeltaViewer::saveRanges(){
  // save to a file..
  QString null;
  QString fileName = Q3FileDialog::getSaveFileName(null, "Viewer ranges (*.ranges);; all (*)");
  if(fileName.isNull()){
    return;
  }
  QString ranges = readRanges();
  // and do something... eh.. 
  QFile file(fileName);
  if( file.open( QIODevice::WriteOnly )){
    QTextStream ts(&file);
    ts << ranges;
    file.close();
  }
}

void DeltaViewer::readRangesFromFile(){
  QString null;
  QString fileName = Q3FileDialog::getOpenFileName(null, "Viewer ranges (*.ranges);; all (*)");
  if(fileName.isNull()){
    return;
  }
  readRangesFromFile(fileName);
}

void DeltaViewer::readRangesFromFile(QString fileName){
  QString ranges;
  QTextStream wts(&ranges, QIODevice::WriteOnly);
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly)){
    QTextStream rts(&file);
    QString line;
    while(!rts.atEnd()){
      line = rts.readLine();
      wts << line << endl;
    }
  }
  //  cout << "and ranges is : " << ranges << endl;
  setRanges(ranges);
}
  
void DeltaViewer::setHome(){
  // string user_home = getenv("HOME");
  // user_home.append("/.dvreader");
  // dvreader_home = user_home.c_str();
}


void DeltaViewer::setRanges(QString text){
  QTextStream ts(&text, QIODevice::ReadOnly);
  QString word;
  while(!ts.atEnd()){
      word = ts.readLine();
      float dMin, dMax, tMin, tMax;
      string id;
      id = word.latin1();
      ts >> dMin >> dMax >> tMin >> tMax;   // should work,, ? 
      word = ts.readLine();   // just gets rid of the extra endl.
      cout << "id : " << id << " dmin: " << dMin << " dmax: "<< dMax << " tmax: " << tMax << " tmin: " << tMin << endl;
      // and now try to find a distchooser with the appropriate id..
      for(uint i=0; i < choosers.size(); i++){
	  if(choosers[i]->statname() == id){
	      choosers[i]->setParams(dMin, dMax, tMin, tMax);
	  }
      }
  }
  changeRanges(0.0, 1.0);   // this is called in order to change the ranges for the image.. 
}  

vector<color_map> DeltaViewer::readColors(string fname){
  cout << "DeltaViewer::readColors filename : " << fname << endl;
  ifstream in(fname.c_str());
  vector<color_map> cm;
  if(!in)
    return(cm);
  float r, g, b;
  while(in >> r >> g >> b)
    cm.push_back(color_map(r, g, b));
  return(cm);
}

void DeltaViewer::pasteRanges(){
    cout << "paste Ranges function " << endl;
    QClipboard* cb = QApplication::clipboard();
    QString text = cb->text(QClipboard::Clipboard);
    setRanges(text);
}

void DeltaViewer::startTimer(){
  animate = true;
  timer->start(redrawWait, true);
}

void DeltaViewer::stopTimer(){
  animate = false;
  timer->stop();
}

void DeltaViewer::findLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue){
    cout << "DeltaViewer::findLocalMaxima currently being redesigned" << endl;
    wl = radius = 0; minPeakValue = maxEdgeValue = 0;//     cout << "DeltaViewer::findLocalMaxima " << endl;
//     peaks.erase(wl);
//     peaks.insert(make_pair(wl, reader->findLocalMaxima(wl, radius, minPeakValue, maxEdgeValue)));
//     // and then we can do stuff with the mapped peaks.. (i.e. provide something to draw in the plot window.. ).. 
//     spotWindow->setPeaks(peaks);
}


void DeltaViewer::findAllLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue, int clusterK, float bgm, bool exportFile){
    findAllLocalMaxima(wl, radius, minPeakValue, maxEdgeValue, clusterK, bgm, exportFile, false);
}

void DeltaViewer::findAllLocalMaxima3D(int wl, int radius, float minPeakValue, float maxEdgeValue, int clusterK, float bgm, bool exportFile){
    findAllLocalMaxima(wl, radius, minPeakValue, maxEdgeValue, clusterK, bgm, exportFile, true);
}

void DeltaViewer::find_spots(string p_file){
    spotWindow->find_spots3D(p_file);  // This loops back and causes this object to do all the relevant stuff..
    exportPeaks();
}

void DeltaViewer::findAllLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue, int clusterK, float bgm, bool exportFile, bool use3D){
    clusterK = clusterK;//    cout << "DeltaViewer::findLocalMaxima is currently all commented out, but is awaiting the availabity of nice new functions" << endl;
    cout << "Calling imageAnalyser to find peaks parameters are : " << wl << ", " << radius << ", " << minPeakValue << ", " << maxEdgeValue << ", " << bgm << endl;
    threeDPeaks* peakInfo = 0;
    if(use3D){
	peakInfo = imageAnalyser->findAllPeaks_3D((unsigned int)wl, radius, minPeakValue, maxEdgeValue, bgm);
    }else{
	peakInfo = imageAnalyser->findAllPeaks((unsigned int)wl, radius, minPeakValue, maxEdgeValue, bgm);
    }

    cout << "DeltaViewer::findAllLocalMaxima got a total of " << peakInfo->simpleDrops.size() << "  peaks" << endl;

    QString wlabel;
    wlabel.setNum(wl);
    int waveLength = fileSet->channel((unsigned int)wl);
    SpotsWidget* spots = new SpotsWidget(wlabel, waveLength, wl, radius, minPeakValue, maxEdgeValue, peakInfo, this, "spots");
    connect(spots, SIGNAL(colorChanged()), this, SLOT(peakColorsChanged()) );
    connect(spots, SIGNAL(repTypeChanged()), this, SLOT(peakRepTypeChanged()) );
//    connect(spots, SIGNAL(colorChanged(int, float, float, float)), this, SLOT(peakColorsChanged(int, float, float, float)) );
//    connect(spots, SIGNAL(repTypeChanged(DropRepresentation)), this, SLOT(peakRepTypeChanged(DropRepresentation)) );
    /// then connect the appropriate signals and slots.. 
    
    if(spotsWidgets.count(wl)){
	delete spotsWidgets[wl];
    }
    spotsWidgets[wl] = spots;
    colorBox->addWidget(spots);
    spots->show();   // which should be fine .. 

    // also tell the spotmapperwindow that we have 
    map<int, threeDPeaks*> pm;
    for(map<int, SpotsWidget*>::iterator it=spotsWidgets.begin(); it != spotsWidgets.end(); it++){
	pm.insert(make_pair(it->second->wlength(), it->second->peaks()));
    }
    spotMapperWindow->setBlobs(pm);
    
    // if exportFile, then we'll write out a file with a systematic file name.
    if(exportFile){
	ostringstream ename;
	ename << fName.latin1() << "_" << waveLength << "_" << radius << "_" << minPeakValue << "_" << maxEdgeValue << ".spots";
	ofstream out(ename.str().c_str());
	if(out){
	    out << "#Total drops : " << peakInfo->simpleDrops.size() << endl;
	    for(map<long, simple_drop>::iterator it = peakInfo->simpleDrops.begin(); it != peakInfo->simpleDrops.end(); it++){
		simple_drop* drop = &(it->second);
		out << drop->x << "\t" << drop->y << "\t" << drop->z << "\t" 
		    << drop->peakValue << "\t" << drop->sumValue << "\t" << drop->background << "\t"
		    << drop->xb << "\t" << drop->xe << "\t" << drop->yb << "\t" << drop->ye << "\t" << drop->zb << "\t" << drop->ze << "\t"
		    << (drop->xe - drop->xb) * (drop->ye - drop->yb) * (drop->ze - drop->zb) << endl;
	    }
	}
    }
    /// UGLY hack warning. The kclusterprocess wants a vector of simple drops.. so..
    vector<simple_drop> simpleDrops;
    for(map<long, simple_drop>::iterator it=peakInfo->simpleDrops.begin(); it != peakInfo->simpleDrops.end(); it++)
	simpleDrops.push_back(it->second);

    // and finally if we have specified a cluster number..
    if(clusterK){
	KClusterProcess* clusterP = new KClusterProcess(clusterK, simpleDrops);
//	KClusterProcess* clusterP = new KClusterProcess(clusterK, peakInfo->drops);
	dropClusterWindow->setClusters(clusterP);
	dropClusterWindow->show();
	dropClusterWindow->raise();
    }
}

void DeltaViewer::recalculateSpotVolumes(){
    cout << "DeltaViewer::recalculateSpotVolumes currently commented out" << endl;

//     reader->recalculateSpotVolumes();
//     map<int, DistChooser*>::iterator it;
//     for(it = spotDistributions.begin(); it != spotDistributions.end(); it++){
// 	vector<float> values = reader->spotValues((*it).first);
// 	float min = 0;
// 	float max = 0;
// 	for(uint i=0; i < values.size(); i++){
// 	    if(min > values[i]){ min = values[i]; }
// 	    if(max < values[i]){ max = values[i]; }
// 	}
// 	(*it).second->setData(values, min, max);
//     }
    
}

void DeltaViewer::findNuclearPerimeters(int wi, float minValue){
    cout << "DeltaViewer::findNuclearPerimeters New Function alleluliahhh." << endl;
    cout << "calling completeProjection to get projection data fo appropriate wavelength" << endl;

//    float* source = completeProjection((unsigned int)wi);
    float* source = new float[completeArea.pw * completeArea.ph];
    fileSet->readToFloat(source, 0, 0, currentSliceNo, completeArea.pw, completeArea.ph, 1, (unsigned int)wi); 
//    fileSet->readToFloat(source, 0, 0, currentSliceNo, completeArea.pw, completeArea.ph, 1, (float)fileSet->channel(wi)); 

   cout << "calling findNuclei from imageAnalyser.. " << endl;
    vector<Nucleus> nuclei = imageAnalyser->findNuclei(source, completeArea.pw, completeArea.ph, minValue);
    cout << "obtained a total of " << nuclei.size() << " nuclei" << endl;
    delete source;
    cout << "deleted source " << endl;

    // make a color chooser that we  can use..
    QString labelString;
    labelString.setNum(fileSet->channel(wi));
    labelString.append(" nuclei");
    ColorChooser* chooser = new ColorChooser(labelString, wi, fileSet->channel(wi), QColor(255, 255, 255), this);
    connect(chooser, SIGNAL(colorChanged(int, float, float, float)), this, SLOT(nuclearColorsChanged(int, float, float, float)));
    nuclearModels.insert(make_pair(chooser, nuclei));
    colorBox->addWidget(chooser);
    chooser->show();
//    reader->findNuclearPerimeters(wi, minValue);
//    flatView->setNuclei(reader->currentNuclei());
}

void DeltaViewer::findContrasts(int wi, float minValue){
    cout << "DeltaViewer::findContrasts " << endl;
    minValue = minValue;  // to remove warning statement.. 
//    float* source = completeProjection((unsigned int)wi);
    
    float* source = new float[completeArea.pw * completeArea.ph];
    fileSet->readToFloat(source, 0, 0, currentSliceNo, completeArea.pw, completeArea.ph, 1, (unsigned int)wi);
//    fileSet->readToFloat(source, 0, 0, currentSliceNo, completeArea.pw, completeArea.ph, 1, (float)fileSet->channel(wi));

    cout << "Calling imageAnalyser->findContrasts" << endl;
    parameterData contrastData = imageAnalyser->findContrasts(source, completeArea.pw, completeArea.ph);
    parameterData secContrastData = imageAnalyser->findContrasts(contrastData.values, completeArea.pw, completeArea.ph);
    cout << "Received the contrastData and will get rid of the source data.." << endl;
    delete source;

    QString labelString;
    labelString.setNum(fileSet->channel(wi));
    labelString.append(" Contrast");
    ParameterChooser* chooser = new ParameterChooser(contrastData, labelString, wi, fileSet->channel(wi), QColor(255, 255, 0), this);
    cout << "findContrasts first param chooser created" << endl;
    connect(chooser, SIGNAL(colorChanged(int, float, float, float)), this, SLOT(paramDataColorChanged(int, float, float, float)) );
    cout << "first chooser connected" << endl;
    connect(chooser, SIGNAL(newRanges(float, float)), this, SLOT(paramDataRangeChanged(float, float)) );
    cout << "first chooser second connection" << endl;
    parameterSets.insert(make_pair(chooser, contrastData));
    colorBox->addWidget(chooser);
    chooser->show();

    // and for the secondary data.. 
    labelString = "";
    labelString.setNum(fileSet->channel(wi));
    labelString.append(" Contrast (sec)");
    ParameterChooser* chooser2 = new ParameterChooser(secContrastData, labelString, wi, fileSet->channel(wi), QColor(0, 255, 0), this);
    connect(chooser2, SIGNAL(colorChanged(int, float, float, float)), this, SLOT(paramDataColorChanged(int, float, float, float)) );
    connect(chooser2, SIGNAL(newRanges(float, float)), this, SLOT(paramDataRangeChanged(float, float)) );
    parameterSets.insert(make_pair(chooser2, secContrastData));
    colorBox->addWidget(chooser2);
    chooser2->show();

}


void DeltaViewer::findBlobs(string& params){
    cout << "Find blobs, let's try out parameter parsing with QRegExp  " << params << endl;
    QString qp(params.c_str());
    QRegExp rx("[,:]?(\\d*\\.?\\d*)[,:]?");
    map<int, float> param_pairs;
    int pos = 0;
    bool ok;
    while( (pos = rx.indexIn(qp, pos) ) != -1){
	int wl = (rx.cap(1)).toInt(&ok);
	pos += rx.matchedLength();
	if( !ok || ( (pos = rx.indexIn(qp, pos) ) == -1) ){
	    cout << "gave up because of ok: " << ok << " or pos " << pos << endl;
	    break;
	}
	pos += rx.matchedLength();
	float min = (rx.cap(1)).toFloat(&ok);
	if(!ok){
	    cout << "didn't get a float from " << rx.cap(1).latin1() << endl;
	    break;
	}
	param_pairs.insert(make_pair(wl, min));
    }
    QString ofName = fName;
    QTextStream qts(&ofName);
    for(map<int, float>::iterator it=param_pairs.begin(); it != param_pairs.end(); ++it){
	cout << "\tParam " << (*it).first << " : " << (*it).second << endl;
	qts << "_" << it->first << "-" << it->second;
	mapBlobs(it->first, it->second);
    }
    qts << ".blobs";
    if(!blobManager){
	cerr << "blobManager should exist here, but doesn't" << endl;
	return;
    }
    blobManager->makeSuperBlobs();
    blobManager->exportSuperBlobs(ofName.latin1());
}


void DeltaViewer::mapBlobs(int wi, float minValue){
    mapBlobs(wi, minValue, 32, 32, 4, 75);
}

void DeltaViewer::mapBlobs(int wi, float minValue, int xw, int yw, int zw, float pcnt){
    cout << "Deltaviewer map blobs.. " << endl;

    // and let's make a thingy blobmapper
    fluorInfo fInfo = fileSet->channelInfo((unsigned int)wi);
    if(!fInfo.excitation)
	return;
    BlobMapper* bm = new BlobMapper(new ImageData(fileSet, 1), xw, yw, zw, pcnt );
    bm->mapBlobs(minValue, (unsigned int)wi, 1, fInfo);
    
    if(!blobManager){
	blobManager = new BlobMapperWidgetManager(this);
	connect(blobManager, SIGNAL(newColor()), this, SLOT(paintProjection()) );
	connect(blobManager, SIGNAL(newRep()), this, SLOT(paintProjection()) );
	connect(blobManager, SIGNAL(newLimits()), this, SLOT(paintProjection()) );
	colorBox->addWidget(blobManager);
	blobManager->show();
    }
    blobManager->addBlobMapper(bm, fInfo, fName.latin1(), wiColor(wi));
}

void DeltaViewer::makeBlobModel(int xy_radius, int z_radius){
  cout << "DeltaViewer makeBlobModel : " << xy_radius << ", " << z_radius << endl;
  int xy_r = xy_radius;
  int z_r = z_radius;
  float* model = blobManager->two_dim_model(xy_radius, z_radius);
  if(xy_r != xy_radius || z_r != z_radius){
    cerr << "makeBlobModel problem lets die" << endl;
    exit(1);
  }
  imageBuilder->setGreyImage(model, (xy_radius + 1), (2 * z_radius + 1));
}

void DeltaViewer::mapCavities(int wi, int xr, int yr, int zr, float P, float DP){
  // create mapper is enough to start the thing:
  CavityMapper* cmapper = new CavityMapper(new ImageData(fileSet, 1), (uint)wi,
					   xr, yr, zr, P, DP);
  // That actually should start the whole edifice running. 
  aux_points = cmapper->ballMembers();  // though this will be horribly slow!! 
  ////// MEMORY LEAK IF WE DON'T DELETE CMAPPER !! OR DO SOMETHING USEFUL WITH IT
}

void DeltaViewer::findSets(int wi, int minSize, int maxSize, float minValue){
    cout << "DeltaViewer FindSets " << endl;
    float* source = new float[completeArea.pw * completeArea.ph];
    fileSet->readToFloat(source, 0, 0, currentSliceNo, completeArea.pw, completeArea.ph, 1, (unsigned int)wi);
//    fileSet->readToFloat(source, 0, 0, currentSliceNo, completeArea.pw, completeArea.ph, 1, (float)fileSet->channel(wi));
    
    cout << "Calling imageAnalyser->findSets" << endl;

    PerimeterData perData = imageAnalyser->findPerimeters(source, completeArea.pw, completeArea.ph, minSize, maxSize, minValue);
//    parameterData setData = imageAnalyser->findPerimeters(source, completeArea.pw, completeArea.ph, minSize, maxSize, minValue);
//    parameterData setData = imageAnalyser->findSets(source, completeArea.pw, completeArea.ph, minSize, maxSize, minValue);

    cout << "findSets was called with a source of dims : " << completeArea.pw << "x" << completeArea.ph << endl;

    QString labelString;
    labelString.setNum(fileSet->channel(wi));
    labelString.append(" Sets");

    perimeterWindow->setPerimeters(perData.perimeterData, (float)fileSet->channel(wi), currentSliceNo);
    perimeterWindow->show();

    cout << "data given to perimeterWindow and show() called" << endl;

    ParameterChooser* chooser = new ParameterChooser(perData.p_data, labelString, wi, fileSet->channel(wi), QColor(255, 255, 0), this);
    cout << "Parameter chooser made for the thingy" << endl;
    connect(chooser, SIGNAL(colorChanged(int, float, float, float)), this, SLOT(paramDataColorChanged(int, float, float, float)) );
    connect(chooser, SIGNAL(newRanges(float, float)), this, SLOT(paramDataRangeChanged(float, float)) );
    parameterSets.insert(make_pair(chooser, perData.p_data));
    colorBox->addWidget(chooser);
    chooser->show();
    
}

// void DeltaViewer::setSpotsToSpotMapper(){
//     // if we have some spots set these to the spotmapper window. (also call show on the spot mapper window)

//     spotMapperWindow->show();
// }

void DeltaViewer::makeModel(int xBegin, int Width, int yBegin, int Height, int zBegin, int Depth, set<int> channels){
    cout << "DeltaViewer::MakeModel currently out of order come back later or fix it yoursefl" << endl;
    xBegin = Width = yBegin = Height = zBegin = Depth = 0;//     cout << "make Model called with paramters : " 
// 	 << xBegin << "\t" << Width << "\t"
// 	 << yBegin << "\t" << Height << "\t"
// 	 << zBegin << "\t" << Depth << endl;
//     voxelVolume vol = reader->makeModel(xBegin, Width, yBegin, Height, zBegin, Depth, channels);
//     modelViewer->setModel(vol);
//     modelViewer->show();
//     modelViewer->raise();
}

void DeltaViewer::newLine(int x1, int y1, int x2, int y2){
    x1 = x1; y1 = y1; x2 = x2; y2 = y2;// flat line so z is equal to frameNo
    cout << "DeltaViewer::newLine is currently commented out please fix for new data model" << endl;
//     int z = reader->currentSectionNo();
//     unsigned int points = 20;
//     float* line = reader->arbitraryLine((unsigned int)1, (unsigned int)x1, (unsigned int)y1, (unsigned int)z, (unsigned int)x2, (unsigned int)y2, (unsigned int)z, points);
//     if(line){
// 	for(uint i=0; i < points; i++){
// 	    cout << i << "\t" << line[i] << endl;
// 	}
//     }
//     delete line;
}
