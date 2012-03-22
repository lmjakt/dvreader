#ifndef SODCONTROLLER_H
#define SODCONTROLLER_H

#include <QWidget>
#include <vector>
#include <map>
#include <deque>
#include "node_set.h"
#include "../imageBuilder/f_parameter.h"

class DistanceViewer;
class CLineEdit;
class QPlainTextEdit;

class SodController : public QWidget
{
  Q_OBJECT
    public:
  SodController(QWidget* parent=0);
  ~SodController();

 private slots:
  void parseInput();
  void repeatCommand(bool up);

 private:
  // members
  CLineEdit* input;
  QPlainTextEdit* output;
  std::deque<QString> commandHistory;
  unsigned int commandPos, historySize;
  std::map<QString, DistanceViewer*> distanceViewers;
  
  // data members that we can do stuff with
  std::map<QString, node_set> positions;
  std::map<QString, node_set> distances;

  // member functions
  void general_command(QString line);
  typedef void (SodController::*g_function)(f_parameter& par);
  std::map<QString, g_function> general_functions;

  // functions taking a f_parameter
  void read_distances(f_parameter& par);
  void read_positions(f_parameter& par);
  void read_annotation(f_parameter& par);
  void set_plot_par(f_parameter& par);
  void titrate(f_parameter& par);
  void make_gaussian_background(f_parameter& par);
  void postscript(f_parameter& par);
  void svg(f_parameter& par);
  void drawGrid(f_parameter& par);

  // and some useful functions
  void warn(QString message);
  node_set read_node_set(QString fileName, bool has_col_header=FALSE);

};

#endif
