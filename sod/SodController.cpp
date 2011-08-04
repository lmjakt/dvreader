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

  commandPos = 0;
  historySize = 300;
  connect(input, SIGNAL(returnPressed()), this, SLOT(parseInput()) );
  connect(input, SIGNAL(arrowPressed(bool)), this, SLOT(repeatCommand(bool)) );

  QVBoxLayout* mainBox = new QVBoxLayout(this);
  mainBox->addWidget(output);
  mainBox->addWidget(input);

  // and the set of general functions
  general_functions["read_distances"] = &SodController::read_distances;
  general_functions["read_positions"] = &SodController::read_positions;
  general_functions["set_plot_par"] = &SodController::set_plot_par;
  general_functions["titrate"] = &SodController::titrate;
  general_functions["gaussian_bg"] = &SodController::make_gaussian_background;
  general_functions["postscript"] = &SodController::postscript;
  general_functions["draw_grid"] = &SodController::drawGrid;
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
  commandHistory.push_back(input->text());
  if(commandHistory.size() > historySize)
    commandHistory.pop_front();
  commandPos = commandHistory.size();
  general_command(input->text());
  input->clear();
}

void SodController::repeatCommand(bool up)
{
  if(up && commandPos > 0){
    input->setText( commandHistory[--commandPos] );
    return;
  }
  if(!up && commandPos < commandHistory.size() - 1){
    input->setText( commandHistory[++commandPos] );
    return;
  }
  input->setText("");
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

  bool trackCoordinates = true;
  par.param("coords", trackCoordinates);
  par.param("comps", trackCoordinates);

  DistanceViewer* viewer = new DistanceViewer(memberInts, ns.Nodes(), fileName, trackCoordinates);
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
  unsigned int grid_points = 0;
  if(!par.param("file", filename))
    qts << "read_distances: no distance file specified\n";
  if(!par.param("pos", pos_name) || pos_name.isEmpty())
    qts << "specify a name for the position set\n";
  if(error.length()){
    warn(error);
    return;
  }
  par.param("grid", grid_points);
  node_set ns = read_node_set(filename);
  if(!ns.n_size()){
    warn("read_positions obtained empty node_set");
    return;
  }
  positions[pos_name] = ns;
  
  bool set_mapper = true;
  par.param("set_mapper", set_mapper);
  if(set_mapper && distanceViewers.count(pos_name)){
    distanceViewers[pos_name]->set_starting_dimensionality(ns.n_dim());
    distanceViewers[pos_name]->setPositions(ns.Nodes(), grid_points);
  }
}

void SodController::set_plot_par(f_parameter& par)
{
  if(par.defined("help")){
    warn("set_plot_par viewer=vname [ plot_type=stress|pie diameter=i forces=bool scale=f move_factor=f threads=i interval=ms ids=bool");
    return;
  }
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
  float scale;
  if(par.param("scale", scale))
    viewer->setPlotScale(scale);
  float moveFactor;
  if(par.param("move_factor", moveFactor))
    viewer->setMoveFactor(moveFactor);
  unsigned int threads;
  if(par.param("threads", threads))
    viewer->setThreadNumber(threads);
  unsigned int drawInterval;
  if(par.param("interval", drawInterval))
    viewer->setDrawInterval(drawInterval);
  unsigned int update_interval;
  if(par.param("update_interval", update_interval))
    viewer->setUpdateInterval(update_interval);
  bool draw_ids;
  if(par.param("ids", draw_ids))
    viewer->drawIds(draw_ids);
  float stress_min, stress_max;
  if(par.param("stress_max", stress_max) && par.param("stress_min", stress_min))
    viewer->setStressRange(stress_min, stress_max);
  if(par.defined("stress_reset"))
    viewer->resetStressRange();
}

void SodController::titrate(f_parameter& par)
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
  unsigned int rep=10;
  std::vector<unsigned int> iter;
  std::vector<unsigned int> dim_no;
  QString error;
  QTextStream qts(&error);
  par.param("rep", rep);
  if(!par.param("iter", ',', iter))
    qts << "Please specify iteration numbers: iter=300,500, etc..\n";
  if(!par.param("dim_no", ',', dim_no))
    qts << "Please specify starting dimensionalities dim_no=6,4,2 or something\n";
  if(error.length()){
    warn(error);
    return;
  }
  std::vector<std::vector<float> > minStresses ;
  for(unsigned int i=0; i < dim_no.size(); ++i){
    for(unsigned int j=0; j < iter.size(); ++j)
      minStresses.push_back( viewer->runMultiple(rep, iter[j], dim_no[i]));
  }
  // and then let's try to print out a simple table..
  std::string out_file;
  std::ofstream out;
  if(par.param("ofile", out_file))
    out.open(out_file.c_str());

  for(unsigned int d=0; d < dim_no.size(); ++d){
    for(unsigned int it=0; it < iter.size(); ++it){
      std::cout << "\t" << dim_no[d] << "," << iter[it];
      if(out.is_open())
	out << "\t" << dim_no[d] << "," << iter[it];
    }
  }
  std::cout << std::endl;
  if(out.is_open())
    out << std::endl;
  for(unsigned int i=0; i < rep; ++i){
    std::cout << i;
    if(out.is_open())
      out << i;
    for(unsigned int d=0; d < dim_no.size(); ++d){
      for(unsigned int it=0; it < iter.size(); ++it){
	unsigned int j = d * iter.size() + it;
	std::cout << "\t" << minStresses[j][i];
	if(out.is_open())
	  out << "\t" << minStresses[j][i];
      }
    }
    std::cout << std::endl;
    if(out.is_open())
      out << std::endl;
  }
}


void SodController::make_gaussian_background(f_parameter& par)
{
  QString error;
  QTextStream qts(&error);
  QString dviewer;
  float var;
  std::vector<unsigned int> dims;
  std::vector<unsigned int> alpha;
  std::vector<unsigned int> red;
  std::vector<unsigned int> green;
  std::vector<unsigned int> blue;
  if(!par.param("viewer", dviewer))
    qts << "Specify viewer (viewer=..)\n";
  if(!dviewer.isEmpty() && !distanceViewers.count(dviewer))
    qts << "Unknown viewer " << dviewer << "\n";
  if(!par.param("dims", ',', dims))
    qts << "Specify dimensions to use (dims=a,b,c)\n";
  if(!par.param("a", ',', alpha) || !par.param("r", ',', red) || 
     !par.param("g", ',', green) || !par.param("b", ',', blue))
    qts << "Specify argb components (a=a,b,c r=a,b,c) one value (0-255) for each dim\n";
  if(!par.param("var", var))
    qts << "Specify the variance to use (radius will be 2xvar) (var=..)\n";
  if(error.length()){
    warn(error);
    return;
  }
  // there should be a viewer, but also there should be a positions of the same name
  if(!positions.count(dviewer)){
    qts << "No positions set for viewer " << dviewer << " aborting";
    warn(error);
    return;
  }
  // check that colours have been specified for all dimensions and that no value is
  // larger than 255
  if(dims.size() != alpha.size() || dims.size() != red.size() ||
     dims.size() != green.size() || dims.size() != blue.size()){
    warn("Colors not same size as specified by dim\n");
  }
  // check that none of the dimensions is too large..
  for(unsigned int i=0; i < dims.size(); ++i){
    if(dims[i] >= positions[dviewer].n_dim()){
      warn("dimension specified too large");
      return;
    }
    if(alpha[i] > 255 || red[i] > 255 || green[i] > 255 || blue[i] > 255){
      warn("Illegal color specified (value larger than 255)");
      return;
    }
  }
  // if that's ok..
  unsigned char* color_matrix = new unsigned char[4 * dims.size()];
  for(unsigned int i=0; i < dims.size(); ++i){
    color_matrix[i * 4 + 0] = alpha[i];
    color_matrix[i * 4 + 1] = red[i];
    color_matrix[i * 4 + 2] = green[i];
    color_matrix[i * 4 + 3] = blue[i];
  }
  // then make a function in the viewer..
  distanceViewers[dviewer]->set_simple_gaussian_background(dims, color_matrix, var);
}

void SodController::postscript(f_parameter& par)
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

  QString error;
  QTextStream qts(&error);
  float w, h;
  QString fname;
  if(!par.param("fname", fname))
    qts << "Specify postscript output file name fname=...\n";
  if(!par.param("w", w))
    qts << "Specify width of plot w=..\n";
  if(!par.param("h", h))
    qts << "Specify height of plot h=..\n";
  if(error.length()){
    warn(error);
    return;
  }
  bool stress = par.defined("stress");
  
  if(!par.defined("svg")){
    viewer->postscript(fname, w, h, stress);
  }else{
    viewer->svg(fname, (int)w, (int)h);
  }
}


void SodController::drawGrid(f_parameter& par)
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
  bool draw;
  if(!par.param("draw", draw)){
    warn("Specify as to whether to draw the grid or not (draw=true/false)");
    return;
  }
  distanceViewers[dviewer]->setGrid(draw);
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
