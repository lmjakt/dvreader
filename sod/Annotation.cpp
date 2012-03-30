#include "Annotation.h"
#include <iostream>
#include <set>

Annotation::Annotation()
{
  init();
}

Annotation::Annotation(node_set ns)
  : annotation(ns)
{
  init();
}

QColor Annotation::node_color(unsigned int n, QString ch)
{
  if(n >= annotation.n_size() || !column_map.count(ch))
    return( null_color );
  unsigned int ci = column_map[ch];
  if(!color_maps.count(ci))
    return( null_color );
  float v = annotation.value(n, ci);
  if(!color_maps[ ci ].count(v) )
    return( null_color );
  return( color_maps[ci][v] );
}

// returns true if the point matches the values in fv
bool Annotation::filter(unsigned int n, QString ch, std::set<float> fv)
{
  if(n >= annotation.n_size() || !column_map.count(ch))
    return(false);
  return( fv.count( annotation.value(n, column_map[ch]) ) );
}

unsigned int Annotation::n_size()
{
  return(annotation.n_size());
}

bool Annotation::has_column(QString ch)
{
  for(std::map<QString, uint>::iterator it=column_map.begin(); it != column_map.end(); ++it)
    std::cout << "has_column:  " << (*it).first.ascii() << " --> " << (*it).second << std::endl;
  return( column_map.count(ch) );
}

void Annotation::init(){
  base_colors = generateColors();
  null_color = QColor(75, 75, 75);
  std::vector<QString> c_labels = annotation.Col_labels();
  if(!c_labels.size() || c_labels.size() != annotation.n_dim() )
    return;
  column_map.clear();
  for(uint i=0; i < c_labels.size(); ++i){
    column_map.insert(std::make_pair( c_labels[i], i ));
    std::set<float> levels;
    std::map<float, QColor> colors;
    for(uint j=0; j < annotation.n_size(); ++j)
      levels.insert( annotation.value(j, i) );
    unsigned int c = 0;
    for(std::set<float>::iterator it=levels.begin(); it != levels.end(); ++it){
      colors.insert( std::make_pair((*it), base_colors[ c % base_colors.size() ]) );
      ++c;
    }
    color_maps.insert( std::make_pair( i, colors ) );
  }
}

// for now use a statically defined list. But we should set some options for
// setting the colors.
// Should do something more clever with a loop and HSL specification.. 
std::vector<QColor> Annotation::generateColors()
{
  std::vector<QColor> colors;
  colors.push_back(QColor(255, 0, 0));  
  colors.push_back(QColor(0, 255, 0));  
  colors.push_back(QColor(0, 0, 255));  
  colors.push_back(QColor(255, 255, 0));
  colors.push_back(QColor(255, 0, 255));  
  colors.push_back(QColor(0, 255, 255));
  colors.push_back(QColor(125, 50, 0));  
  colors.push_back(QColor(205, 125, 53));
  colors.push_back(QColor(225, 125, 229));
  colors.push_back(QColor(165, 227, 140));
  colors.push_back(QColor(162, 160, 251));
  colors.push_back(QColor(254, 73, 113));
  return(colors);
}

