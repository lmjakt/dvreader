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

#include "spotWindow.h"
#include "spotDensityWidget.h"
#include "blurWidget.h"
#include <iostream>
//#include <qlayout.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <values.h>
#include <fstream>
#include <string>

using namespace std;

SpotWindow::SpotWindow(int Width, int Height, int Depth, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
  setContentsMargins(1, 1, 1, 1);
    imageWidth = Width;
    imageHeight = Height;
    imageDepth = Depth;
    xPlot = new PlotWidget(this, "xPlot");
    yPlot = new PlotWidget(this, "yPlot");

    int plotMinH = 150;
    xPlot->setMinimumHeight(plotMinH);
    yPlot->setMinimumHeight(plotMinH);

    connect(xPlot, SIGNAL(incrementImage(int)), this, SIGNAL(increment_x_line(int)) );
    connect(yPlot, SIGNAL(incrementImage(int)), this, SIGNAL(increment_y_line(int)) );

    modelWidget = new ModelWidget(imageWidth, imageHeight, imageDepth, this, "modelWidget");
    connect(modelWidget, SIGNAL(makeModel(int, int, int, int, int, int, set<int>)), 
	    this, SIGNAL(makeModel(int, int, int, int, int ,int, set<int>)) );
    connect(modelWidget, SIGNAL(recalculateSpotVolumes()), this, SIGNAL(recalculateSpotVolumes()) );

    nucleusWidget = new NucleusWidget("Find Nuclei", this, "nucleusWidget");
    connect(nucleusWidget, SIGNAL(findNuclearPerimeters(int, float)), this, SIGNAL(findNuclearPerimeters(int, float)) );
    
    contrastWidget = new NucleusWidget("Find Contrasts", this, "contrastWidget");
    connect(contrastWidget, SIGNAL(findNuclearPerimeters(int, float)), this, SIGNAL(findContrasts(int, float)) );
    
    blobMapperWidget = new NucleusWidget("Map Blobs", this);
    connect(blobMapperWidget, SIGNAL(findNuclearPerimeters(int, float)), this, SIGNAL(mapBlobs(int, float)) );

    setWidget = new SetWidget("Find Sets", this);
    connect(setWidget, SIGNAL(findSets(int, int, int, float)), this, SIGNAL(findSets(int, int, int, float)) );

    SpotDensityWidget* spotDensityWidget = new SpotDensityWidget(this);
    connect(spotDensityWidget, SIGNAL(findSpotDensity(int, double, double)), this, SIGNAL(findSpotDensity(int, double, double)) );

    blurWidget = new BlurWidget(this);
    connect(blurWidget, SIGNAL(blur(std::set<uint>, int, double, double)), this, SIGNAL(blur(std::set<uint>, int, double, double)) );

    QCheckBox* useProjectionBox = new QCheckBox("Use Projection Data", this, "useProjectionData");
    connect(useProjectionBox, SIGNAL(toggled(bool)), this, SIGNAL(setUseProjection(bool)) );

    QPushButton* saveParamButton = new QPushButton("Save Parameters", this);
    QPushButton* loadParamButton = new QPushButton("Read Parameters", this);

    connect(saveParamButton, SIGNAL(clicked()), this, SLOT(saveSpotParameters()) );
    connect(loadParamButton, SIGNAL(clicked()), this, SLOT(readSpotParameters()) );

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setSpacing(1);
    vbox->setContentsMargins(1, 1, 1, 1);
    vbox->addWidget(xPlot);
    vbox->addWidget(yPlot);
    channelBox = new QVBoxLayout();
    channelBox->setContentsMargins(0, 0, 0, 0);
    channelBox->setSpacing(1);
    vbox->addLayout(channelBox);
    QHBoxLayout* checkBox = new QHBoxLayout();
    checkBox->addStretch();
    checkBox->addWidget(saveParamButton);
    checkBox->addWidget(loadParamButton);
    checkBox->addWidget(useProjectionBox);
    vbox->addLayout(checkBox);
    vbox->addStretch();   // but this might be bad.. 
    vbox->addWidget(modelWidget);
    vbox->addWidget(nucleusWidget);
    vbox->addWidget(contrastWidget);
    vbox->addWidget(blobMapperWidget);
    vbox->addWidget(setWidget);
    vbox->addWidget(spotDensityWidget);
    vbox->addWidget(blurWidget);

    setMinimumWidth(500);
}

//void SpotWindow::resizeEvent(QResizeEvent* e){
// set the top part to be a set height.. 

void SpotWindow::setLineValues(int slicePosition, int dim, vector<vector<float> > v, vector<int> m, int LMargin, int RMargin, float MinY, float MaxY){
    // It would probably be considered better style to use a temporary pointer to the appropriate
    // object, but..
    switch(dim){
	case 1 :
	    yPosition = slicePosition;
	    xPlot->setPeaks(slicePeaks(slicePosition, dim));
	    xPlot->setValues(v, m, LMargin, RMargin, MinY, MaxY);
	    break;
	case 2:
	    xPosition = slicePosition;
	    yPlot->setPeaks(slicePeaks(slicePosition, dim));
	    yPlot->setValues(v, m, LMargin, RMargin, MinY, MaxY);
	    break;
	default :
	    cerr << "SpotWindow::setLineValues unknown dimension : " << dim << endl;
    }
}

void SpotWindow::setAuxLines(int dim, map<uint, vector<float> > auxLines){
    switch(dim){
	case 1 :
	    xPlot->setAuxLines(auxLines);
	    break;
	case 2 :
	    yPlot->setAuxLines(auxLines);
	    break;
	default:
	    cerr << "SpotWindow::setAuxLines unknown dimension : " << dim << endl;
    }
}

map<int, vector<int> > SpotWindow::slicePeaks(int slicePosition, int dim){
    map<int, vector<int> > peaks;
    map<int, linearPeaks>::iterator it;
    switch(dim){
	case 1 :
	    for(it = linePeaks.begin(); it != linePeaks.end(); it++){
		peaks.insert(make_pair((*it).first, (*it).second.x_line_peaks(slicePosition)));
	    }
	    break;
	case 2:
	    for(it = linePeaks.begin(); it != linePeaks.end(); it++){
		peaks.insert(make_pair((*it).first, (*it).second.y_line_peaks(slicePosition)));
	    }
	    break;
	default :
	    cerr << "unknown dimension " << endl;
    }
    return(peaks);
}
	    

void SpotWindow::setLineColors(vector<QColor> Colors){
    xPlot->setColors(Colors);
    yPlot->setColors(Colors);
}

void SpotWindow::setChannels(vector<QString> Channels){
    // do something..
    // Replace old version..
    // for(uint i=0; i < channelWidgets.size(); i++){
//	delete channelWidgets[i];
    //   }
//    channelWidgets.resize(0);
//    for(uint i=0; i < Channels.size(); i++){
    
    // make sure that we have enough norm_data ..
    norm_values.resize(Channels.size());

    cout << "\t\t\tSpotWindow setChannels begin channelWidgets size : " << channelWidgets.size() << "\tChannels size : " << Channels.size() << endl;
    for(uint i=channelWidgets.size(); i < Channels.size(); i++){
	cout << "\t\t\t" << i << endl;
	channelWidgets.push_back(new ChannelWidget(i, Channels[i], QColor(255, 255, 255), this));
	channelBox->addWidget(channelWidgets.back());
	connect(channelWidgets.back(), SIGNAL(newPeakValue(int, float)), this, SLOT(newPeakValue(int, float)) );
	connect(channelWidgets.back(), SIGNAL(newMaxEdgeValue(int, float)), this, SLOT(newMaxEdgeValue(int, float)) );
	connect(channelWidgets.back(), SIGNAL(newScaleFactor(int, float)), this, SLOT(newScaleFactor(int, float)) );
	connect(channelWidgets.back(), SIGNAL(findspots(int, int, float, float, float, int, int, int)), 
		this, SLOT(findSpots(int, int, float, float, float, int, int, int)) );
	connect(channelWidgets.back(), SIGNAL(findallspots(int, int, float, float, float, int, int, int, int, float, bool)), 
		this, SLOT(findAllSpots(int, int, float, float, float, int, int, int, int, float, bool)) );
	connect(channelWidgets.back(), SIGNAL(findallspots3D(int, int, float, float, float, int, int, int, int, float, bool)), 
		this, SLOT(findAllSpots3D(int, int, float, float, float, int, int, int, int, float, bool)) );
	channelWidgets.back()->show();
    }
    cout << "\t\t\tcalling set channels on modelwidget" << endl;
    modelWidget->setChannels(Channels);
    cout << "\t\t\tcalling setChannels on nucleus widget " << endl;
    nucleusWidget->setChannels(Channels);
    contrastWidget->setChannels(Channels);
    blobMapperWidget->setChannels(Channels);
    setWidget->setChannels(Channels);
    blurWidget->setChannels(Channels);
    cout << "\t\t\tstuff OK " << endl;
}

void SpotWindow::setNormalisations(vector<float*> areas, int width, int height){
    if(areas.size() != norm_values.size()){
	cerr << "SpotWindow::setNormalisations areas.size " << areas.size() << " != " << norm_values.size() << endl;
	for(uint i=0; i < areas.size(); ++i){
	    if(areas[i]){
		delete areas[i];
	    }
	}
	return;
    }
    int l = width * height;
    for(uint i=0; i < areas.size(); ++i){
	float sum = 0;
	float sumSq = 0;
	for(int j=0; j < l; ++j){
	    sum += areas[i][j];
	    sumSq += (areas[i][j] * areas[i][j]);
	}
	float SS = sumSq - (sum * sum)/float(l);
	SS = (SS == 0) ? MINFLOAT : SS;
	norm_values[i].setValues(sum/float(l), sqrt(SS/float(l - 1)) );
	if(areas[i]){
	    delete areas[i];
	}
    }
}




// that's pretty much all to begin with.. 
    
void SpotWindow::newPeakValue(int id, float pv){
    cout << "splotwindow new peak value " << id << "\t" << pv << endl;
    xPlot->setMinPeakValue((unsigned int)id, pv);
    yPlot->setMinPeakValue((unsigned int)id, pv);
}

void SpotWindow::newMaxEdgeValue(int id, float ev){
    cout << "spotwindow new max edge value " << ev << endl;
    xPlot->setMaxEdgeValue((unsigned int)id, ev);
    yPlot->setMaxEdgeValue((unsigned int)id, ev);
}

void SpotWindow::newScaleFactor(int id, float sf){
    xPlot->setScale((unsigned int)id, sf);
    yPlot->setScale((unsigned int)id, sf);
}

void SpotWindow::findSpots(int id, int wsize, float minPeakValue, float maxEdgeValue, float minCorrelation, int r, int g, int b){
    // and send a message to the powers that be..
    cout << "SpotWindow::findSpots called " << endl;
    emit findLocalMaxima(id, wsize/2, minPeakValue, maxEdgeValue);
}

void SpotWindow::findAllSpots(int id, int wsize, float minPeakValue, float maxEdgeValue, float minCorrelation, int r, int g, int b, int K, float bgm, bool exportFile){
    // and send a message to the powers that be..
    cout << "SpotWindow::findSpots called " << endl;
    emit findAllLocalMaxima(id, wsize/2, minPeakValue, maxEdgeValue, K, bgm, exportFile);
}

void SpotWindow::findAllSpots3D(int id, int wsize, float minPeakValue, float maxEdgeValue, float minCorrelation, int r, int g, int b, int K, float bgm, bool exportFile){
    // and send a message to the powers that be..
    cout << "SpotWindow::findSpots called " << endl;
    emit findAllLocalMaxima3D(id, wsize/2, minPeakValue, maxEdgeValue, K, bgm, exportFile);
}

void SpotWindow::find_spots3D(string p_file){
    readSpotParameters(p_file);
    // and then go through the various channelWidgets and get the appropriate numbers from them
    QString ident;
    int id, ws, mev, minCorr, r, g, b, k, bgm;
    bool expFile;
    float mpv;
    for(uint i=0; i < channelWidgets.size(); ++i){
	mpv = 0;
	channelWidgets[i]->currentState(ident, id, ws, mpv, mev, minCorr, r, g, b, k, bgm, expFile);
	if(mpv > 0)
	    emit findAllLocalMaxima3D(id, ws/2, mpv, (float)mev/100.0, k, (float)bgm/100.0, expFile);
    }
    // which should be enough.. 
}

void SpotWindow::saveSpotParameters(){
    QString s_file = QFileDialog::getSaveFileName(this, "Save Parameters", "", "Parameters (*.spp);;All (*.* *)");
    if(!s_file.length())
	return;
    
    // use ifstream since I'm more familiar with the syntax for it..
    ofstream out(s_file.latin1());
    if(!out){
	cerr << "Unable to open file for writing : " << s_file.latin1() << endl;
	return;
    }
    for(uint i=0; i < channelWidgets.size(); ++i){
	int id, wsize, mev, minCorr, r, g, b, k, bgm;
	float mpv;
	bool expfile;
	QString identifier;
	channelWidgets[i]->currentState(identifier, id, wsize, mpv, mev, minCorr, r, g, b, k, bgm, expfile);
	out << id << "\t" << identifier.latin1() << "\t" << wsize << "\t" << mpv << "\t" << mev << "\t" << minCorr 
	    << "\t" << r << "\t" << g << "\t" << b << "\t" << k << "\t" << bgm << "\t" << expfile << endl;
    }    
    out.flush();
    out.close();
}

void SpotWindow::readSpotParameters(){
    QString s_file = QFileDialog::getOpenFileName(this, "Load Parameters", "", "Parameters (*.spp);;All (*.* *)");
    if(!s_file.length())
	return;
    readSpotParameters(s_file.latin1());
}


void SpotWindow::readSpotParameters(string p_file){
    ifstream in(p_file.c_str());
    if(!in){
	cerr << "SpotWindow::readSpotParameters unable to read file : " << p_file << endl;
	return;
    }
    int id, wsize, mev, minCorr, r, g, b, k, bgm;
    float mpv;
    bool expfile;
    string identifier;
    
    while(in >> id){
	getline(in, identifier, '\t');  // this ends up reading a null string .. 
	getline(in, identifier, '\t');

	if(in >> wsize >> mpv >> mev >> minCorr >> r >> g >> b >> k >> bgm >> expfile){
	    if(id >= 0 && id < channelWidgets.size()){
		channelWidgets[id]->setState(identifier.c_str(), id, wsize, mpv, mev, minCorr, r, g, b, k, bgm, expfile);
	    }else{
		cerr << "bugger got a bad id "
		     << "  got identifier: " << identifier << " and an id of : " << id << " wsize " << wsize << " mpv " << mpv << "  mev " << mev << "  minCorr " << minCorr
		     << " r, g, b " << r << "," << g << "," << b << " k  " << k << "  bgm " << bgm << " expfile  " << expfile
		     << endl;
	    }
	}else{
	    cerr << "Unable to read in all the appropriate spot parameters giving up" 
		 << "  got identifier: " << identifier << " and an id of : " << id << " wsize " << wsize << " mpv " << mpv << "  mev " << mev << "  minCorr " << minCorr
		 << " r, g, b " << r << "," << g << "," << b << " k  " << k << "  bgm " << bgm << " expfile  " << expfile
		 << endl;
	    //break;
	}
    }
    
}

void SpotWindow::setPeaks(map<int, linearPeaks> Peaks){
    linePeaks = Peaks;
    cout << "setting new peak information .. " << endl;
//    for(map<int, linearPeaks>::iterator it = Peaks.begin(); it != Peaks.end(); it++){
//	cout << "\t" << (*it).first << "\t" << (*it).second.xPeaks.size() << "\t" << (*it).second.yPeaks.size() << endl;
//    }
    // and the set the appropriate things.. when updating the plotting information..
    xPlot->setPeaks(slicePeaks(yPosition, 1));
    yPlot->setPeaks(slicePeaks(xPosition, 2));
    xPlot->update();
    yPlot->update();
}
