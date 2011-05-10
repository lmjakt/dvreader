#include "BlobSetPlotter.h"
#include "../linGraph/linePlotter.h"
#include "../image/blob.h"
#include "blob_set.h"
#include <iostream>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextStream>

using namespace std;

BlobSetPlotter::BlobSetPlotter(QWidget* parent) :
  QWidget(parent)
{
  setCaption("Blob Set Plotter");
  coll_name = "NULL";
  id = 0;
  plotter = new LinePlotter(this);

  y_label = new QLabel("***", this);
  x_label = new QLabel("***", this);

  pos_label = new QLabel("******", this);

  connect(plotter, SIGNAL(mousePos(int, float)), this, SLOT(setMousePos(int, float)) );
  
  QLabel* bs_label = new QLabel("Blob Set", this);
  QLabel* b_label = new QLabel("Blob", this);
  QLabel* z_label = new QLabel("z-pos", this);
  
  blobSetSelector = new QSpinBox(this);
  blobSelector = new QSpinBox(this);
  z_selector = new QSpinBox(this);

  connect(blobSetSelector, SIGNAL(valueChanged(int)), this, SLOT(setChanged(int)) );
  connect(blobSelector, SIGNAL(valueChanged(int)), this, SLOT(blobChanged(int)) );
  connect(z_selector, SIGNAL(valueChanged(int)), this, SLOT(plot(int)) );

  QVBoxLayout* mainBox = new QVBoxLayout(this);
  mainBox->addWidget(plotter);
  QHBoxLayout* controlBox = new QHBoxLayout();
  mainBox->addLayout(controlBox);
  mainBox->setStretchFactor(plotter, 1);
  mainBox->setStretchFactor(controlBox, 0);
  controlBox->addWidget(bs_label);
  controlBox->addWidget(blobSetSelector);
  controlBox->addWidget(b_label);
  controlBox->addWidget(blobSelector);
  controlBox->addWidget(z_label);
  controlBox->addWidget(z_selector);
  controlBox->addStretch();
  controlBox->addWidget(pos_label);
  controlBox->addStretch();
  controlBox->addWidget(x_label);
  controlBox->addWidget(y_label);
}

BlobSetPlotter::~BlobSetPlotter()
{
  deleteBlobSpaces();
  delete plotter;
  // do I need to delete other stuff, or is that taken care of by the
  // parent methods?
}

void BlobSetPlotter::setBlobSet(QString collection_name, unsigned int sub_id, vector<blob_set_space*> blobs)
{
  if(!blobs.size())
    return;
  if(blobSpaces.size())
    deleteBlobSpaces();
  blobSpaces = blobs;
  coll_name = collection_name;
  id = sub_id;
  
  //  blockSelectorSignals(true);
  blobSetSelector->setRange(0, blobSpaces.size()-1);
  if(blobSetSelector->value() == 0){
    setChanged(0);
    return;
  }
  blobSetSelector->setValue(0);
  // blobSelector->setRange(0, blobSpaces[0]->size()-1);
  // blobSelector->setValue(0);
  // z_selector->setRange(0, blobSpaces[0]->space(0)->pos.d - 1);
  // z_selector->setValue( (blobSpaces[0]->space(0)->pos.d / 2) - 1);
  // blockSelectorSignals(false);

  
  // plot(0);
}

void BlobSetPlotter::plot(int ignored)
{
  ignored = ignored;   // avoid warnings.
  unsigned int bs_id = (unsigned int)blobSetSelector->value(); 
  unsigned int b_id = (unsigned int)blobSelector->value();
  unsigned int z = (unsigned int)z_selector->value();
  
  if(bs_id >= blobSpaces.size())
    return;
  if(b_id >= blobSpaces[bs_id]->size())
    return;
  blob_space* bs = blobSpaces[bs_id]->space(b_id);
  if(!bs) return;
  if(z >= bs->pos.d) return;
  float* values = bs->values + z * (bs->pos.w * bs->pos.h);
  bool* mb = bs->membership + z * (bs->pos.w * bs->pos.h);
  plotter->setData(values, bs->pos.w, bs->pos.h, true, true);
  plotter->setPointMask(mb, bs->pos.w, bs->pos.h, true);  // this might cause flickering.
  
}

void BlobSetPlotter::setChanged(int bset)
{
  bset = bset;
  unsigned int bs_id = (unsigned int)blobSetSelector->value();
  if(bs_id >= blobSpaces.size())
    return;
  cout << "BlobSetPlotter::setChanged size of blobSpaces " << bs_id << "  : ";
  cout << blobSpaces[bs_id]->size() << endl;
  blobSelector->setRange(0, blobSpaces[bs_id]->size() -1 );
  if(!blobSelector->value()){  // no signal if the value doesn't change.
    blobChanged(0);
    return;
  }
  blobSelector->setValue(0);
}

void BlobSetPlotter::blobChanged(int bid)
{
  bid = bid;
  unsigned int bs_id = (unsigned int)blobSetSelector->value(); 
  unsigned int b_id = (unsigned int)blobSelector->value();
  if(bs_id >= blobSpaces.size())
    return;
  if(b_id >= blobSpaces[bs_id]->size())
    return;
  QString posString;
  QTextStream qts(&posString);
  stack_info pos = blobSpaces[bs_id]->space(b_id)->pos;
  qts << pos.x << "," << pos.y << "," << pos.z;
  pos_label->setText(posString);

  z_selector->setRange(0, (blobSpaces[bs_id]->space(b_id)->pos.d - 1));
  float min, max;
  getRange(blobSpaces[bs_id]->space(b_id), min, max);
  plotter->useConstantRange(true, 0, max);
  int z = (blobSpaces[bs_id]->space(b_id)->pos.d / 2) - 1;
  if(z == z_selector->value()){
    plot(z);
    return;
  }
  z_selector->setValue((blobSpaces[bs_id]->space(b_id)->pos.d / 2) - 1);  
}

void BlobSetPlotter::setMousePos(int xp, float yp)
{
  QString lstring;
  lstring.setNum(xp);
  x_label->setText(lstring);
  lstring.setNum(yp);
  y_label->setText(lstring);
}

void BlobSetPlotter::deleteBlobSpaces()
{
  for(unsigned int i=0; i < blobSpaces.size(); ++i)
    delete blobSpaces[i];
}

void BlobSetPlotter::blockSelectorSignals(bool block)
{
  blobSetSelector->blockSignals(block);
  blobSelector->blockSignals(block);
  z_selector->blockSignals(block);
}

void BlobSetPlotter::getRange(blob_space* space, float& min, float& max)
{
  unsigned int r = space->pos.w * space->pos.h * space->pos.d;
  max = min = space->values[0];
  for(unsigned int i=0; i < r; ++i){
    max = space->values[i] > max ? space->values[i] : max;
    min = space->values[i] < min ? space->values[i] : min;
  }
}
