#ifndef IMAGEBUILDERWIDGET_H
#define IMAGEBUILDERWIDGET_H

#include <QWidget>
//#include <QLineEdit>
#include <QTextEdit>
#include <QString>
#include <QCryptographicHash>
#include <vector>
#include <map>
#include <string>
#include "imageBuilder.h"
#include "../customWidgets/clineEdit.h"
#include <deque>

struct channel_info;
class QTabWidget;
class QPlainTextEdit;

class ImageBuilderWidget: public QWidget
{
  Q_OBJECT
 public:
  ImageBuilderWidget(FileSet* fs, std::vector<channel_info> ch, QWidget* parent=0);
  ~ImageBuilderWidget();

  void setImage(float* image, int w, int h);
  void setGreyImage(float* image, int w, int h, bool destroy_source=true);  // for single channel images
  
 signals:
  void showCoverage(float);
  void setChooserMax(float);
  void getBlobModel(int, int); // xy_radius, z_radius

 private:
  std::string builder_dir;
  std::string builder_cmd_dir;
  std::set<QString> saved_commands;
  QCryptographicHash::Algorithm hashAlgorithm;

  void initialise();

  CLineEdit* input;
  std::deque<QString> commandHistory;
  unsigned int historySize;
  unsigned int commandPos;
  //  QLineEdit* input;
  QTabWidget* outputTabs;
  QPlainTextEdit* output;
  QPlainTextEdit* helpPage;
  QPlainTextEdit* commandPage;
  ImageBuilder* builder;
  
  typedef void (ImageBuilderWidget::*w_function)(std::vector<QString> words);
  typedef void (ImageBuilderWidget::*na_function)();
  std::map<QString, w_function> w_functions;
  std::map<QString, na_function> na_functions; // na = no argument

  //std::map<QString, Task> tasks;
  std::map<QString, QString> helpStrings;

  // the following functions parse the words within ImageBuilderWidget
  void parseInput(std::vector<QString> words);
  void setParameter(std::vector<QString> words);
  void makeSlice(std::vector<QString> words);
  void makeGaussianSlice(std::vector<QString> words);
  void makeProjection(std::vector<QString> words);
  void makeBGRSlice(std::vector<QString> words);
  void addBackground(std::vector<QString> words);
  void subBackground(std::vector<QString> words);
  void addMCP(std::vector<QString> words);
  void exportImage(std::vector<QString> words);
  void requestCoverage(std::vector<QString> words);
  void printHelp(std::vector<QString> words);
  void bg_spectrum(std::vector<QString> words);
  void paint_spectral_background(std::vector<QString> words);
  void pipe_slice(std::vector<QString> words);
  void setChooserMax(std::vector<QString> words);
  void getStackStats(std::vector<QString> words);
  void req_blob_model(std::vector<QString> words);

  void saveCommand(std::vector<QString> words);
  
// calls ImageBuilder directly and parses the data there.
  void generalFunction(std::vector<QString> words);  

  void resetRGB();
  void reportParameters();

  std::vector<float> getFloats(std::vector<QString>& words, unsigned int offset);
  std::vector<int> getInts(std::vector<QString>& words, unsigned int offset);

  std::string toString(QString& qstr);

  private slots :
  void parseInput();
  void repeatCommand(bool up);
  void makeHelpStatements();
  void readSavedCommands(std::string dir_name);
  void saveSelectedCommands();
  void saveCommand(QString& cmd, std::string& dir_name);
};

#endif
