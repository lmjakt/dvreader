#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QString>
#include <QColor>
#include <vector>
#include <map>
#include <set>
#include "node_set.h"

class Annotation {
 public:
  Annotation();
  Annotation(node_set ns);

  QColor node_color(unsigned int n, QString ch);
  bool filter(unsigned int n, QString ch, std::set<float> fv);
  unsigned int n_size();
  bool has_column(QString ch);

 private:
  node_set annotation;
  std::vector<QColor> base_colors;
  QColor null_color;

  std::map< QString, unsigned int > column_map;
  std::map< unsigned int, std::map<float, QColor> > color_maps;

  void init();
  std::vector<QColor> generateColors();
  std::map<float, QColor> generateParameterColors(std::set<float> levels, bool use_log=false);
};
#endif
