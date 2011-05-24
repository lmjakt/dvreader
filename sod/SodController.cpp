#include "SodController.h"
#include "distanceViewer.h"
#include "pointDrawer.h"
#include "../customWidgets/clineEdit.h"
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QString>
#include <QTextStream>
#include <QFile>

#include <iostream>
#include <string>
#include <fstream>

SodController::SodController(QWidget* parent)
  : QWidget(parent)
{
  input = new CLineEdit(this);
  output = new QPlainTextEdit(this);

  connect(input, SIGNAL(returnPressed()), this, SLOT(parseInput()) );

  QVBoxLayout* mainBox = new QVBoxLayout(this);
  mainBox->addWidget(output);
  mainBox->addWidget(input);

  // and the set of general functions
  general_functions["read_distances"] = &SodController::read_distances;
  general_functions["read_positions"] = &SodController::read_positions;
  general_functions["set_plot_par"] = &SodController::set_plot_par;
}

SodController::~SodController()
{
  for(std::map<QString, DistanceViewer*>::iterator it=distanceViewers.begin(); it != distanceViewers.end(); ++it)
    delete((*it).second);
  // delete other components?
  delete input;
  delete output;
  std::cout << "End of SodController destructor" << std::endl;
}

void SodController::parseInput()
{
  output->appendPlainText(input->text());
  general_command(input->text());
  input->clear();
}

void SodController::general_command(QString line)
{
  f_parameter par(line);
  if(!general_functions.count(par.function())){
    output->appendPlainText("Unknown command");    
    return;
  }
  g_function func = general_functions[ par.function() ];
  (*this.*func)(par);
}

void SodController::read_distances(f_parameter& par)
{
  QString error;
  QTextStream qts(&error);
  QString fileName;
  QString disName;
  //  std::string filename;
  if(!par.param("file", fileName))
    qts << "read_distances: no distance file specified\n";
  if(!par.param("dist", disName))
    qts << "read_distances: specify name for distances (dist=..)";
  if(error.length()){
    warn(error);
    return;
  }
  node_set ns = read_node_set(fileName);
  if(!ns.n_size()){
    warn("read_distances obtained empty node_set");
    return;
  }
  if(ns.n_size() != ns.n_dim()){
    qts << "read_distances node_set is not square: " << ns.n_size() << "*" << ns.n_dim();
    warn(error);
    return;
  }
  distances[disName] = ns;
  std::vector<int> memberInts(ns.n_size());
  for(unsigned int i=0; i < memberInts.size(); ++i)
    memberInts[i] = i;

  DistanceViewer* viewer = new DistanceViewer(memberInts, ns.Nodes(), fileName);
  if(distanceViewers.count(disName))
    delete distanceViewers[disName];

  distanceViewers[disName] = viewer;
  viewer->show();
  viewer->raise();
}


void SodController::read_positions(f_parameter& par)
{
  QString error;
  QTextStream qts(&error);
  QString filename;
  QString pos_name;
  if(!par.param("file", filename))
    qts << "read_distances: no distance file specified\n";
  if(!par.param("pos", pos_name) || pos_name.isEmpty())
    qts << "specify a name for the position set\n";
  if(error.length()){
    warn(error);
    return;
  }
  node_set ns = read_node_set(filename);
  if(!ns.n_size()){
    warn("read_positions obtained empty node_set");
    return;
  }
  positions[pos_name] = ns;
  
  bool set_mapper = true;
  par.param("set_mapper", set_mapper);
  if(set_mapper && distanceViewers.count(pos_name))
    distanceViewers[pos_name]->setPositions(ns.Nodes());
  
}

void SodController::set_plot_par(f_parameter& par)
{
  QString dviewer;
  if(!par.param("viewer", dviewer)){
    warn("Specify viewer to be used viewer=(..)");
    return;
  }
  if(!distanceViewers.count(dviewer)){
    warn("Unknown viewer specified");
    return;
  }
  DistanceViewer* viewer = distanceViewers[dviewer];
  QString plot_type;
  if(par.param("type", plot_type)){
    PointDrawer::PointPlotType ppt = PointDrawer::STRESS;
    if(plot_type == "pie") ppt = PointDrawer::LEVELS_PIE;
    viewer->setPointPlotType(ppt);
  }
  int diameter;
  if(par.param("diameter", diameter))
    viewer->setPointDiameter(diameter);
  bool forces;
  if(par.param("forces", forces))
    viewer->drawForces(forces);
  
}

void SodController::warn(QString message)
{
  output->appendPlainText(message);
}

// The file should contain n rows and n+1 columns.
// The first column identifies the node and the
// following columns contain it's coordinates. (These
// can indicate all against all distances as well as
// spatial coordinates)..
node_set SodController::read_node_set(QString fileName)
{
  node_set ns;
  QString error;
  QTextStream qts(&error);
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    qts << "Unable to open file : " << fileName;
    warn(error);
    return(ns);
  }
  QTextStream in(&file);
  QString line;
  bool ok;
  while(!in.atEnd()){
    line = in.readLine();
    QStringList words = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if(words.size() < 2){
      warn("read_node_set skipping line");
      continue;
    }
    QString label = words[0];
    std::vector<float> coords;
    for(int i=1; i < words.size(); ++i){
      float f = words[i].toFloat(&ok);
      if(!ok) 
	continue;
      coords.push_back(f);
    }
    if(!ns.push_node(label, coords))
      warn("read_node_set failed to push node");
  }
  return(ns);
}
