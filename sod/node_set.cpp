#include "node_set.h"
#include <QFile>
#include <iostream>

node_set::node_set()
  : dimensions(0)
{
}

node_set::node_set(std::vector<QString> labels, std::vector<std::vector<float> > nodes)
  : labels(labels), nodes(nodes), dimensions(0)
{
  // make sure that the vector sizes are ok..
  if(nodes.size() != labels.size()){
    nodes.resize(0);
    labels.resize(0);
    return;
  }
  if(!nodes.size()) return;
  dimensions = nodes[0].size();
  for(unsigned int i=0; i < nodes.size(); ++i){
    if(nodes[i].size() != dimensions){
      dimensions = 0;
      nodes.resize(0);
      labels.resize(0);
      return;
    }
  }
}

unsigned int node_set::n_size()
{
  return(nodes.size());
}

unsigned int node_set::n_dim()
{
  return(dimensions);
}

std::vector<std::vector<float> > node_set::Nodes()
{
  return(nodes);
}

// always returns a node. if n too large returns origin
std::vector<float> node_set::node(unsigned int n, QString* label)
{
  if(n < nodes.size()){
    if(label)
      (*label) = labels[n];
    return(nodes[n]);
  }
  std::vector<float> f(dimensions, 0);
  return(f);
}

float node_set::value(unsigned int n, unsigned int d, QString* label)
{
  if(n < nodes.size() && d < dimensions){
    if(label)
      (*label) = labels[n];
    return(nodes[n][d]);
  }
  return(0);
}

std::vector<QString> node_set::Labels()
{
  return(labels);
}

bool node_set::push_node(QString label, std::vector<float> v)
{
  if(!nodes.size())
    dimensions = v.size();
  if(v.size() != dimensions)
    return(false);
  nodes.push_back(v);
  labels.push_back(label);
  return(true);
}

bool node_set::set_node(unsigned int n, std::vector<float> v)
{
  if(n >= nodes.size() || v.size() != dimensions)
    return(false);
  nodes[n] = v;
  return(true);
}

