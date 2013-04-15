#ifndef NODE_SET_H
#define NODE_SET_H

#include <QString>
#include <vector>

class node_set {
 public:
  node_set();
  node_set(std::vector<QString> labels, std::vector<std::vector<float> > nodes);
  node_set(std::vector<QString> names, std::vector<std::vector<float> > nodes, std::vector<QString> col_labels);
  
  node_set distances();

  unsigned int n_size();
  unsigned int n_dim();
  std::vector<std::vector<float> > Nodes();
  std::vector<float> node(unsigned int n, QString* label=0);
  float value(unsigned int n, unsigned int d, QString* label=0);
  std::vector<QString> Labels();
  std::vector<QString> Col_labels();

  bool push_node(QString label, std::vector<float> v);
  bool set_node(unsigned int n, std::vector<float> v);
  bool set_col_header(std::vector<QString>& ch);
  
 private:
  void init();
  float e_distance(std::vector<float>& a, std::vector<float>& b);

  std::vector<QString> labels;
  std::vector<std::vector<float> > nodes;
  std::vector<QString> col_labels;
  unsigned int dimensions;
};


#endif
