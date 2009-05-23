#ifndef LINEPLOTTER_H
#define LINEPLOTTER_H

#include <QWidget>
#include <QColor>
#include <QString>
#include <vector>

typedef unsigned int uint;

class LinePlotter : public QWidget
{
    Q_OBJECT
	public:
    LinePlotter(QWidget* parent=0);
    ~LinePlotter();
    
    void setData(std::vector< std::vector<float> >& v, std::vector<QColor>& c);
   
 signals:
    void doubleClicked();
    void mousePos(int, float);
    void ctl_left(int, float);
    void ctl_mid(int, float);
    void ctl_right(int, float);
    void ctl_unknown(int, float);
    
    private:
    void paintEvent(QPaintEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void emitMousePos(int x, int y);
    void translateMousePos(QMouseEvent* e, int& x, float& y);

    std::vector< std::vector<float> > values;
    std::vector<QColor> colors;
    float min, max;
    float xScale;
    float yScale;
    unsigned int maxLength;

    int vMargin;
    int hMargin;
    int tick_spacing;
    int tick_length;
};

#endif
