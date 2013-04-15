#include "node_set.h"
#include <QFile>
#include <iostream>
#include <math.h>

node_set::node_set()
  : dimensions(0)
{
}

node_set::node_set(std::vector<QString> labels, std::vector<std::vector<float> > nodes)
  : labels(labels), nodes(nodes), dimensions(0)
{
  init();
}

node_set::node_set(std::vector<QString> labels, std::vector<std::vector<float> > nodes, std::vector<QString> col_labels)
  : labels(labels), nodes(nodes), col_labels(col_labels), dimensions(0)
{
  init();
}

// calculates a set of distances from the positions specified in pos
node_set node_set::distances()
{
  std::vector<std::vector<float> > dist(nodes.size());
  for(unsigned int i=0; i < nodes.size(); ++i){
    dist[i].resize( nodes.size() );
    for(unsigned int j=0; j < nodes.size(); ++j)
      dist[i][j] = e_distance(nodes[i], nodes[j]);
  }
  node_set dist_set(labels, dist);
  return(dist_set);
}

void node_set::init()
{
  // make sure that the vector sizes are ok..
  if(nodes.size() != labels.size()){
    nodes.resize(0);
    labels.resize(0);
    return;
  }
  if(!nodes.size()) return;
  dimensions = nodes[0].size();
  if(col_labels.size() != dimensions)
    col_labels.resize(0);
  for(unsigned int i=0; i < nodes.size(); ++i){
    if(nodes[i].size() != dimensions){
      dimensions = 0;
      nodes.resize(0);
      labels.resize(0);
      return;
    }
  }
}

float node_set::e_distance(std::vector<float>& a, std::vector<float>& b)
{
  float d = 0;
  if(a.size() != b.size()){
    std::cerr << "node_set::e_distance a and b have different sizes, calculating partial distance" 
	      << a.size() << " != " << b.size() << std::endl;
  }
  unsigned int k = a.size() < b.size() ? a.size() : b.size();
  for(unsigned int i=0; i < k; ++i)
    d += (a[i] - b[i]) * (a[i] - b[i]);
  d = sqrt(d);
  return(d);
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

std::vector<QString> node_set::Col_labels()
{
  return(col_labels);
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

bool node_set::set_col_header(std::vector<QString>& ch)
{
  if(ch.size() != dimensions)
    return(false);
  col_labels = ch;
  return(true);
}
