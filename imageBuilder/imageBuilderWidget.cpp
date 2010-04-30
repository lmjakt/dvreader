#include "imageBuilderWidget.h"
#include <iostream>
#include <QString>
#include <QVBoxLayout>
#include "../dataStructs.h"

using namespace std;

ImageBuilderWidget::ImageBuilderWidget(FileSet* fs, vector<channel_info> ch, QWidget* parent)
  : QWidget(parent)
{
  builder = new ImageBuilder(fs, ch);
  input = new CLineEdit(this);
  historySize = 1000;
  commandPos = 0;
  //  input = new QLineEdit(this);
  output = new QTextEdit(this);

  connect(input, SIGNAL(returnPressed()), this, SLOT(parseInput()) );
  connect(input, SIGNAL(arrowPressed(bool)), this, SLOT(repeatCommand(bool)) );

  tasks["set"] = SET;
  tasks["slice"] = SLICE;
  tasks["project"] = PROJECT;
  tasks["report"] = REPORT;
  tasks["bg_add"] = BG_ADD;
  tasks["bg_sub"] = BG_SUB;
  tasks["reset_rgb"] = RESET_RGB;
  tasks["add_mcp"] = ADD_MCP;

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->addWidget(output);
  vbox->addWidget(input);
}

ImageBuilderWidget::~ImageBuilderWidget(){
  delete builder;
}

void ImageBuilderWidget::parseInput(){
  QString line = input->text();
  commandHistory.push_back(line);
  output->append(line);
  if(commandHistory.size() > historySize)
    commandHistory.pop_front();
  commandPos = commandHistory.size();
  input->clear();
  QTextStream str(&line);
  QString word;
  vector<QString> words;
  while(!str.atEnd()){
    str >> word;
    words.push_back(word);
  }
  parseInput(words);
}

void ImageBuilderWidget::repeatCommand(bool up){
  if(up && commandPos > 0){
    input->setText( commandHistory[--commandPos] );
    return;
  }
  if(!up && commandPos < commandHistory.size() - 1){
    input->setText( commandHistory[++commandPos] );
  }
}

void ImageBuilderWidget::parseInput(vector<QString> words)
{
  // for now, just do something simple
  if(!words.size() || !tasks.count(words[0])){
    cerr << "ImageBuilderWidget::parseInput failed" << endl;
    return;
  }
  switch(tasks[words[0]]){
  case SET:
    setParameter(words);
    break;
  case SLICE:
    makeSlice(words);
    break;
  case PROJECT:
    makeProjection(words);
    break;
  case BG_ADD:
    addBackground(words);
    break;
  case BG_SUB:
    subBackground(words);
    break;
  case REPORT:
    builder->reportParameters();
    break;
  case RESET_RGB:
    builder->resetRGB();
    break;
  case ADD_MCP:
    addMCP(words);
    break;
  default:
    cerr << "ImageBuilderWidget::parseInput(vector<QString>): unknown command " << endl;
  }
}

void ImageBuilderWidget::setParameter(vector<QString> words)
{
  if(words.size() < 3)
    return;
  bool ok;
  unsigned int wi = (unsigned int)words[2].toInt(&ok);
  if(!ok)
    return;
  vector<float> values = getFloats(words, 3);
  if(!values.size())
    return;
  
  if(words[1] == "sandb"){
    if(values.size() == 2){
      builder->setScaleAndBias(wi, values[1], values[0]);
    }
    return;
  }
  if(words[1] == "color"){
    if(values.size() == 3){
      builder->setColor(wi, values[0], values[1], values[2]);
    }
    return;
  }
  if(words[1] == "max"){
    if(values.size() == 1){
      builder->setMaxLevel(wi, values[0]);
    }
    return;
  }
  if(words[1] == "bgpar"){
    if(values.size() == 5)
      builder->setBackgroundPars(wi, (int)values[0], (int)values[1], (int)values[2],
				 values[3], (bool)values[4]);
    return;
  }
}

void ImageBuilderWidget::makeSlice(vector<QString> words)
{
  vector<int> values = getInts(words, 1);
  if(values.size() < 2)
    return;
  // first value is the wi, the others are the wavelengths.
  set<unsigned int> wi;
  for(uint i=1; i < values.size(); ++i)
    wi.insert((unsigned int)values[i]);
  builder->buildSlice(wi, (unsigned int)values[0]);
}

void ImageBuilderWidget::makeProjection(vector<QString> words)
{
  vector<int> values = getInts(words, 1);
  if(values.size() < 3)
    return;
  vector<unsigned int> wi;
  for(uint i=2; i < values.size(); ++i)
    wi.push_back((unsigned int)values[i]);
  builder->buildProjection(wi, (unsigned int)values[0], (unsigned int)values[1]);
}

void ImageBuilderWidget::addBackground(vector<QString> words)
{
  vector<int> ints = getInts(words, 1);
  vector<float> floats = getFloats(words, 3);
  if(floats.size() == 3 && ints.size() > 2)
    builder->addBackground((unsigned int)ints[0], (unsigned int)ints[1],
			   color_map(floats[0], floats[1], floats[2]) );
}

void ImageBuilderWidget::subBackground(vector<QString> words)
{
  vector<int> ints = getInts(words, 1);
  if(ints.size() == 2)
    builder->subBackground((unsigned int)ints[0], (unsigned int)ints[1]);
}

void ImageBuilderWidget::addMCP(vector<QString> words)
{
  vector<int> ints = getInts(words, 1);
  if(ints.size() == 3)
    builder->addMCP((unsigned int)ints[0], (unsigned int)ints[1], (unsigned int)ints[2]);
}

vector<float> ImageBuilderWidget::getFloats(vector<QString>& words, unsigned int offset){
  vector<float> values;
  float v;
  bool ok;
  for(uint i=offset; i < words.size(); ++i){
    v = words[i].toFloat(&ok);
    if(!ok)
      return(values);
    values.push_back(v);
  }
  return(values);
}


vector<int> ImageBuilderWidget::getInts(vector<QString>& words, unsigned int offset){
  vector<int> values;
  int v;
  bool ok;
  for(uint i=offset; i < words.size(); ++i){
    v = words[i].toInt(&ok);
    if(!ok)
      return(values);
    values.push_back(v);
  }
  return(values);
}
      
