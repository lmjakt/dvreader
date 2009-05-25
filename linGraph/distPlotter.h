#ifndef DISTPLOTTER_H
#define DISTPLOTTER_H

#include <QWidget>
#include <QColor>
#include "linePlotter.h"
#include <vector>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>

class DistPlotter : public QWidget
{
    Q_OBJECT
	public:
    DistPlotter(bool useLimits, QWidget* parent=0);
    ~DistPlotter();
  
    void setData(std::vector<std::vector<float> >& v, std::vector<QColor>& c, bool resetLimits);
    void setLog(bool l);

    public slots:
	void toggleLog();
    void displayPos(int xp, float yp);
    void setPlotLimits(float l, float r, bool updatePlot);
    
 signals:
    void setLimits(float, float);

    private slots:
    void setCellNo(int dno);
    void setMinCell(double v);
    void setMaxCell(double v);
    void setLeftLimit(int x, float y);
    void setRightLimit(int x, float y);

 private:

    LinePlotter* linePlotter;
    QLabel* xPos;
    QLabel* yPos;

    QDoubleSpinBox* minValueBox;
    QDoubleSpinBox* maxValueBox;
    QSpinBox* cellNoBox;

    std::vector<std::vector<float> > values;
    std::vector<std::vector<float> > logValues;
    std::vector<std::vector<float> > counts;  // float rather than int for linePlotter's sake
    std::vector<QColor> colors;
    
    float leftLimit, rightLimit;

    unsigned int divNo;                       // the division no (or bucket number).
    float min, max;
    bool isLog;                               // just convert all the values back and forth using exp() and log().

    void initCount(bool resetLimits=true);
    void countItems();
    void setLogValues();
    //    void setLinValues();
    float translate_xpos(int x);
    unsigned int cell_pos(float v);
    void zero_counts();
};

#endif
