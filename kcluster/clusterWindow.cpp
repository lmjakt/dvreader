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

#include "../stat/stat.h"
#include "clusterWindow.h"
#include <qpushbutton.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <iostream>


using namespace std;

ClusterWindow::ClusterWindow(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    plotter = new PlotWidget(this, "plotter");
    shapePlotter = new PlotWidget(this, "shapePlotter");
    dropSection = new SpotSection(this, "dropSection");

    clusterProcess = 0;       // important so we know if we've got one or not

    generateColors(2);    // shouldn't use the constant there, but ..
    lineNumber = 0;
    
    indLine = new QSpinBox(0, 0, 1, this, "indLine");
    connect(indLine, SIGNAL(valueChanged(int)), this, SLOT(plotSingleLine(int)) );
    QLabel* indLabel = new QLabel("Individual Line", this, "indLabel");
    
    QVBoxLayout* vbox = new QVBoxLayout(this);
    QHBoxLayout* lowBox = new QHBoxLayout();
    vbox->addWidget(plotter);
    vbox->addLayout(lowBox);
    QVBoxLayout* controlBox = new QVBoxLayout();
    QVBoxLayout* shapeBox = new QVBoxLayout();
    lowBox->addLayout(controlBox);
    lowBox->addLayout(shapeBox);
    cwBox = new QVBoxLayout();
    lowBox->addLayout(cwBox);
    QHBoxLayout* spinbox = new QHBoxLayout();
    controlBox->addStretch();
    controlBox->addLayout(spinbox);
    spinbox->addWidget(indLabel);
    spinbox->addWidget(indLine);
    shapeBox->addWidget(shapePlotter);
    shapeBox->addWidget(dropSection);
    
    vbox->setStretchFactor(plotter, 2);
    vbox->setStretchFactor(lowBox, 3);
    
    //  lowBox->setStretchFactor(controlBox, 1);
    lowBox->setStretchFactor(shapeBox, 3);
    //lowBox->setStretchFactor(cwBox, 1);
}

ClusterWindow::~ClusterWindow(){
    if(clusterProcess){
	delete clusterProcess;
    }
}

void ClusterWindow::generateColors(int l){
    colors.resize(0);    
    int m = 255;
    l--;    // because we want the number of levels, not the numberr of things..
    //  cout << "generate Colors l is : " << l << endl;
    l < 1 ? l=1 : l=l;
    for(int i=0; i <= l; i++){
	for(int j=0; j <= l; j++){
	    for(int k=0; k <= l; k++){
		if(i || j || k){
		    // assign a colour..
		    int r = (i * m) / l;
		    int g = (j * m) / l;
		    int b = (k * m) / l;
		    colors.push_back(QColor(r, g, b));
		}
	    }
	}
    }
    cout << "total of : " << colors.size() << " colors generated" << endl;
}

void ClusterWindow::setClusters(KClusterProcess* clusterPr){
    if(clusterProcess){
	delete clusterProcess;
    }
    clusterProcess = clusterPr;
    connect(clusterProcess, SIGNAL(finished()), this, SLOT(setClusters()) );
    clusterProcess->start();
    
}

void ClusterWindow::setClusters(){
    vector<Cluster> clusters = clusterProcess->return_clusters();
    // and then let's remove any old clusterWidgets that we might have..
    for(uint i=0; i < clusterWidgets.size(); i++){
	delete(clusterWidgets[i]);
    }
    clusterWidgets.resize(0);
    // we also want to make sure we don't have anything in the plot maps ..
    plot_members.erase(plot_members.begin(), plot_members.end());
    plot_centers.erase(plot_centers.begin(), plot_centers.end());
    lineNumber = 0;

    for(uint i=0; i < clusters.size(); i++){
	// let's call makeGaussianModel here..
	clusters[i].makeGaussianModel();
	// and then lets refine the model
//	clusters[i].makeGaussianModel();

	// we don't need to set a color here, as we'll do that in the plotting function, not too much of an issue I think..
	clusters[i].color = colors[i % colors.size()];
	ClusterWidget* widget = new ClusterWidget(i, clusters[i], this, "widget");
	cwBox->addWidget(widget);
	connect(widget, SIGNAL(plotMembers(int, vector<simple_drop>)), this, SLOT(plotMembers(int, vector<simple_drop>)) );
//	connect(widget, SIGNAL(plotMembers(int, vector<vector<float> >)), this, SLOT(plotMembers(int, vector<vector<float> >)) );
	connect(widget, SIGNAL(plotCenter(int, vector<float>)), this, SLOT(plotCenter(int, vector<float>)) );
	connect(widget, SIGNAL(unPlotMembers(int)), this, SLOT(unPlotMembers(int)) );
	connect(widget, SIGNAL(unPlotCenter(int)), this, SLOT(unPlotCenter(int)) );
	clusterWidgets.push_back(widget);
	widget->show();
    }
    cwBox->addStretch();
    emit drawClusters(clusters);    // which now all have some nice clusters set for themselves.. 
    // and that should be ok..
}

//void ClusterWindow::plotMembers(int id, vector<vector<float> > members){
void ClusterWindow::plotMembers(int id, vector<simple_drop> members){
    // basically insert into the map and call plot..
    cout << "ClusterWindow plotMembers : " << id << "  size : " << members.size() << endl;
    plot_members.insert(make_pair(id, members));
    lineNumber += members.size();
    plot();
}

void ClusterWindow::plotCenter(int id, vector<float> center){
    cout << "ClusterWindow plotCenter : " << id << " size of center : " << center.size() << endl;
    plot_centers.insert(make_pair(id, center));
    lineNumber++;
    plot();
}

void ClusterWindow::unPlotMembers(int id){
    cout << "ClusterWindow unPlotMembers id " << id << endl;
    map<int, vector<simple_drop> >::iterator it = plot_members.find(id);
    if(it != plot_members.end()){
	lineNumber -= (*it).second.size();
	plot_members.erase(it);
	cout << " found and removing, lineNumber is now " << lineNumber << endl;
	plot();
    }
}


void ClusterWindow::unPlotCenter(int id){
    cout << "ClusterWindow unPlotCenter : " << id << endl;
    map<int, vector<float> >::iterator it = plot_centers.find(id);
    if(it != plot_centers.end()){
	plot_centers.erase(id);
	lineNumber--;
	plot();
    }
}

void ClusterWindow::plot(){
    cout << "ClusterWindow plot function lineNumber " << lineNumber  << endl;
    cout << "\tmember size  " << plot_members.size() << "\tcenter size " << plot_centers.size() << endl;
    // go through plot_members and plot_centers and create an appropriate
    // vector<vector<float> >
    // and an appropriate vector<QColor> which gives lines from different clusters different
    // colours..
    // plot thet the centers last of all.. so that it is given on top. We might want to do something like
    // allow the setting of the thickness of the line, but will check that later on.. 
    vector<vector<float> > values;   // these are the ones that we'll be using, don't know the size, so..
    vector<QColor> lineColors;
    values.reserve(lineNumber);
    lineColors.reserve(lineNumber);
    int lineLength = 0;
    plot_drops.resize(0);
    // since the lineNumber might be rather large...
    // oke dokey.. 
    map<int, vector<simple_drop> >::iterator it;
    for(it = plot_members.begin(); it != plot_members.end(); it++){
	QColor col = colors[(*it).first % colors.size()];  // the color of the thingy that we want.. 
	vector<simple_drop>::iterator vit;
	for(vit = (*it).second.begin(); vit != (*it).second.end(); vit++){
	    if(!lineLength){
		lineLength = (*vit).values.size();
	    }
	    if(lineLength != (*vit).values.size()){
		cerr << "inappropariate line length " << (*vit).values.size() << "  but should be : " << lineLength << endl;
		continue;
	    }
	    values.push_back((*vit).values);
	    plot_drops.push_back(*vit);
	    lineColors.push_back(col);
	}
    }
    // and then we set the centers..
    map<int, vector<float> >::iterator cit;
    for(cit = plot_centers.begin(); cit != plot_centers.end(); cit++){
	if(!lineLength){
	    lineLength = (*cit).second.size();
	}
	if(lineLength != (*cit).second.size()){
	    cerr << "inappropriate line length for a center index : " << (*cit).first << "  is : " << (*cit).second.size() << "  but should be " << lineLength << endl;
	    continue;
	}
	values.push_back((*cit).second);
	lineColors.push_back(colors[(*cit).first % colors.size()]);
    }
    // and then just call the function in the thingy..
    plotter->setColors(lineColors);
    plotter->setValues(values);
//    plot_lines = values;
    
    indLine->setRange(0, plot_drops.size()-1);
    cout << "colour size : " << lineColors.size() << "\tvalues : " << values.size() << endl;
    // and we are done..
}
    
void ClusterWindow::plotSingleLine(int lno){
    cout << "ClusterWindow plotSingleLine " << lno << endl;
    if(lno < 0 || lno > (int)plot_drops.size()){
	cerr << "lno is too large " << endl;
	return;
    }
    // make a fake vector of vector..
    vector<vector<float> > temp(1);

    temp.push_back(plot_drops[lno].values);

    // and then calculate a gaussian model for the thingy..
    int radius = plot_drops[lno].radius;
    float tv = plot_drops[lno].sumValue;   
    float xc = (float)plot_drops[lno].x;
    float yc = (float)plot_drops[lno].y;
    float zc = (float)plot_drops[lno].z;

    float xvar, yvar, zvar, mean;
    vector<float> xl = plot_drops[lno].centralLine(0);    
    vector<float> yl = plot_drops[lno].centralLine(1);
    vector<float> zl = plot_drops[lno].centralLine(2);
    
    dist_stats(mean, xvar, xl, 0, (float)xl.size());
    dist_stats(mean, yvar, yl, 0, (float)yl.size());
    dist_stats(mean, zvar, zl, 0, (float)zl.size());

//    float xvar = plot_drops[lno].xVariance;
//    float yvar = plot_drops[lno].yVariance;
//    float zvar = plot_drops[lno].zVariance;

//     float xvar = plot_drops[lno].gm_xvar;
//     float yvar = plot_drops[lno].gm_yvar;
//     float zvar = plot_drops[lno].gm_zvar;
    // which is enough to actually calculate the values..
    int sl = radius * 2 + 1;
    int SL = sl * sl;
    vector<float> gm_model(SL * sl, 0);
    cout << "xvar : " << xvar << "\tyvar : " << yvar << "\tzvar : " << zvar << "\tcenter : " << xc << "," << yc << "," << zc << endl;
    for(int z=-radius; z <= radius; z++){
	int zp = z + radius;
	float zd = (float(zp) + 0.5) - zc;
	zd = zd * zd;
	for(int y=-radius; y <= radius; y++){
	    int yp = y + radius;
	    float yd = (float(yp) + 0.5) - yc;
	    yd = yd * yd;
	    for(int x=-radius; x <= radius; x++){
		int xp = x + radius;
		float xd = (float(xp) + 0.5) - xc;
		xd = xd * xd;
		gm_model[zp * SL + yp * sl + xp] = tv * exp(-xd/xvar) * exp(-yd/yvar) * exp(-zd/zvar);
//		cout << zp << "," << yp << "," << xp << "\t" << tv << "\t" << xd << "\t" << yd << "\t" << zd << "\t" << gm_model[zp * SL + yp * sl + xp] << endl;
		//float mv = pv * exp(-xd/xVar) * exp(-yd/yVar) * exp(-zd/zVar);
		//float ratio = values[zp * Sl + yp * sl + xp] / mv;   // i.e. the number by which we have to multiply the thingy.. 
		//ratios.push_back(ratio);     // which we'll then use to identify the thingy with.. 
	    }
	}
    }
    temp.push_back(gm_model);
//    temp.push_back(plot_drops[lno].gaussian_values);
    plotter->setValues(temp);
    cout << "set main plotter to plot values " << endl;

    // and plot the shapes..
    vector<vector<float> > shapes(3);

    shapes[0] = xl; //plot_drops[lno].centralLine(0);
    shapes[1] = yl; //plot_drops[lno].centralLine(1);
    shapes[2] = zl; //plot_drops[lno].centralLine(2);
    
    cout << "set the shapes " << endl;

 //    float xMean, xVar, yMean, yVar, zMean, zVar;
//     float diam = float(shapes[0].size());
//     dist_stats(xMean, xVar, shapes[0], 0, diam);
//     dist_stats(yMean, yVar, shapes[1], 0, diam);
//     dist_stats(zMean, zVar, shapes[2], 0, diam);

    cout << "and got the dist_stats " << endl;
    
    float var = 2.5;    // the variance we use in the smoothening.. (just a guess for now.. ) 
    int smult = 10;
    int smSize = shapes[0].size() * smult;
    vector<vector<float> > smCurves(9);
    for(uint i=0; i < 6; i += 2){
//    for(uint i=0; i < smCurves.size(); i += 2){
	smCurves[i].resize(smSize);
	smCurves[i+1].resize(smSize);
	// i is the curve just extended, i+1 is the smoothened curve.. 
	for(uint j=0; j < smSize; j++){
	    smCurves[i][j] = shapes[i/2][j / smult];
	    smCurves[i+1][j] = 0;
	    for(uint k=0; k < shapes[i/2].size(); k++){
		// then calculate the position 
		float d = (float(k) + 0.5) - float(j)/float(smult);
		d = d * d;
		smCurves[i+1][j] += (shapes[i/2][k] * exp(-d/var));
	    }
	}
    }
    cout << "before calling resize on smCurves" << endl;

    smCurves[6] = xl; //plot_drops[lno].xcline;
    smCurves[7] = yl; //plot_drops[lno].ycline;
    smCurves[8] = zl; //plot_drops[lno].zcline;

//    cout << "Calcluated center position is : " << plot_drops[lno].x_center << "," << plot_drops[lno].y_center << ","  << plot_drops[lno].z_center << endl;

//     smCurves[6].resize(smSize);
//     smCurves[7].resize(smSize);
//     smCurves[8].resize(smSize);
//     cout << "smCurves resied up to 8 for smSize : " << smSize << endl;
//     for(uint i=0; i < smSize; i++){
// 	// set the values for the model,, 
// 	// though, unfortunately I don't know the peak position or the peak value.. hmm
// 	// use the mean and the std_deviation,, 
	
// 	// don't bother scaling either..
// 	smCurves[6][i] = exp(-( (float(i)/float(smult) - xMean) * (float(i)/float(smult) - xMean) )/xVar );
// 	smCurves[7][i] = exp(-( (float(i)/float(smult) - yMean) * (float(i)/float(smult) - yMean) )/yVar );
// 	smCurves[8][i] = exp(-( (float(i)/float(smult) - zMean) * (float(i)/float(smult) - zMean) )/zVar );
//     }
    

//     shapes[0] = plot_drops[lno].x_peak_dist;
//     shapes[1] = plot_drops[lno].y_peak_dist;
//     shapes[2] = plot_drops[lno].z_peak_dist;

//     shapes[0] = plot_drops[lno].x_peak_shape;
//     shapes[1] = plot_drops[lno].y_peak_shape;
//     shapes[2] = plot_drops[lno].z_peak_shape;

//     shapes[0] = plot_drops[lno].x_shape;
//     shapes[1] = plot_drops[lno].y_shape;
//     shapes[2] = plot_drops[lno].z_shape;
    vector<QColor> scols(9);
    scols[0] = QColor(255, 0, 0);
    scols[2] = QColor(0, 255, 0);
    scols[4] = QColor(0, 0, 255);
    scols[1] = QColor(255, 0, 0);
    scols[3] = QColor(0, 255, 0);
    scols[5] = QColor(0, 0, 255);
    scols[6] = QColor(255, 0, 0);
    scols[7] = QColor(0, 255, 0);
    scols[8] = QColor(0, 0, 255);

    cout << "calling shapePlotter setColors" << endl;
    shapePlotter->setColors(scols);
    cout << "calling shapePlotter set Values " << endl;
    shapePlotter->setValues(smCurves);
//    shapePlotter->setValues(shapes);
    dropSection->setDrop(plot_drops[lno]);
}
