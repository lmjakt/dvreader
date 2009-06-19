#include "superBlobWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <iostream>

using namespace std;

SuperBlobWidget::SuperBlobWidget(QWidget* parent)
  : QWidget(parent)
{
  //  classifier = new BlobClassifier();
  // Set up the layout and the paramChecks. Everything else done before..
  makeParamBox(BlobMapper::VOLUME, "Volume");
  makeParamBox(BlobMapper::SUM, "Sum");
  makeParamBox(BlobMapper::MEAN, "Mean");
  makeParamBox(BlobMapper::MAX, "Max");
  makeParamBox(BlobMapper::MIN, "Min");
  makeParamBox(BlobMapper::EXTENT, "Extent");
  makeParamBox(BlobMapper::SURFACE, "Surface");
  makeParamBox(BlobMapper::BACKGROUND, "Background");

  blobRowCounter = 0;
  QLabel* blobGridLabel = new QLabel("Blob Mappers", this);
  QLabel* idLabel = new QLabel("ID", this);
  QLabel* exLabel = new QLabel("ex", this);
  QLabel* emLabel = new QLabel("em", this);
  QLabel* minLabel = new QLabel("min", this);
  QLabel* sizeLabel = new QLabel("blob no.", this);
  

  QPushButton* superBlobButton = new QPushButton("Super", this);
  connect(superBlobButton, SIGNAL(clicked()), this, SLOT(reqSuperBlobs()) );

  QPushButton* trainButton = new QPushButton("Train", this);
  connect(trainButton, SIGNAL(clicked()), this, SLOT(trainClassifier()) );

  QPushButton* classifyButton = new QPushButton("Classify", this);
  connect(classifyButton, SIGNAL(clicked()), this, SLOT(classifyBlobs()) );

  QVBoxLayout* mainBox = new QVBoxLayout(this);
  mainBox->setSpacing(3);
  mainBox->setMargin(1);
  grid = new QGridLayout();
  mainBox->addLayout(grid);
  grid->addWidget(blobGridLabel, 0, 0, 1, 6, Qt::AlignLeft);
  grid->addWidget(idLabel, 1, 0);
  grid->addWidget(exLabel, 1, 1);
  grid->addWidget(emLabel, 1, 2);
  grid->addWidget(minLabel, 1, 3);
  grid->addWidget(sizeLabel, 1, 4);
  mainBox->addStretch();
  QHBoxLayout* buttonBox = new QHBoxLayout();
  mainBox->addLayout(buttonBox);
  buttonBox->addStretch();
  buttonBox->addWidget(superBlobButton, Qt::AlignRight);
  QGridLayout* paramBox = new QGridLayout();
  mainBox->addLayout(paramBox);
  int count = 0;
  for(map<BlobMapper::Param, QCheckBox*>::iterator it=paramChecks.begin();
      it != paramChecks.end(); ++it)
    {
      paramBox->addWidget((*it).second, count/2, count%2);
      ++count;
    }
  paramBox->addWidget(trainButton, 1 + count/2, 0);
  paramBox->addWidget(classifyButton, 1 + count/2, 1);
}

SuperBlobWidget::~SuperBlobWidget(){
  // widgets delete their own children
  // does this override that behaviour ?
  
}

void SuperBlobWidget::makeParamBox(BlobMapper::Param p, QString label){
  QCheckBox* box = new QCheckBox(label, this);
  box->setChecked(true);
  connect(box, SIGNAL(clicked()), this, SLOT(paramSelectionChanged()) );
  paramChecks.insert(make_pair(p, box));
}


void SuperBlobWidget::addBlobMapperWidget(BlobMapperWidget* bmw)
{
  BlobMapperInfo* bi = new BlobMapperInfo();
  fluorInfo fi = bmw->fInfo();
  bi->bmw = bmw;
  bi->exLabel = makeFloatLabel(fi.excitation);
  bi->emLabel = makeFloatLabel(fi.emission);
  bi->minLabel = makeFloatLabel(bmw->minEdge());
  bi->sizeLabel = makeFloatLabel((float)bmw->blobNo());
  bi->idLabel = makeFloatLabel((float)bmw->blobMapper()->mapId());
  //  bi->idLabel = new QLabel("-", this);
  bi->superCheck = new QCheckBox("S", this);
  bi->superCheck->setChecked(true);
  cout << "before inserting new bi into bmw->blobMapper()" << endl;
  blobInfo.insert(make_pair(bmw->blobMapper(), bi));
  cout << "inserted, and possibly made some copy of the label which might induce trouble" << endl;
  // At this point I need some reasonable way of choosing a row to put the widgets into
  setLabels();
  //  addLabels(bi);
}

QLabel* SuperBlobWidget::makeFloatLabel(float f){
  QString s;
  s.setNum(f);
  QLabel* label = new QLabel(s, this);
  return(label);
}

void SuperBlobWidget::removeBlobMapperWidget(BlobMapperWidget* bmw){
  BlobMapper* bm = bmw->blobMapper();
  if(!blobInfo.count(bm)){
    cerr << "SuperBlobWidget::removeBlobMapperWidget : no blobInfo assigned to blobMapper : " << bm << endl;
    return;
  }
  // if I understand this correctly, a simple erase will do everything as I'm not using pointers
  delete blobInfo[bm];
  blobInfo.erase(bm);
  setLabels();
}

void SuperBlobWidget::setSuperBlobs(vector<SuperBlob*>& sblobs){
  clearSuperBlobInfo();
  map<BlobMapper*, map<int, int> > classCounts;
  //map<BlobMapper*, uint> mapperIds;
  for(uint i=0; i < sblobs.size(); ++i){
    uint c = sblobs[i]->membership;
    for(uint j=0; j < sblobs[i]->blobs.size(); ++j){
      BlobMapper* bm = sblobs[i]->blobs[j].mapper;
      //uint mid = sblobs[i]->blobs[j].mapper_id;
      //mapperIds[ bm ] = mid;
      if(!classCounts[bm].count(c))
	classCounts[bm][c] = 0;
      classCounts[bm][c]++;
    }
  }

  // Then we go through and set the appropriate labels;
  
  //QString lb;
  for(map<BlobMapper*, map<int, int> >::iterator it=classCounts.begin(); it != classCounts.end(); ++it)
    {
      BlobMapper* bm = (*it).first;
      if(!blobInfo.count(bm)){
	cerr << "SuperBlobWidget::setSuperBlobs : unknown blobMapper no info set" << endl;
	  continue;
      }
      //      lb.setNum(mapperIds[bm]);
      //blobInfo[bm]->idLabel->setText(lb);
      blobInfo[bm]->classes.clear();
      //      blobInfo[bm]->classes.resize(0);
      for(map<int, int>::iterator bit=(*it).second.begin(); bit != (*it).second.end(); ++bit)
	{
	  sblobClassInfo* ci = new sblobClassInfo();
	  ci->id_label = makeFloatLabel((float)(*bit).first);
	  ci->sizeLabel = makeFloatLabel((float)(*bit).second);
	  ci->reClassSize = makeFloatLabel((float)0);
	  blobInfo[bm]->classes.insert(make_pair( (*bit).first, ci));
	  //	  blobInfo[bm]->classes.push_back(ci);  // again, we might have to change this
	}
    }
  setLabels();
  // and that's it?
}      


void SuperBlobWidget::setClassCounts(vector<BlobClassCounts> classCounts)
{
  cout << "SuperBlobWidget::setClassCounts classCounts size : " << classCounts.size() << endl;
  for(uint i=0; i < classCounts.size(); ++i){
    if(!blobInfo.count(classCounts[i].mapper)){
      cerr << "SuperBlobWidget::setClassCounts : unknown BlobMapper : " << classCounts[i].mapper << endl;
      continue;
    }
    BlobMapper* bm = classCounts[i].mapper;
    for(map<int, uint>::iterator it = classCounts[i].counts.begin();
	it != classCounts[i].counts.end(); ++it)
      {
	if(!blobInfo[bm]->classes.count( (*it).first )){
	  cerr << "unknown class" << endl;
	  continue;
	}
	// set the label ok..
	QString number;
	number.setNum( (*it).second );
	blobInfo[bm]->classes[ (*it).first ]->reClassSize->setText(number);
	cout << "Class : " << (*it).first << "  : " << (*it).second << endl;
      }
  }
  cout << "SuperBlobWidget::setClassCounts finished" << endl;
}

void SuperBlobWidget::paramSelectionChanged()
{
  // do someting reasonable ..?
}

void SuperBlobWidget::reqSuperBlobs(){
  set<BlobMapperWidget*> bmw;
  for(map<BlobMapper*, BlobMapperInfo*>::iterator it=blobInfo.begin();
      it != blobInfo.end(); ++it){
    if((*it).second->superCheck->isChecked())
      bmw.insert((*it).second->bmw);
  }
  if(bmw.size() < 2){
    cerr << "Cannot request superblobs with less than two blob types" << endl;
    return;
  }
  emit makeSuperBlobs(bmw);
}

void SuperBlobWidget::classifyBlobs(){
  set<BlobMapperWidget*> bmw;
  for(map<BlobMapper*, BlobMapperInfo*>::iterator it=blobInfo.begin();
      it != blobInfo.end(); ++it){
    if((*it).second->superCheck->isChecked())
      bmw.insert((*it).second->bmw);
  }
  if(!bmw.size())
    return;
  cout << "emitting classify blobs from classifyBlobs ??" << endl;
  emit classifyBlobs(bmw);
}

void SuperBlobWidget::trainClassifier(){
  set<BlobMapper::Param> p;
  for(map<BlobMapper::Param, QCheckBox*>::iterator it = paramChecks.begin();
      it != paramChecks.end(); ++it){
    if((*it).second->isChecked())
      p.insert((*it).first);
  }
  emit classifySuperBlobs(p);
}

void SuperBlobWidget::setLabels(){
  blobRowCounter = 1;
  for(map<BlobMapper*, BlobMapperInfo*>::iterator it=blobInfo.begin();
      it != blobInfo.end(); ++it)
    {
      addLabels((*it).second);
    }
}
// and 
void SuperBlobWidget::addLabels(BlobMapperInfo* bi)
{
  blobRowCounter++;
  grid->addWidget(bi->idLabel, blobRowCounter, 0);
  grid->addWidget(bi->exLabel, blobRowCounter, 1);
  grid->addWidget(bi->emLabel, blobRowCounter, 2);
  grid->addWidget(bi->minLabel, blobRowCounter, 3);
  grid->addWidget(bi->sizeLabel, blobRowCounter, 4);
  grid->addWidget(bi->superCheck, blobRowCounter, 5);
  for(map<int, sblobClassInfo*>::iterator it=bi->classes.begin(); it != bi->classes.end(); ++it){
    //  for(uint i=0; i < bi->classes.size(); ++i){
    blobRowCounter++;
    grid->addWidget((*it).second->id_label, blobRowCounter, 1);
    grid->addWidget((*it).second->sizeLabel, blobRowCounter, 2);
    grid->addWidget((*it).second->reClassSize, blobRowCounter, 3);
    //grid->addWidget(bi->classes[i]->id_label, blobRowCounter, 1);
    //grid->addWidget(bi->classes[i]->sizeLabel, blobRowCounter, 2);
    //    grid->addWidget(bi->classes[i]->reClassSize, blobRowCounter, 3);
  }
}		  
      
void SuperBlobWidget::clearSuperBlobInfo(){
  for(map<BlobMapper*, BlobMapperInfo*>::iterator it=blobInfo.begin(); it != blobInfo.end(); ++it){
    while((*it).second->classes.size()){
      delete( (*it).second->classes.begin()->second );
      (*it).second->classes.erase( (*it).second->classes.begin() );
    }
//     for(uint i=0; i < (*it).second->classes.size(); ++i){
//       delete (*it).second->classes[i];
//     }
//     (*it).second->classes.resize(0);
  }
}
