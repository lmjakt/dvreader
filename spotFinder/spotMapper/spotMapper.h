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

#ifndef SPOTMAPPER_H
#define SPOTMAPPER_H

#include <QObject>
#include <vector>
#include "../../dataStructs.h"
#include "../perimeter.h"

class SpotMapper : public QObject
{
    Q_OBJECT
    
	public :
	SpotMapper(int ms, double ss, double s, double l, double rs);
          // I'm not really sure whether or not it makes much sense to put these functions into a separate class
    ~SpotMapper(){}
    
    bool walkOnePoint(threeDPoint& point, std::vector<threeDPoint>& points, int& nucleus, std::vector<threeDPoint>& path, std::vector<Perimeter>& nuclei);
    std::vector<int> walkPoints(std::vector<threeDPoint>& points, std::vector<Perimeter>& nuclei); // walks all points vs each other and assigns ids .. (defaulting to -1)

    public slots :

	void setStepSize(double ss){
	stepSize = ss;
    }
    void setSigma(double s){
	sigma = s;
	setMaxD();
    }
    void setLimit(double l){
	limit = 1.0 - l;
	setMaxD();
    }
    void setMaxSteps(int ms){
	maxStep = ms;
    }
    void setRepSigma(double rs){
	repSigma = rs;
    }

    private :
	void nextStep(threeDPoint& point, std::vector<threeDPoint>& points, std::vector<threeDPoint>& path, std::vector<Perimeter>& nuclei);
    bool checkNuclei(threeDPoint& point, std::vector<Perimeter>& nuclei, int& nucleus);
    void calculateVector(threeDPoint& a, threeDPoint& b, double& dx, double& dy, double& dz, double m, double s); // use sigma, a is -1 or +1 depending on the point being attractive or repulsive
    void setMaxD();

    int maxStep;
    double stepSize, sigma, repSigma, maxD, limit;  // sigma is a spread coordinate that we'll use.. 
};

#endif
