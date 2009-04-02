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

#ifndef CLUSTERWINDOW_H
#define CLUSTERWINDOW_H

#include <qcolor.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <map>
#include <vector>
#include "kClusterProcess.h"
#include "clusterWidget.h"
#include "../spotFinder/spotSection.h"
#include "../linGraph/plotWidget.h"
#include <qwidget.h>
#include <qlayout.h>
#include <qspinbox.h>

using namespace std;

class ClusterWindow : public QWidget
{
    Q_OBJECT
	public :
	ClusterWindow(QWidget* parent=0, const char* name=0);
    ~ClusterWindow();
    
    public slots :
	void setClusters(KClusterProcess* clusterPr);   // which needs to be destroyed if we get a new one.. 
    
    private slots :
	void setClusters();
	void plotMembers(int id, vector<simple_drop> members);
//	void plotMembers(int id, vector<vector<float> > members);
    void plotCenter(int id, vector<float> center);
    void unPlotMembers(int id);
    void unPlotCenter(int id);
    void plotSingleLine(int lno);

    signals :
	void drawClusters(vector<Cluster>&);

    private :
	void plot();        // work out what to plot from the appropriate set of values..
    void generateColors(int l);  // set colors for plotting. l = divs or levels of the primary colours.. 
    
    PlotWidget* plotter;
    PlotWidget* shapePlotter;
    SpotSection* dropSection;
    KClusterProcess* clusterProcess;    // initially this is set to 0. when we get a new one, delete if we have one..
    QSpinBox* indLine;                  // plot individual lines.. 
    vector<ClusterWidget*> clusterWidgets;
    map<int, vector<simple_drop> > plot_members;   // the members that we want to plot.
//    map<int, vector<vector<float> > > plot_members;   // the members that we want to plot.
    map<int, vector<float> > plot_centers;   // the centers that we want to plot
    //vector<vector<float> > plot_lines;       // keep track of this to draw individual lines (wasteful, but who cares).. 
    vector<simple_drop> plot_drops;
    int lineNumber;   // try to keep track of the line numer
    vector<QColor> colors;              // the colors associate with each plot.. 

    QVBoxLayout* cwBox;                      // the box for the clusterWidgets (cw);
};
#endif
