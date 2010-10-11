#ifndef VALUELABELS_H
#define VALUELABELS_H

#include <QWidget>
#include <QColor>
#include <vector>

class QLabel;

class ValueLabels : public QWidget
{
  Q_OBJECT
    public:
  ValueLabels(std::vector<QColor>& colors, QWidget* parent=0);

  void setValues(std::vector<float> v);
  void setValue(unsigned int i, float v);
  void setColor(unsigned int i, QColor c);
 private:
  std::vector<QLabel*> labels;

};

#endif
