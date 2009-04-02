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

//Copyright Notice
/*
    eXintegrator integrated expression analysis system
    Copyright (C) 2004  Martin Jakt & Okada Mitsuhiro
  
    This file is part of the eXintegrator integrated expression analysis system. 
    eXintegrator is free software; you can redistribute it and/or modify
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

#include "compareController.h"
//#include "traceDrawer.h"
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3filedialog.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

CompareController::CompareController(QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  
  int labelMargin = 4;
  QLabel* fullLabel = new QLabel("Full Comparisons", this, "fullLabel");
  QLabel* flatLabel = new QLabel("Flat Comparisons", this, "flatLabel");
  fullLabel->setAlignment(Qt::AlignCenter);
  flatLabel->setAlignment(Qt::AlignCenter);   // font sizes.. 

  int minWidth = 75;
  flatSigma = new QDoubleSpinBox(this);
  flatSigma->setRange(0.01, 5);
  flatSigma->setSingleStep(0.01);
//  flatSigma = new FSpinBox(0.01, 5, 500, this, "flatSigma");
  flatSigma->setValue(0.75);
  flatSigma->setMinimumWidth(minWidth);
  QLabel* sigmaLabel = new QLabel("sigma", this, "sigmaLabel");
  flatOrder = new QDoubleSpinBox(this);
  flatOrder->setRange(0.5, 10);
  flatOrder->setSingleStep(0.1);
//  flatOrder = new FSpinBox(0.5, 10, 21, this, "flatSigma");
  flatOrder->setValue(4);
  flatOrder->setMinimumWidth(minWidth);
  QLabel* orderLabel = new QLabel("order", this, "orderLabel");

  QPushButton* fullButton = new QPushButton("Compare", this, "fullButton");
  QPushButton* flatButton = new QPushButton("Compare", this, "flatButton");
  connect(fullButton, SIGNAL(clicked()), this, SLOT(compareFull()) );
  connect(flatButton, SIGNAL(clicked()), this, SLOT(compareFlat()) );

  QPushButton* readPhylipButton = new QPushButton("Read Phylip Distances", this, "readPhylipButton");
  connect(readPhylipButton, SIGNAL(clicked()), this, SLOT(readPhylipDistances()) );

//  QPushButton* traceButton = new QPushButton("Trace Experiments", this, "traceButton");
//  connect(traceButton, SIGNAL(clicked()), this, SLOT(trace()) );
//  traceSigma = new FSpinBox(0.01, 10, 500, this, "traceSigma");
//  traceSigma->setFValue(2.0);
//  traceSigma->setMinimumWidth(minWidth);
//  QLabel* traceSigmaLabel = new QLabel("Sigma Mult.", this, "traceSigmaLabel");

  makeRecord = new QCheckBox("Record Distances", this, "makeRecord");

  fullBox = new QVBoxLayout();
  flatBox = new QVBoxLayout();
  phylipBox = new QVBoxLayout();
//  traceBox = new QVBoxLayout();

  QVBoxLayout* vbox = new QVBoxLayout(this, 3, 2);
  vbox->addWidget(fullLabel);
  QHBoxLayout* fullControls = new QHBoxLayout();
  vbox->addLayout(fullControls);
  fullControls->addStretch();
  fullControls->addWidget(fullButton);
  vbox->addLayout(fullBox);    // put new distanceViewer widgets into here.
  
  vbox->addWidget(flatLabel);
  QHBoxLayout* flatControls = new QHBoxLayout();
  vbox->addLayout(flatControls);
  flatControls->addWidget(sigmaLabel);
  flatControls->addWidget(flatSigma);
  flatControls->addWidget(orderLabel);
  flatControls->addWidget(flatOrder);
  flatControls->addStretch();
  flatControls->addWidget(flatButton);
  vbox->addLayout(flatBox);
  vbox->addWidget(readPhylipButton);
  vbox->addLayout(phylipBox);
//  QHBoxLayout* traceControls = new QHBoxLayout(vbox);
//  traceControls->addWidget(traceSigmaLabel);
//  traceControls->addWidget(traceSigma);
//  traceControls->addStretch();
//  traceControls->addWidget(traceButton);
//  vbox->addLayout(traceBox);
  vbox->addWidget(makeRecord);
}

CompareController::~CompareController(){
  cout << "Deleteing Compare Controller" << endl;
}

void CompareController::newDistances(objectDistanceInfo info){
  QString name;
  if(info.isFlat){
    name = "Flat Comparison";
  }else{
    name = "Full Comparison";
  }
  
  cout << "compare Controller, got new Distances " << endl;

  if(makeRecord->isChecked()){
    recordDistances(info);
  }
  cout << "making distance viewer, " << endl;
  DistanceViewer* viewer = new DistanceViewer(info.experiments, info.values, name, this);
  cout << "viewer made " << endl;
  connect(viewer, SIGNAL(compareCells(vector<int>, vector<int>)), this, SIGNAL(compareCells(vector<int>, vector<int>)) );
  connect(viewer, SIGNAL(setCoordinates(vector<PointCoordinate>)), this, SIGNAL(setCoordinates(vector<PointCoordinate>)) );
  connect(viewer, SIGNAL(deleteMe()), this, SLOT(deleteSender()) );
  if(info.isFlat){
    flatDists.insert(viewer);
    flatBox->addWidget(viewer);
  }else{
    fullDists.insert(viewer);
    fullBox->addWidget(viewer);
  }
  viewer->show();
}

// void CompareController::newTrace(vector<tracePoint> points){
//   for(uint i=0; i < points.size(); i++){
//     cout << "id: " << points[i].id << "\tdistance: " << points[i].distance << "\tx: " << points[i].x << "\ty: " << points[i].y << endl;
//   }
//   // and do nothing more fore now.. ..
//   // ok let's just have a look at it..

//   TraceViewer* viewer = new TraceViewer(points, this, "viewer");
//   traces.insert(viewer);
//   traceBox->addWidget(viewer);
//   connect(viewer, SIGNAL(deleteMe()), this, SLOT(deleteSender()) );
//   viewer->show();
//   //  TraceDrawer* drawer = new TraceDrawer(points);
//   //  drawer->show();
// }

void CompareController::readPhylipDistances(){
  // first get a file name and check if it exists..
  QString infile = Q3FileDialog::getOpenFileName();
  if(infile.isNull()){
    cerr << "Null File name am just ignoring.. " << endl;
    return;
  }
  // read file and parse it using // -- Qt methods or ?? maybe thingy methods.. QString is convenient.. 
  // as there's a split function.. .. but I have to think how to use it.. 
  ifstream in(infile.latin1());
  if(!in){
    cerr << "couldn't open file for reading .. " << endl;
    return;
  }
  int mNum;   // the number of members with distances associated.. usually proteins or genes or something like that..
  int i, o;   // inner and outer counters.. in order to make sure everything is ok..
  vector<string> memberNames;
  vector<int> memberInts;    // just use the counter.. we can't assume these will be integers.. -- get translation at some later stage..
  vector<vector<float> > values;
  string name;
  float value;
  if(!(in >> mNum)){
    cerr << "couldn't get the mNum" << endl;
    return;
  }
  cout << "member number is : " << mNum << endl;
  i = 0;
  values.resize(mNum);   // hmmm,,, should check for reasonable number.. but ahh.. user's responsibility.. 
  while(i < mNum && in >> name){
    memberNames.push_back(name);
    memberInts.push_back(i);
    o = 0;
    values[i].resize(mNum);
    while(o < mNum && in >> value){
      values[i][o] = value;
      o++;
    }
    if(o != mNum){
      cerr << "o has wrong number should be : " << mNum << "  but in fact is : " << o << endl;
      return;
    }
    i++;
  }
  if(i != mNum){
    cerr << "i should be equal to : " << mNum << " but has become " << i << "   bugger : " << endl;
    return;
  }
  // which at this state if everything went well I can make a thingy... and insert.. but I need to make some more stuff.. first..
  //
  DistanceViewer* viewer = new DistanceViewer(memberInts, values, infile, this);
  phylipDists.insert(viewer);
  phylipBox->addWidget(viewer);
  viewer->show();
  // but first let's just print out some stuff and see how things went..
//   cout << "Printing out values found in " << infile << "  with a total of " << mNum << "  members " << endl;
//   for(int i=0; i < values.size(); i++){
//     cout << memberNames[i] << "\t";
//     for(int j=0; j < values[i].size(); j++){
//       cout << values[i][j] << "  ";
//     }
//     cout << endl;
//   }
//   cout << endl;


}
  
void CompareController::recordDistances(objectDistanceInfo& info){
  QString fileName = Q3FileDialog::getSaveFileName();
  if(fileName.isNull()){
    cerr << "No file name obtained " << endl;
    return;
  }
  ofstream out(fileName.latin1());
  if(!out){
    cerr << "Couldn't open " << fileName.latin1() << endl;
    return;
  }
  fileName.append("_table");
  ofstream out2(fileName.latin1());
  if(!out2){
    cerr << "Couldn't open " << fileName.latin1() << endl;
    return;
  }
  
  // then just print out the values in the thingy .. -- should check the values and things, but..
  if(info.experiments.size() != info.values.size()){
    cerr << "Values and experiments are different sizes can't really do much about that. " << endl;
    return;
  }
  for(int i=0; i < info.experiments.size(); i++){
    out2 << i + 1;
    for(int j=0; j < info.values[i].size() && j < info.experiments.size(); j++){
      out << i + 1 << "\t" << j+1 << "\t" << info.experiments[i] << "\t" << info.experiments[j] << "\t" << info.values[i][j] << "\t" << info.values[i][j]* info.values[i][j] << "\t"
	  << info.values[i][j]/info.geneNo << "\t" << (info.values[i][j]* info.values[i][j])/info.geneNo << endl ;
      out2 << "\t" << info.values[i][j] << "\t" <<  info.values[i][j]*info.values[i][j] << "\t" << info.values[i][j]/info.geneNo << "\t" <<  (info.values[i][j]*info.values[i][j])/info.geneNo;
    }
    out << endl;
    out2 << endl;
  }
  // and that's it really.. 
}

void CompareController::compareFull(){
//  emit doFullCompare();
    cout << "emitting doFlatCompare with 0, 0" << endl;
    emit doFlatCompare(0, 0);  // impossible numbers so we can just say do if flat.. 
}

// void CompareController::trace(){
//   emit traceExperiments(traceSigma->fvalue());
// }

void CompareController::compareFlat(){
  emit doFlatCompare(flatSigma->value(), flatOrder->value());
}

void CompareController::deleteSender(){
  const QObject* obj = sender();
  delete obj;
}
  

  
