#ifndef QNT_COLORS_H
#define QNT_COLORS_H

#include <QColor>
#include <QString>
#include <vector>

// colors must be at least one larger than breaks 
// as a colour has to be defined either side of each break.

struct qnt_colors {
  qnt_colors();
  qnt_colors(QColor c);
  qnt_colors(QString pname, std::vector<QColor>& col, std::vector<float>& bks);

  void setColor(float v, float& r, float& g, float& b);
  void setColor(float v, unsigned char& r, unsigned char& g, unsigned char& b);

  QString par;
  std::vector<QColor> colors;
  std::vector<float> breaks;
};

#endif
