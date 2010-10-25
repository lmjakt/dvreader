#include "imageBuilderWidget.h"
#include <iostream>
#include <QString>
#include <QVBoxLayout>
#include <QDir>
#include <QFile>
#include <QTabWidget>
#include <QTextCursor>
#include <QPlainTextEdit>
#include <QTextStream>
#include <QChar>
#include <QStringList>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
//#include "../globalVariables.h"
#include "../dataStructs.h"
#include "../globalVariables.h"


using namespace std;

ImageBuilderWidget::ImageBuilderWidget(FileSet* fs, vector<channel_info> ch, QWidget* parent)
  : QWidget(parent)
{
  hashAlgorithm = QCryptographicHash::Md5;
  initialise();
  builder = new ImageBuilder(fs, ch);
  connect(builder, SIGNAL(stackCreated(QString)), this, SLOT(objectCreated(QString)) );
  connect(builder, SIGNAL(stackDeleted(QString)), this, SLOT(objectDeleted(QString)) );
  input = new CLineEdit(this);
  historySize = 1000;
  commandPos = 0;
  //  input = new QLineEdit(this);
  outputTabs = new QTabWidget(this);
  output = new QPlainTextEdit();
  helpPage = new QPlainTextEdit();
  commandPage = new QPlainTextEdit();
  objectPage = new QPlainTextEdit();
  objectPage->setReadOnly(true);
  for(set<QString>::iterator it=saved_commands.begin(); it != saved_commands.end(); ++it)
    commandPage->appendPlainText((*it));
  outputTabs->insertTab(output, "History");
  outputTabs->insertTab(helpPage, "Help");
  outputTabs->insertTab(commandPage, "Commands");
  outputTabs->insertTab(objectPage, "Objects");

  connect(input, SIGNAL(returnPressed()), this, SLOT(parseInput()) );
  connect(input, SIGNAL(arrowPressed(bool)), this, SLOT(repeatCommand(bool)) );


  //  for(map<QString, Task>::iterator it=tasks.begin(); it != tasks.end(); ++it)
  //  cout << (*it).first.toAscii().data() << "  -->  " << (*it).second << endl;

  makeHelpStatements();

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->addWidget(outputTabs);
  vbox->addWidget(input);
}

ImageBuilderWidget::~ImageBuilderWidget(){
  delete builder;
}

void ImageBuilderWidget::setImage(float* image, int w, int h){
  builder->setRGBImage(image, w, h);
  delete []image;
}

void ImageBuilderWidget::setGreyImage(float* image, int w, int h, bool destroy_source)
{
  if(w <= 0 || h <= 0)
    return;
  // no scale specifed. lets autoscale between 1 and 0.
  int l = w * h;
  float max = image[0];
  for(int i=1; i < l; ++i)
    max = image[i] > max ? image[i] : max;
  
  float* rgbImage = new float[w * h * 3];
  for(int i=0; i < l; ++i){
    rgbImage[ i * 3 ] = image[i] / max;
    rgbImage[ i * 3 + 1] = image[i] / max;
    rgbImage[ i * 3 + 2] = image[i] / max;
  }
  for(int y=0; y < h; ++ y){
    cout << y << ":";
    for(int x=0; x < w; ++x){
      cout << "\t" << image[ y * w + x ];
    }
    cout << endl;
  }
  cout << "max : " << max << endl;
  builder->setRGBImage(rgbImage, w, h);
  delete []rgbImage;
  if(destroy_source)
    delete []image;
}

// open home directory, if not present create appropriate directories..
void ImageBuilderWidget::initialise(){
  // First set up the set of commands..
  na_functions["report"] = &ImageBuilderWidget::reportParameters;
  na_functions["reset_rgb"] = &ImageBuilderWidget::resetRGB;
  
  w_functions["set"] = &ImageBuilderWidget::setParameter;
  w_functions["slice"] = &ImageBuilderWidget::makeSlice;
  w_functions["g_slice"] = &ImageBuilderWidget::makeGaussianSlice;
  w_functions["project"] = &ImageBuilderWidget::makeProjection;
  w_functions["bgr_slice"] = &ImageBuilderWidget::makeBGRSlice;
  w_functions["bg_add"] = &ImageBuilderWidget::addBackground;
  w_functions["bg_sub"] = &ImageBuilderWidget::subBackground;
  w_functions["add_mcp"] = &ImageBuilderWidget::addMCP;
  w_functions["export"] = &ImageBuilderWidget::exportImage;
  w_functions["coverage"] =  &ImageBuilderWidget::requestCoverage;
  w_functions["help"] = &ImageBuilderWidget::printHelp;
  w_functions["bg_spec"] = &ImageBuilderWidget::bg_spectrum;
  w_functions["spec_bg"] = &ImageBuilderWidget::paint_spectral_background;
  w_functions["p_slice"] = &ImageBuilderWidget::pipe_slice;
  w_functions["ch_gmax"] = &ImageBuilderWidget::setChooserMax;
  w_functions["stack_stats"] = &ImageBuilderWidget::getStackStats;
  w_functions["blob_model"] = &ImageBuilderWidget::req_blob_model;  
  w_functions["save_command"] = &ImageBuilderWidget::saveCommand;
  // builder directory should be the home_dir/Imagebuilder
  // where home_dir is the application home. However, if home_dir 
  // is not given we have to mak
  cout << "dvreader_home is : " << dvreader_home() << endl;
  if(!dvreader_home()){
    cerr << "dvreader_home is nothing" << endl;
    exit(1);
  }
  builder_dir = dvreader_home();
  builder_dir.append("/builder");
  builder_cmd_dir = builder_dir + string("/cmds");
  QDir cmd_dir;
  if(!cmd_dir.exists(builder_cmd_dir.c_str()) && !cmd_dir.mkpath(builder_cmd_dir.c_str())){
      cerr << "ImageBuilderWidget unable to create directory : " << builder_cmd_dir << endl;
      return;
  }
  readSavedCommands(builder_cmd_dir);
  
}

void ImageBuilderWidget::parseInput(){
  QString line = input->text();
  commandHistory.push_back(line);
  
  // QTextCursor cursor = output->cursor();
  // cursor.movePosition(QTextCursor::End);
  // cursor.       ;//  line feeds are translated to unicode block separators but not read as lines using readline

  output->appendPlainText(line);
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

void ImageBuilderWidget::makeHelpStatements(){
  helpStrings["set"] += "set paramtype params params are :";
  helpStrings["set"] += "\n\tsandb ch scale bias";
  helpStrings["set"] += "\n\tcolor ch r g b";
  helpStrings["set"] += "\n\tmax ch max_value";
  helpStrings["set"] +=  "\n\tbgpar ch w h d qnt sub?";

  helpStrings["slice"] = "slice section ch1 ch2 ..";
  helpStrings["project"] = "project sec_start sec_end ch1 ch2 ..";
  helpStrings["report"] = "report";
  helpStrings["bg_add"] = "bg_add ch section r g b";
  helpStrings["bg_sub"] = "bg_sub ch section";
  helpStrings["reset_rgb"] = "reset_rgb";
  helpStrings["add_mcp"] = "add_mcp ch sec_start sec_end";
  helpStrings["export"] = "export filetype filename";
  helpStrings["coverage"] = "coverage max_count";
  helpStrings["help"] = "help [command]";
}

void ImageBuilderWidget::readSavedCommands(string dir_name)
{
  // let's try using the opendir and other commands..
  DIR* dir = opendir(dir_name.c_str());
  if(!dir)
    return;
  // and then read file names..
  dirent* dent;
  while( (dent = readdir(dir)) != 0){
    if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..") || dent->d_name[0] == '.')
      continue;
    cout << "Found a file called " << dent->d_name << endl;
    string file_path = dir_name + string("/") + string(dent->d_name);
    ifstream in(file_path.c_str());
    if(!in){
      cerr << "ImageBuilderWidget::readSavedCommands Unable to read file : " << file_path << endl;
      continue;
    }
    string cmd;
    getline(in, cmd);  // this will terminate at \n, or eof. Not sure how to check if the operation was successful though
    // if we want to have multi-line 
    if(!cmd.length())
      continue;
    // At this point we should check whether the content matches the file name, but I can't be bothered right now
    if(saved_commands.count(cmd.c_str()))
      continue;
    saved_commands.insert( QString(cmd.c_str()) );  // Why QString ? I don't quite remember
  }
}

// Obtains selected text from the past history (in output)
void ImageBuilderWidget::saveSelectedCommands()
{
  QTextCursor cursor = output->textCursor();
  if(!cursor.hasSelection()){
    cout << "Cursor has no selection" << endl;
    return;
  }
  QString selection = cursor.selectedText();

  QStringList lineList = selection.split(QChar(QChar::ParagraphSeparator));
  for(int i=0; i < lineList.size(); ++i){
    saveCommand(lineList[i], builder_cmd_dir);
    commandPage->appendPlainText(lineList[i]);
  }
  // // for now let's see if that works.. maybe it's impossible??
  // QByteArray sel = selection.toAscii();
  // for(int i=0; i < sel.size(); ++i)
  //   cout << i << "\t" << (int)sel[i] << "\t" << sel[i] << endl;


  // QTextStream qts(&selection);
  // QString line = qts.readLine();
  // while( line.length() ){
  //   saveCommand(line, builder_cmd_dir);
  //   commandPage->appendPlainText(line);
  //   line = qts.readLine();
  // }
}

void ImageBuilderWidget::saveCommand(QString& cmd, string& dir_name)
{
  if(saved_commands.count(cmd)){
    return;
  }
  QDir cdir(builder_cmd_dir.c_str());
  if(!cdir.exists()){
    cerr << "ImageBuilderWidget::saveCommand builder_cmd_dir: " << builder_cmd_dir << " either not specified or does not exist" << endl;
    return;
  }
  QByteArray hash = QCryptographicHash::hash(cmd.toAscii(), hashAlgorithm);
  string file_path = dir_name + string("/") + string(hash.toHex().constData());  // that is so ugly.
  QFile file( QString(file_path.c_str()) );
  if(file.exists())
    return;
  file.open(QIODevice::WriteOnly);
  file.write(cmd.toAscii());
  file.close();
  cout << "File : " << file_path << " opened, written and closed" << endl; 
}

void ImageBuilderWidget::objectCreated(QString description)
{
  if(objectDescriptions.count(description))
    return;
  objectDescriptions.insert(description);
  setObjectList();
}

void ImageBuilderWidget::objectDeleted(QString description)
{
  if(!objectDescriptions.count(description))
    return;
  objectDescriptions.erase(description);
  setObjectList();
}

void ImageBuilderWidget::setObjectList()
{
  QString list;
  QTextStream qts(&list);
  for(set<QString>::iterator it=objectDescriptions.begin();
      it != objectDescriptions.end(); ++it)
    qts << (*it) << "\n";
  objectPage->setPlainText(list);
}

void ImageBuilderWidget::parseInput(vector<QString> words)
{
  // for now, just do something simple
  if(!words.size()){
    cerr << "ImageBuilderWidget::parseInput failed" << endl;
    return;
  }
  if(na_functions.count(words[0])){
    na_function func = na_functions[words[0]];
    (*this.*func)();
    return;
  }
  if(w_functions.count(words[0])){
    w_function func = w_functions[words[0]];
    (*this.*func)(words);
    return;
  }
  generalFunction(words);
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

void ImageBuilderWidget::makeGaussianSlice(vector<QString> words)
{
  vector<int> values = getInts(words, 1);
  if(values.size() < 3)
    return;
  set<unsigned int> wi;
  for(uint i=2; i < values.size(); ++i)
    wi.insert((unsigned int)values[i]);
  builder->gaussianSlice(wi, (unsigned int)values[0], (unsigned int)values[1]);
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

void ImageBuilderWidget::makeBGRSlice(vector<QString> words)
{
  vector<int> values = getInts(words, 1);
  if(values.size() < 3)
    return;
  set<unsigned int> wi;
  for(uint i=1; i < values.size(); ++i)
    wi.insert((unsigned int)values[i]);
  builder->buildSpectrallySubtractedImage(wi, (unsigned int)values[0]);
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

void ImageBuilderWidget::bg_spectrum(vector<QString> words)
{
  vector<int> ints = getInts(words, 1);
  // first int is the slice number, following ints are the channels to be used
  // need to specif at least two channels
  if(ints.size() < 3)
    return;
  set<unsigned int> ch;
  for(uint i=1; i < ints.size(); ++i)
    ch.insert((unsigned int)ints[i]);
  builder->paintSpectralResponse(ch, (unsigned int)ints[0]);
}

void ImageBuilderWidget::paint_spectral_background(vector<QString> words)
{
  vector<int> ints = getInts(words, 1);
  if(ints.size() < 3)
    return;
  set<unsigned int> ch;
  for(uint i=1; i < ints.size(); ++i)
    ch.insert((unsigned int)ints[i]);
  builder->paintSpectralBackground(ch, (unsigned int)ints[0], (unsigned int)ints[1]);
}

void ImageBuilderWidget::pipe_slice(vector<QString> words)
{
  // the command should contain some information as to how to 
  // paint the data, followed by a number of commands that obtain
  // the data.

  // the prefix should specify the width and height as well as offsets,
  // of the image requested; the channel index to use for setting the colour,
  // and a boolean to determine whether rgb should be reset.

  if(words.size() < 7){
    return;
  }
  // first get a bool for reset rgb and a channel index to check things.
  // prefix fields are:
  // 0. waveindex (used for setting the bias, scale and colour, etc..)
  // 1. boolean, indicating whether to clear the rgb buffer
  // 2. width
  // 3. height
  // 4. x offset
  // 5. y offset
  //
  // then follow with specific pipe commands.. 
  vector<QString> prefix;
  vector<QString>::iterator it = words.begin();
  ++it;
  while(it != words.end() && (*it) != "|" ){
    prefix.push_back((*it));
    ++it;
  }
  if(prefix.size() != 6){
    return;
  }
  bool ok;
  unsigned int wi = prefix[0].toUInt(&ok);
  if(!ok)
    return;
  bool resetRGB = (prefix[1] != "0" && prefix[1] != "false");

  int im_w, im_h, xo, yo;
  im_w = prefix[2].toInt(&ok);
  im_h = prefix[3].toInt(&ok);
  xo = prefix[4].toInt(&ok);
  yo = prefix[5].toInt(&ok);

  vector<p_parameter> pars;
  // and then we collect the rest..
  while(it != words.end()){
    ++it;
    map<string, float> values;
    vector<QString> terms;
    while(it != words.end() && (*it) != "|"){
      terms.push_back(*it);
      ++it;
    }
    // The first term should be the name of the function.
    // the positions are taken from the prefix above.
    if(!terms.size())
      return;
    string cmd = toString(terms[0]);
    for(uint i=1; i < (terms.size() -1 ); i += 2)
      values.insert(make_pair(toString(terms[i]), terms[i+1].toFloat())); 
    pars.push_back(p_parameter(cmd, 0, im_w, im_h, xo, yo, values));
  }
  builder->pipe_slice(pars, wi, resetRGB);
}

void ImageBuilderWidget::setChooserMax(vector<QString> words)
{
  if(words.size() < 2)
    return;
  bool ok;
  float mv = words[1].toFloat(&ok);
  if(!ok)
    return;
  emit setChooserMax(mv);
}

void ImageBuilderWidget::getStackStats(vector<QString> words)
{
  if(words.size() < 8)
    return;
  vector<int> ints = getInts(words, 1);
  if(ints.size() != 7){
    cerr << "ImageBuilderWidget::getStackStats, int.size is not 5 : " << ints.size() << endl;
    return;
  }
  // ints are : waveIndex, x_begin, y_begin, z_begin, width, height, depth..
  builder->getStackStats((unsigned int)ints[0], ints[1], ints[2], ints[3], ints[4], ints[5], ints[6]);
}

void ImageBuilderWidget::req_blob_model(vector<QString> words)
{
  if(words.size() < 3)
    return;
  vector<int> ints = getInts(words, 1);
  if(ints.size() != 2)
    return;
  emit getBlobModel(ints[0], ints[1]);   // xy_radius and z_radius
}

void ImageBuilderWidget::saveCommand(vector<QString> words)
{
  cout << "ImageBuilderWidget saveCommand words.size() " << words.size() << endl;
  if(words.size() < 2)
    return;
  if(words[1] == "selection"){
    cout << "Calling saveSelectedCommands() " << endl;
    saveSelectedCommands();
    return;
  }
  cout << "but words[1] is not selection" << words[1].toAscii().constData() << endl;
  // we can include things like last, and so on.. 
}

void ImageBuilderWidget::generalFunction(vector<QString> words)
{
  cout << "ImageBuilderWidget::generalFunction.. " << endl;
  f_parameter param(words);
  builder->general_command(param);
}

void ImageBuilderWidget::reportParameters()
{
  builder->reportParameters();
}

void ImageBuilderWidget::resetRGB()
{
  builder->resetRGB();
}

void ImageBuilderWidget::exportImage(vector<QString> words)
{
  if(words.size() < 3){
    cerr << "export filetype filename" << endl;
    return;
  }
    
  if(words[1] == "tiff"){
    builder->exportTiff(words[2]);
  }
}

void ImageBuilderWidget::requestCoverage(vector<QString> words){
  if(words.size() < 2){
    cerr << "coverage maxValue";
    return;
  }
  float mv = words[1].toFloat();
  emit showCoverage(mv);
}

void ImageBuilderWidget::printHelp(vector<QString> words){
  if(words.size() > 1 && helpStrings.count(words[1])){
    helpPage->appendPlainText(helpStrings[words[1]]);
    helpPage->appendPlainText("\n");
    outputTabs->setCurrentWidget(helpPage);
    return;
  }
  QString helpText;
  QTextStream qts(&helpText);
  qts << "Commands:";
  for(map<QString, w_function>::iterator it=w_functions.begin();
      it != w_functions.end(); ++it)
    qts << "\n" << it->first;
  for(map<QString, na_function>::iterator it=na_functions.begin(); 
      it != na_functions.end(); ++it)
    qts << "\n" << it->first;
  qts << "\n\n";
  helpPage->appendPlainText(helpText);
  outputTabs->setCurrentWidget(helpPage);
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
      
string ImageBuilderWidget::toString(QString& qstr){
  string str(qstr.toAscii().constData());
  return(str);
}
