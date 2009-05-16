#ifndef DISTPLOTTER_H
#define DISTPLOTTER_H

#include <QWidget>
#include <QColor>
#include "linePlotter.h"
#include <vector>
#include <QLabel>

class DistPlotter : public QWidget
{
    Q_OBJECT
	public:
    DistPlotter(QWidget* parent=0);
    ~DistPlotter();

    void setData(std::vector<std::vector<float> >& v, std::vector<QColor>& c, bool lg=false);
    void setLog(bool l);

    public slots:
	void toggleLog();
    void displayPos(int xp, float yp);
    
 private:
    LinePlotter* linePlotter;
    QLabel* xPos;
    QLabel* yPos;

    std::vector<std::vector<float> > values;
    std::vector<std::vector<float> > counts;  // float rather than int for linePlotter's sake
    std::vector<QColor> colors;

    unsigned int divNo;                       // the division no (or bucket number).
    float min, max;
    bool isLog;                               // just convert all the values back and forth using exp() and log().

    void initCount();
    void setLogValues();
    void setLinValues();
};

#endif
