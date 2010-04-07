#ifndef IMAGEBUILDERWIDGET_H
#define IMAGEBUILDERWIDGET_H

#include <QWidget>
//#include <QLineEdit>
#include <QTextEdit>
#include <QString>
#include <QTextStream>
#include <vector>
#include <map>
#include "imageBuilder.h"
#include "../customWidgets/clineEdit.h"
#include <deque>

struct channel_info;

class ImageBuilderWidget: public QWidget
{
  Q_OBJECT
 public:
  enum Task {
    SLICE, PROJECT, SET, REPORT
  };
  
  ImageBuilderWidget(FileSet* fs, std::vector<channel_info> ch, QWidget* parent=0);
  ~ImageBuilderWidget();

 private:
  CLineEdit* input;
  std::deque<QString> commandHistory;
  unsigned int historySize;
  unsigned int commandPos;
  //  QLineEdit* input;
  QTextEdit* output;
  ImageBuilder* builder;
  std::map<QString, Task> tasks;

  void parseInput(std::vector<QString> words);
  void setParameter(std::vector<QString> words);
  void makeSlice(std::vector<QString> words);
  void makeProjection(std::vector<QString> words);
  
  std::vector<float> getFloats(std::vector<QString>& words, unsigned int offset);
  std::vector<int> getInts(std::vector<QString>& words, unsigned int offset);

  private slots :
  void parseInput();
  void repeatCommand(bool up);
 
};

#endif
