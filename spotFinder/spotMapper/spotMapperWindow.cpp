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
 
    PS. If you can think of a better name, please lent me know...
*/
//End Copyright Notice

#include "spotMapperWindow.h"
#include "../../button/arrowButton.h"
#include "../../deltaViewer.h"
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QString>
#include <QPoint>
#include <QColor>
#include <iostream>
#include <fstream>

using namespace std;

SpotMapperWindow::SpotMapperWindow(DeltaViewer* dv, int gw, int gh, int gd, int texSize, QWidget* parent)
    : QWidget(parent)
{
    textureSize = texSize;
    globalWidth = gw;
    globalHeight = gh;
    globalDepth = gd;
    sectionNo = gd / 2;  // default to the middle slice
    currentNuclearPerimeter = -1;

    deltaViewer = dv;

    x_origin = y_origin = 0;   // then the 
    backgroundImage = 0;

    // Make the relevant widgets..
    plotter = new PerimeterPlotter(textureSize, this);
    connect(plotter, SIGNAL(mousePos(int, int)), this, SLOT(newMousePos(int, int)) );

    // The following are elements that are used to control the mapping process
    checksBox = new QHBoxLayout();   // insert this layout into the appropriate position lower down

    QLabel* repLabel = new QLabel("Repulse", this);
    QDoubleSpinBox* repBox = new QDoubleSpinBox(this);
    repBox->setRange(1, 100);
    repBox->setValue(20);

    QLabel* difLabel = new QLabel("Spread", this);
    QDoubleSpinBox* diffuserSelector = new QDoubleSpinBox(this);
    diffuserSelector->setRange(1, 100);
    diffuserSelector->setValue(20);
    
    QLabel* ssLabel = new QLabel("Step size", this);
    QDoubleSpinBox* stepSizeSelector = new QDoubleSpinBox(this);
    stepSizeSelector->setRange(1, 100);
    stepSizeSelector->setValue(20);

    QLabel* mfLabel = new QLabel("Min force", this);
    QDoubleSpinBox* minForceSelector = new QDoubleSpinBox(this);
    minForceSelector->setRange(0.01, 0.99);
    minForceSelector->setSingleStep(0.01);

    QLabel* sNoLabel = new QLabel("Max step no.", this);
    QSpinBox* stepNumberSelector = new QSpinBox(this);
    stepNumberSelector->setRange(1, 500);
    stepNumberSelector->setValue(200);

    spotMapper = new SpotMapper(stepNumberSelector->value(), stepSizeSelector->value(), diffuserSelector->value(), minForceSelector->value(), repBox->value());
    // and make the set of connections I need to make sure that everything is kept up to date:
    connect(stepNumberSelector, SIGNAL(valueChanged(int)), spotMapper, SLOT(setMaxSteps(int)) );
    connect(stepSizeSelector, SIGNAL(valueChanged(double)), spotMapper, SLOT(setStepSize(double)) );
    connect(diffuserSelector, SIGNAL(valueChanged(double)), spotMapper, SLOT(setSigma(double)) );
    connect(minForceSelector, SIGNAL(valueChanged(double)), spotMapper, SLOT(setLimit(double)) );
    connect(repBox, SIGNAL(valueChanged(double)), spotMapper, SLOT(setRepSigma(double)) );


    /// some labels and a margin input for the nearest neighbour mapper
    QLabel* nnLabel = new QLabel("Nearest Neighbor", this);
    QLabel* nnMarginLabel = new QLabel("Margin", this);
    QSpinBox* nnMargin = new QSpinBox(this);
    nnMargin->setRange(10, 200);
    nnMargin->setValue(60);
    nnMapper = new NearestNeighborMapper(nnMargin->value());
    connect(nnMargin, SIGNAL(valueChanged(int)), nnMapper, SLOT(setMargin(int)) );
    QPushButton* nnMapButton = new QPushButton("Map", this);
    connect(nnMapButton, SIGNAL(clicked()), this, SLOT(mapAllByNeighbor()) );
    
    QPushButton* nnMapOneButton = new QPushButton("Map One", this);
    connect(nnMapOneButton, SIGNAL(clicked()), this, SLOT(mapOneByNeighbor()) );

    QLabel* maxPerimeterLabel = new QLabel("Perimeter max D", this);
    QSpinBox* maxPerimeterD = new QSpinBox(this);
    maxPerimeterD->setRange(10, 200);
    maxPerimeterD->setValue(30);
    spotPerimeterMapper = new SpotPerimeterMapper(maxPerimeterD->value());
    connect(maxPerimeterD, SIGNAL(valueChanged(int)), spotPerimeterMapper, SLOT(setMaxDistance(int)) );

    QPushButton* mapPerimeterButton = new QPushButton("Map Perimeter", this);
    connect(mapPerimeterButton, SIGNAL(clicked()), this, SLOT(mapPerimeter()) );

    QPushButton* makeCellsButton = new QPushButton("Make Cells", this);
    connect(makeCellsButton, SIGNAL(clicked()), this, SLOT(makeCells()) );

    QLabel* nucleusLabel = new QLabel("Selected nucleus", this);
    selectedPerimeterLabel = new QLabel("----", this);

    QLabel* bLabel = new QLabel("Blob", this);
    blobSelector = new QSpinBox(this);
    blobSelector->setRange(0, 0);
    connect(blobSelector, SIGNAL(valueChanged(int)), this, SLOT(setBlob(int)) );
    selBlobLabel = new QLabel("----, ----", this);

    mouse_X_label = new QLabel("-----");
    mouse_Y_label = new QLabel("-----");

    QLabel* scaleLabel = new QLabel("Scale", this);
    QDoubleSpinBox* scaleBox = new QDoubleSpinBox(this);
    scaleBox->setRange(0.1, 10.0);
    scaleBox->setSingleStep(0.1);
    scaleBox->setValue(1.0);
    connect(scaleBox, SIGNAL(valueChanged(double)), plotter, SLOT(setScale(double)) );

    QLabel* mapLabel = new QLabel("Map", this);
    QPushButton* mapOne = new QPushButton("one", this);
    QPushButton* mapVis = new QPushButton("visible", this);
    QPushButton* mapAll = new QPushButton("all", this);
    connect(mapOne, SIGNAL(clicked()), this, SLOT(mapOneBlob()) );
    connect(mapVis, SIGNAL(clicked()), this, SLOT(mapAllVisible()) );
    connect(mapAll, SIGNAL(clicked()), this, SLOT(mapAllBlobs() ) );
    
    // and then a label for the perimeters.. -just to give some information at the moment.
    // later we can try to do something a little bit more complex like centering on the given perimeters
    perimeterLabel = new QLabel("No Perimeters Loaded", this);
    
    // and then the things for other stuff..
    ArrowButton* upButton = new ArrowButton(0, this);
    ArrowButton* downButton = new ArrowButton(180, this);
    ArrowButton* leftButton = new ArrowButton(270, this);
    ArrowButton* rightButton = new ArrowButton(90, this);

    connect(upButton, SIGNAL(clicked()), this, SLOT(goUp()) );
    connect(downButton, SIGNAL(clicked()), this, SLOT(goDown()) );
    connect(leftButton, SIGNAL(clicked()), this, SLOT(goLeft()) );
    connect(rightButton, SIGNAL(clicked()), this, SLOT(goRight()) );
    
    QLabel* posLabel = new QLabel("Position");
    positionLabel = new QLabel("0, 0");
    QLabel* h_delta = new QLabel("Horizontal");
    QLabel* v_delta = new QLabel("Vertical");
    // and then ..
    vertDeltaBox = new QSpinBox(this);
    horDeltaBox = new QSpinBox(this);
    vertDeltaBox->setRange(1, textureSize);
    horDeltaBox->setRange(1, textureSize);
    vertDeltaBox->setValue(textureSize);
    horDeltaBox->setValue(textureSize);


    // and then try to work out the layout for the stuff..
    QVBoxLayout* mainBox = new QVBoxLayout(this);
    mainBox->addWidget(plotter);
    mainBox->setStretchFactor(plotter, 10);
    QHBoxLayout* blobBox = new QHBoxLayout();
    mainBox->addLayout(blobBox);
    blobBox->addLayout(checksBox);
    blobBox->addStretch();

    blobBox->addWidget(repLabel);
    blobBox->addWidget(repBox);
    blobBox->addWidget(difLabel);
    blobBox->addWidget(diffuserSelector);
    blobBox->addWidget(ssLabel);
    blobBox->addWidget(stepSizeSelector);
    blobBox->addWidget(mfLabel);
    blobBox->addWidget(minForceSelector);
    blobBox->addWidget(sNoLabel);
    blobBox->addWidget(stepNumberSelector);

    blobBox->addWidget(mapLabel);
    blobBox->addWidget(mapOne);
    blobBox->addWidget(mapVis);
    blobBox->addWidget(mapAll);
    QHBoxLayout* perimeterBox = new QHBoxLayout();
    mainBox->addLayout(perimeterBox);
    perimeterBox->addStretch();
    perimeterBox->addWidget(perimeterLabel);
    /// make a gridlayout inside hboxlayout.. maybe .. 
    QHBoxLayout* positionBox = new QHBoxLayout();
    mainBox->addLayout(positionBox);
    QGridLayout* positionGrid= new QGridLayout();
    positionBox->addLayout(positionGrid);
    positionGrid->addWidget(upButton, 0, 1);
    positionGrid->addWidget(leftButton, 1, 0);
    positionGrid->addWidget(downButton, 2, 1);
    positionGrid->addWidget(rightButton, 1, 2);
    positionGrid->addWidget(posLabel, 0, 3);
    positionGrid->addWidget(positionLabel, 0, 4);
    positionGrid->addWidget(h_delta, 1, 3);
    positionGrid->addWidget(horDeltaBox, 1, 4);
    positionGrid->addWidget(v_delta, 2, 3);
    positionGrid->addWidget(vertDeltaBox, 2, 4);
    positionBox->addStretch();
    
    QVBoxLayout* lowerRightVBox = new QVBoxLayout();
    positionBox->addLayout(lowerRightVBox);
    QHBoxLayout* nnBox = new QHBoxLayout();
    lowerRightVBox->addLayout(nnBox);
    nnBox->addStretch();
    nnBox->addWidget(nnLabel);
    nnBox->addSpacing(2);
    nnBox->addWidget(nnMarginLabel);
    nnBox->addWidget(nnMargin);
    nnBox->addWidget(nnMapOneButton);
    nnBox->addWidget(nnMapButton);
    QHBoxLayout* nucleusBox = new QHBoxLayout();
    lowerRightVBox->addLayout(nucleusBox);
    nucleusBox->addStretch();
    nucleusBox->addWidget(maxPerimeterLabel);
    nucleusBox->addWidget(maxPerimeterD);
    nucleusBox->addWidget(mapPerimeterButton);
    nucleusBox->addWidget(makeCellsButton);
    nucleusBox->addSpacing(10);
    
    nucleusBox->addWidget(nucleusLabel);
    nucleusBox->addSpacing(3);
    nucleusBox->addWidget(selectedPerimeterLabel);
    lowerRightVBox->addStretch();
    QHBoxLayout* lowerRightHBox = new QHBoxLayout();
    lowerRightVBox->addLayout(lowerRightHBox);
    lowerRightHBox->addStretch();
    lowerRightHBox->addWidget(bLabel);
    lowerRightHBox->addWidget(blobSelector);
    lowerRightHBox->addWidget(selBlobLabel);
    lowerRightHBox->addWidget(scaleLabel);
    lowerRightHBox->addWidget(scaleBox);
    lowerRightHBox->addWidget(mouse_X_label);
    lowerRightHBox->addSpacing(3);
    lowerRightHBox->addWidget(mouse_Y_label);
    // at this point
    mouse_X_label->setFixedWidth(mouse_X_label->sizeHint().width());
    mouse_Y_label->setFixedWidth(mouse_Y_label->sizeHint().width()); // ?
    selBlobLabel->setFixedWidth(selBlobLabel->sizeHint().width() + 10);
    selectedPerimeterLabel->setFixedWidth(selectedPerimeterLabel->sizeHint().width());
}

void SpotMapperWindow::setPerimeters(vector<PerimeterSet> perimeters, float wl, int sno){
    cout << "setPerimeters " << endl;
    perSets = perimeters;
    waveLength = wl;
    sectionNo = sno;
    // then format the perimeterLabel and that's all we need to do for now
    QString label = QString("Perimeter Sets: %1  wavelength: %2   section: %3")
	.arg(perimeters.size())
	.arg(waveLength)
	.arg(sectionNo);
    perimeterLabel->setText(label);

    selectedPerimeters.resize(0);
    for(vector<PerimeterSet>::iterator it = perSets.begin(); it != perSets.end(); it++){
	for(vector<Perimeter>::iterator pit = (*it).selectedPerimeters.begin(); pit != (*it).selectedPerimeters.end(); pit++){
	    selectedPerimeters.push_back((*pit));
	}
    }
    currentNuclearPerimeter = -1;
    updatePerimeters(x_origin, y_origin);
    plotter->update();
    show();
}

void SpotMapperWindow::setBlobs(map<int, threeDPeaks*> Blobs){
    blobs = Blobs;
    // delete the current checkboxes if any..

    while(blobChecks.size()){
	map<int, QCheckBox*>::iterator it = blobChecks.begin();
	(*it).second->hide();
	delete (*it).second;
	blobChecks.erase(it);
    }
    // and make some new ones..
    QString label;
    for(map<int, threeDPeaks*>::iterator it=blobs.begin(); it != blobs.end(); it++){
	label.setNum((*it).first);
	QCheckBox* box = new QCheckBox(label, this);
	connect(box, SIGNAL(released()), this, SLOT(waveLengthSelectionChanged()) );
	blobChecks.insert(make_pair((*it).first, box));
	checksBox->addWidget(box);
	box->show();
    }
    // when setting blobs the first time, nothing will be present in the allBlobs or 
    // visBlobs since none of the wavelengths will be selected.. 
    // then resize the blobs
    visibleBlobs.resize(0);
    allBlobs.resize(0);
    selectedDrops.resize(0);
    
}

void SpotMapperWindow::waveLengthSelectionChanged(){
    // First check if there is a current threeDPoint and if so remember it.. 
    threeDPoint currentPoint;
    if((uint)blobSelector->value() < visibleBlobs.size() ){
	currentPoint = visibleBlobs[(uint)blobSelector->value()];
    }
    int sel_blob_index = 0;
    visibleBlobs.resize(0);
    allBlobs.resize(0);
    selectedDrops.resize(0);

    // and allocate new ones depending on whether we can see them or not..
    int x_min = x_origin; // * horDeltaBox->value();
    int y_min = y_origin; // * vertDeltaBox->value();
    int x_max = x_min + textureSize;
    int y_max = y_min + textureSize;
    /// Then check to see which blobs are going to fit within the visible view..
    for(map<int, threeDPeaks*>::iterator it = blobs.begin(); it != blobs.end(); it++){
	if(!blobChecks.count((*it).first)){
	    cerr << "SpotMapperWindow::waveLengthSelectionChanged no check box for the given wavelength index" << endl;
	    continue;
	}
	if(blobChecks[(*it).first]->isChecked()){
	    // then we go through and add all points to the allBlobs, and all visible points to the visibleBlobs struct
	    for(map<long, simple_drop>::iterator sit = (*it).second->simpleDrops.begin(); sit != (*it).second->simpleDrops.end(); sit++){
		allBlobs.push_back(threeDPoint((*sit).second.x, (*sit).second.y, (*sit).second.z, (*sit).second.id));
		selectedDrops.push_back(&(*sit).second);
		if((*sit).second.x > x_min && (*sit).second.x < x_max
		   &&
		   (*sit).second.y > y_min && (*sit).second.y < y_max){
		    visibleBlobs.push_back(allBlobs.back());
		    if(visibleBlobs.back() == currentPoint)
			sel_blob_index = visibleBlobs.size() - 1;
		}
	    }
	}
    }
    // and now simply set the range of the blobSelector..
    blobSelector->setRange(0, visibleBlobs.size() - 1);
    blobSelector->blockSignals(true);     // block the signals so that we do not trigger an update (we want
    blobSelector->setValue(sel_blob_index); 
    blobSelector->blockSignals(false);
    updateBlobs(x_origin, y_origin);
}


void SpotMapperWindow::goLeft(){
    if(x_origin - horDeltaBox->value() <= 0){
	if(x_origin)
	    updateBlobs(0, y_origin);
	return;
    }
    updateBlobs(x_origin - horDeltaBox->value(), y_origin);
}

void SpotMapperWindow::goRight(){
    if(x_origin + horDeltaBox->value() < globalWidth)
	updateBlobs(x_origin + horDeltaBox->value(), y_origin);
}

void SpotMapperWindow::goDown(){
    if(y_origin - vertDeltaBox->value() <= 0){
	if(y_origin)
	    updateBlobs(x_origin, 0);
	return;
    }
    updateBlobs(x_origin, y_origin - vertDeltaBox->value());
}

void SpotMapperWindow::goUp(){
    if(y_origin + vertDeltaBox->value() < globalHeight)
	updateBlobs(x_origin, y_origin + vertDeltaBox->value());
}

void SpotMapperWindow::updateBlobs(int x, int y){
    if(x != x_origin || y != y_origin || !backgroundImage)
	updateBackground(x, y);
    
    // if the position has changed I also need to go through and change the visible blobs as done initially in thingy.
    if(x != x_origin || y != y_origin){
	setPosLabel(x, y);
	updateVisibility(x, y);
	updatePerimeters(x, y);
    }
    x_origin = x;
    y_origin = y;
    // then go through and update the visibleBlobs scenario..
    vector<twoDPoint> tdp(visibleBlobs.size());
    for(uint i=0; i < visibleBlobs.size(); ++i)
	tdp[i] = twoDPoint(visibleBlobs[i].x - x, visibleBlobs[i].y - y, visibleBlobs[i].id);
    // uncomment the setPoints below once the plotter has implemented the function.
    plotter->setPoints(tdp);
    plotter->update();
}

void SpotMapperWindow::updateVisibility(int x, int y){
    threeDPoint currentPoint;
    if((uint)blobSelector->value() < visibleBlobs.size() ){
	currentPoint = visibleBlobs[(uint)blobSelector->value()];
    }
    int sel_blob_index = 0;
    visibleBlobs.resize(0);

    // and allocate new ones depending on whether we can see them or not..
    int x_max = x + textureSize;
    int y_max = y + textureSize;
    /// Then check to see which blobs are going to fit within the visible view..
    for(map<int, threeDPeaks*>::iterator it = blobs.begin(); it != blobs.end(); it++){
	if(!blobChecks.count((*it).first)){
	    cerr << "SpotMapperWindow::waveLengthSelectionChanged no check box for the given wavelength index" << endl;
	    continue;
	}
	if(blobChecks[(*it).first]->isChecked()){
	    // then we go through and add all points to the allBlobs, and all visible points to the visibleBlobs struct
	    for(map<long, simple_drop>::iterator sit = (*it).second->simpleDrops.begin(); sit != (*it).second->simpleDrops.end(); sit++){
		if((*sit).second.x > x && (*sit).second.x < x_max
		   &&
		   (*sit).second.y > y && (*sit).second.y < y_max){
		    visibleBlobs.push_back(threeDPoint((*sit).second.x, (*sit).second.y, (*sit).second.z, (*sit).second.id));
		    if(visibleBlobs.back() == currentPoint)
			sel_blob_index = visibleBlobs.size() - 1;
		}
	    }
	}
    }
    // and now simply set the range of the blobSelector..
    blobSelector->setRange(0, visibleBlobs.size() - 1);
    blobSelector->blockSignals(true);     // block the signals so that we do not trigger an update (we want
    blobSelector->setValue(sel_blob_index); 
    blobSelector->blockSignals(false);
    
//    updateBlobs(x_origin, y_origin); // this should not be necessary since updateVisibility is only called from updateBlobs
    setBlob(sel_blob_index, false);    // don't update since this will be taken care of by the caller of updateVisibilty
}

void SpotMapperWindow::updatePerimeters(int x, int y){
    int x_max = x + textureSize;
    int y_max = y + textureSize;
    // then check which perimeters fit within this area..
    // oh, this is ugly, I'll need to be a friend of the perimeter.. breaking down all the things
    // I shouldn't have
    vector<outlineData> prs;  // the perimeters we want to draw..
    for(vector<PerimeterSet>::iterator it = perSets.begin(); it != perSets.end(); it++){
      for(vector<Perimeter>::iterator pit = (*it).selectedPerimeters.begin(); pit != (*it).selectedPerimeters.end(); pit++){
	//	    cout << "We have here a case of pit->x and others is this bad ? (*pit).minX : " << (*pit).minX << endl;
	//	    cout << "\t\tpit->xmin() " << pit->xmin() << endl;
	if(pit->xmin() < x_max && pit->xmax() > x
	   &&
	   pit->ymin() < y_max && pit->ymax() > y){
	  outlineData od(pit->xmin(), pit->xmax(), pit->ymin(), pit->ymax());
	  int xp, yp;
	  for(uint i=0; i < (*pit).length(); ++i){
	    (*pit).pos(i, xp, yp);
	    od.points.push_back(twoDPoint(xp-x, yp-y));
	  }
	  // loop below replaced by one above
	  // for(vector<int>::iterator vit=(*pit).perimeter.begin(); vit != (*pit).perimeter.end(); vit++){
	  //   od.points.push_back(twoDPoint((*vit) % pit->globalWidth - x, (*vit) / pit->globalWidth - y));
	  // }
	  prs.push_back(od);
	}
      }
    }
    plotter->setLines(prs);
}

void SpotMapperWindow::updateBackground(int x, int y){
    if(!backgroundImage){
	cout << "creating new background image" << endl;
	backgroundImage = new float[textureSize * textureSize * 3];
    }
    // and call dv with the appropriate thingy.. 
    memset((void*)backgroundImage, 0, textureSize * textureSize * 3 * sizeof(float));
    cout << "calling readToRGB with : " << x << ", " << y << ", " << sectionNo << " (" << textureSize << ")" << endl;
//    if(!deltaViewer->readToRGBPro(backgroundImage, x, y, (uint)textureSize, (uint)textureSize)){
    if(! (deltaViewer->readToRGB(backgroundImage, x, y, (uint)textureSize, (uint)textureSize, (uint)sectionNo))){
	cerr << "SpotMapperWindow::updateBackground failed to read to : " << x_origin << ", " << y_origin << "  (" << textureSize << ")" << endl;
	return;
    }
    plotter->setBackground(backgroundImage, textureSize, textureSize);
}

void SpotMapperWindow::mapOneBlob(){
    // map the current blob, but let's now show yet
    // do this only in the context of the currently visisble blobs.. 
    if(!visibleBlobs.size())
	return;
    if(!(visibleBlobs.size() > (uint)blobSelector->value()) )
	return;
    // in which case we should be able to call :
    vector<threeDPoint> path;
    int blobNucleus;
    spotMapper->walkOnePoint(visibleBlobs[blobSelector->value()], visibleBlobs, blobNucleus, path, selectedPerimeters);
    vector<twoDPoint> pts;
    for(uint i=0; i < path.size(); ++i){
	cout << i << "  : " << path[i].xd << ", " << path[i].yd << ", " << path[i].zd << endl;
	pts.push_back(twoDPoint(path[i].x - x_origin, path[i].y - y_origin));
    }
    plotter->setPath(pts);
    plotter->update();
    cout << "Mapped one blob and the resulting path has a size of : " << path.size() << endl;
}

void SpotMapperWindow::mapAllVisible(){
    // map visible blobs..
}

void SpotMapperWindow::mapAllBlobs(){
    // map them all boys.. 
    if(!allBlobs.size() || !selectedPerimeters.size() )
	return;
    vector<int> nuclearIds = spotMapper->walkPoints(allBlobs, selectedPerimeters);
    // we now need some way of displaying these.. 
    // just set these to the map of selected blobs
    if(nuclearIds.size() != selectedDrops.size()){
	cerr << "SpotMapperWindow:mapAllBlobs ERROR ERROR selectedDrops.size() = " << selectedDrops.size() << "  but nuclearIds.size() = " << nuclearIds.size() << endl;
	return;
    }
    for(uint i=0; i < selectedDrops.size(); ++i){
	selectedDrops[i]->id = nuclearIds[i];
    }
    // and then make the system reload the drops with the appropriate ids..
    waveLengthSelectionChanged();
}

void SpotMapperWindow::mapAllByNeighbor(){
    if(!allBlobs.size() || !selectedPerimeters.size() )
	return;
    vector<int> nuclearIds = nnMapper->mapPoints(allBlobs, selectedPerimeters);
    // just set these to the map of selected blobs
    if(nuclearIds.size() != selectedDrops.size()){
	cerr << "SpotMapperWindow:mapAllBlobs ERROR ERROR selectedDrops.size() = " << selectedDrops.size() << "  but nuclearIds.size() = " << nuclearIds.size() << endl;
	return;
    }
    for(uint i=0; i < selectedDrops.size(); ++i){
	selectedDrops[i]->id = nuclearIds[i];
    }
    // and then make the system reload the drops with the appropriate ids..
    waveLengthSelectionChanged();
}

void SpotMapperWindow::mapOneByNeighbor(){
    // map the current blob, but let's now show yet
    // do this only in the context of the currently visisble blobs.. 
    cout << "mapOneByNeighbour : " << endl;
    if(!visibleBlobs.size()){
	cout << "no visible blobs " << endl;
	return;
    }
    if(!(visibleBlobs.size() > (uint)blobSelector->value()) ){
	cout << "blob selector value too high .. not good" << endl;
	return;
    }
    if(!selectedPerimeters.size()){
	cout << "No selected perimeters available " << endl;
	return;
    }
    vector<twoDPoint> pts = nnMapper->mapOnePoint(visibleBlobs[blobSelector->value()],visibleBlobs, selectedPerimeters);
    cout << "One point mapped to nucleus with id " << " and a total of " << pts.size() << "  segements"<< endl;
    //// the nc is only valid as long as the space isn't updated. Use only in this case..
    // we need to adjust the map..
    for(uint i=0; i < pts.size(); ++i){
	pts[i].x -= x_origin;
	pts[i].y -= y_origin;
    }
    cout << "calling plotter to set polypaths" << endl;
    plotter->setPolyPath(pts);  
    cout << "calling plotter to update" << endl;
    plotter->update();
    cout << "plottre updated" << endl;
}

void SpotMapperWindow::mapPerimeter()
{
    // first check if 
    cout << "SpotMapperWindow mapPerimeter currentNuclearPerimeter is : " << currentNuclearPerimeter << endl;
    if(currentNuclearPerimeter < 0 || currentNuclearPerimeter >= (int)selectedPerimeters.size())
	return;
    
    // then we need to find all the points associated with this particular perimeter, this can be done by going 
    // through all of the drops and including those which we have stuff with.. 
    vector<simple_drop*> points;
    for(uint i=0; i < selectedDrops.size(); ++i){
	if(selectedDrops[i]->id == currentNuclearPerimeter)
	    points.push_back(selectedDrops[i]);
    }
    cout << "calling findPerimeter with points of size : " << points.size() << endl;
    vector<Point> perimeterPoints;
    if(currentNuclearPerimeter >= 0 && currentNuclearPerimeter < (int)selectedPerimeters.size()){
	perimeterPoints = spotPerimeterMapper->findPerimeter(points, selectedPerimeters[currentNuclearPerimeter], 10);
    }else{
	cout << "currentlySelectedPerimeter does not seem to work very well.. " << endl;
	perimeterPoints = spotPerimeterMapper->findPerimeter(points);
    }
    // then somehow cause the perimeterPoints to be drawn.. hmm
    cout << "and obtained a vector of perimeter points of size : " << perimeterPoints.size() << endl;
    vector<twoDPoint> pts;
    for(uint i=0; i < perimeterPoints.size(); ++i)
	pts.push_back(twoDPoint(perimeterPoints[i].x - x_origin, perimeterPoints[i].y - y_origin));
    plotter->setPath(pts);
    plotter->update();
}

void SpotMapperWindow::makeCells()
{
    cells.resize(0);
    // Then make a reasonable structure for the points ..
    map<int, vector<simple_drop*> > cellDrops;
    int undeffed = 0;
    int totalDrops = 0;
    for(uint i=0; i < selectedDrops.size(); ++i){
	++totalDrops;
	if(selectedDrops[i]->id == -1){
	    ++undeffed;
	    continue;
	}
	cellDrops[selectedDrops[i]->id].push_back(selectedDrops[i]);
    }
    cout << "Assigned drops to structure. Total of " << totalDrops << "  drops found of which " << undeffed << "  were not assigned to any ids" << endl;
    ///
    /// The ids are supposed to refer to indices in the selectedPerimeters vector representing the 
    /// nuclear bodies, but this should be changed to something more robust at some point.
    
    // Then go through all the cellDrops, and make a cell for each one of them.. 
    for(map<int, vector<simple_drop*> >::iterator it = cellDrops.begin(); it != cellDrops.end(); it++){
	if(it->first < 0 || it->first >= (int)selectedPerimeters.size()){
	    cerr << "SpotMapperWindow::makeCells simple_drop has an unaccounted for id (" << it->first << ")" << endl;
	    continue;
	}
	vector<simple_drop*>& drops = it->second;
	vector<Point> cellPerimeter = spotPerimeterMapper->findPerimeter(drops, selectedPerimeters[it->first], 10);
	cout << "Made a perimeter for cell # " << it->first << endl;
	Cell cell(selectedPerimeters[it->first], cellPerimeter);
	for(uint i=0; i < drops.size(); ++i){
	    simple_drop* d = drops[i];
	    HybPoint hp(d->x, d->y, d->z, d->waveIndex, d->waveLength);
	    cell.addHybPoint(hp);
	}
	cells.push_back(cell);
	cout << "Made cell # thingy" << endl;
    }
    // next we want to write a little report for the thingy. I wonder how many things we can put into it...
    ofstream out("cells");   // make something easy to parse by a perl script..
    for(uint i=0; i < cells.size(); ++i){
	out << i 
	    << "\t" << cells[i].cellPerimeter.size() << "\t" << cells[i].nucleus.area()
	    << "\t" << cells[i].MeanPointDensity();
	map<int, float> wl = cells[i].waves();
	for(map<int, float>::iterator it = wl.begin(); it != wl.end(); it++){
	    out << "\t" << it->second << " : " << cells[i].pointNo(it->first) << " : " << cells[i].MeanPointDensity(it->first);
	}
	out << endl;
	// and then the actual points, or rather their densities. well, do I really need that ? 
    }
    out.close();
}

void SpotMapperWindow::setBlob(int bNo, bool update){
    // tell the plotter to highlight this blob
    // only visible blobs allowed..
    if(bNo < 0 || bNo >= (int)visibleBlobs.size()){
	cerr << "SpotMapperWindow::setBlob bad index no. : " << bNo << endl;
	return;
    }
    twoDPoint p(visibleBlobs[bNo].x - x_origin, visibleBlobs[bNo].y - y_origin);
    QColor c(200, 0, 200);
    map<twoDPoint, QColor> pc;
    pc.insert(make_pair(p, c));
    cout << "Setting BLOB with index " << bNo << "  at pos : " << p.x << ", " << p.y << endl;
    QString l = QString("%1, %2")
	.arg(p.x)
	.arg(p.y);
    selBlobLabel->setText(l);
    plotter->setHighlights(pc);
    if(update)
	plotter->update();
}

void SpotMapperWindow::setPosLabel(int x, int y){
    QString ls = QString("%1, %2")
	.arg(x)
	.arg(y);
    positionLabel->setText(ls);
}

void SpotMapperWindow::newMousePos(int x, int y){
    QString l;
    l.setNum(x);
    mouse_X_label->setText(l);
    l.setNum(y);
    mouse_Y_label->setText(l);
    // but also go through the visible blobs and if one is close enough then call that the visible blob..
    x += x_origin;
    y += y_origin;
    double xd = (double)x;
    double yd = (double)y;
    double minDist = 5.0;
    for(uint i=0; i < visibleBlobs.size(); ++i){
//	cout << "checking blob : " << i << visibleBlobs[i].xd << "," << visibleBlobs[i].yd << "  --> " << xd << " : " << yd << endl;
	if(sqrt( (xd - visibleBlobs[i].xd) * (xd - visibleBlobs[i].xd) + (yd - visibleBlobs[i].yd) * (yd - visibleBlobs[i].yd)) < minDist){
	    blobSelector->setValue(i);
//	    cout << "and blobSelector value is set.. " << endl;
	    break;
	}
    }
    for(uint i=0; i < selectedPerimeters.size(); ++i){
//	cout << "checkingp perimeter " << i << endl;
	if(selectedPerimeters[i].contains(x, y)){
//	    cout << "selectedPerimeter " << i << "  contains position : " << x << "," << y << endl;
	    // would be nice to draw this one, but what the hell... 
	    QString l;
	    l.setNum(i);
	    currentNuclearPerimeter = i;
	    selectedPerimeterLabel->setText(l);
	}
    }
}
