#include "Annotation.h"
#include <iostream>
#include <set>
#include <math.h>

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
  if(!color_maps[ ci ].count(v) ){
    std::cerr << "Unknown level for ci: " << ci << "  v: " << v << "    size of map: " << color_maps[ci].size() << std::endl;
    return( null_color );
  }
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
    
    if(levels.size() > 12){
      colors = generateParameterColors(levels); // assume linear scaling
    }else{
      unsigned int c = 0;
      for(std::set<float>::iterator it=levels.begin(); it != levels.end(); ++it){
	colors.insert( std::make_pair((*it), base_colors[ c % base_colors.size() ]) );
	++c;
      }
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

// Use HSV coloring to generate a color map float -> QColor

std::map<float, QColor> Annotation::generateParameterColors(std::set<float> levels, bool use_log)
{
  std::map<float, QColor> colors;
  if(!levels.size()){
    return(colors);
  }
  float min_value = *levels.begin();
  float max_value = *levels.rbegin();
  if(use_log && min_value <= 0){
    std::cerr << "generateParameterColors: use_log is true, but 0 or negative values present\n"
	      << "use_log forced to false" << std::endl;
    use_log = false;
  }
  min_value = use_log ? log(min_value) : min_value;
  max_value = use_log ? log(max_value) : max_value;
  float range = max_value - min_value;
  if(!range){
    colors.insert(std::make_pair(*levels.begin(), QColor(255, 0, 0))); // do not insert min_value as this would be the 
    return(colors);
  }
  QColor c;
  for(std::set<float>::iterator it=levels.begin(); it != levels.end(); ++it){
    float v = use_log ? log(*it) : *it;
    // we want to run colors from blue (240) -> purple (300)
    // via red (300) in the opposite direction.
    // we make the maximum value 300 since we do not want to run
    // blue --> blue (360 = full circle)
    int hue = (int)(300.0 * (v - min_value) / range);
    hue = (360 + (240 - hue)) % 360;
    c.setHsv(hue, 255, 220);
    colors.insert(std::make_pair(*it, c));
    std::cout << "Inserting color level: " << *it << "  HSV: " << hue << ",255,220" << std::endl;
  }
  return(colors);
}
