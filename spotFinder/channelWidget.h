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

#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <qwidget.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <QCheckBox>

class ChannelWidget : public QWidget
{
    Q_OBJECT

	public :
	ChannelWidget(int ID, QString Ident, QColor c, QWidget* parent=0, const char* name=0);
    bool setState(QString ident, int Id, int ws, float mpv, int mev, int minCorr, int r, int g, int b, int k, int bgm, bool expFile); 
    void currentState(QString& ident, int& Id, int& ws, float& mpv, int& mev, int& minCorr, int& r, int& g, int& b, int& k, int& bgm, bool& expFile); 
    
    private :
	int id;
    QString identifier;
    QSpinBox* windowSize;     // defines the peak width.. (or rather the window size used for looking for peaks)
    QLineEdit* minPeakValue;  // the minimum peak value (lineedit as the range could be very large..)
    float mpValue;
    QSpinBox* maxEdgeValue;   // the maximum edge value (i.e. should be low). This given as a percentage, so easy to use a spinbox
    QSpinBox* minCorrelation; // minimum correlation coefficient to some idealised profile (express as a percentage)
    QSpinBox* scaleFactor;    // sets the scale factor for the line edge (10 = 1.0, set between 0 -> 1000);

    QCheckBox* fileCheckBox;  // export spots to a file.. 

    QSpinBox* backgroundMultiplier;

    QSpinBox* clusterNumber;   // if we want to do a clustering of the spots .. 
    // we also would like to include a colour chooser to indicate how the identified spots should be displayed..
    QPushButton* colorButton;

    // and then some variables that we need to remember...
    QColor currentColor;   // which is the colour of the button.. 


    private slots :
	void findSpots();      // read the values and emit the appropriate signal.
    void findAllSpots();
    void findAllSpots3D();
    void pvChanged();  // handle and emit the appropriate signal
    void meChanged(int me);
    void sfChanged(int sf);   
    void changeColor();
    
    signals :             // these all take the form, -identifier, followed by some value..
	// the first three of these are primarily given so that we can display the results of these values in the plot
	void newPeakValue(int, float);     
    void newMaxEdgeValue(int, float);      
    void newScaleFactor(int, float);
    // and then one for doing the actual spotfinding..
    void findspots(int, int, float, float, float, int, int, int);  // id, wsize, minpeakvalue, maxEdgeValue, minCorrelation, r, g, b,
    void findallspots(int, int, float, float, float, int, int, int, int, float, bool);  // find spots in all the slices..  -- bool is export to file or not
    void findallspots3D(int, int, float, float, float, int, int, int, int, float, bool);  // find spots in all the slices..  -- bool is export to file or not



};
    

#endif
