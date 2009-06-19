#ifndef SUPERBLOBWIDGET_H
#define SUPERBLOBWIDGET_H

#include "blobMapperWidget.h"
#include "blobMapper.h"
#include "blobClassifier.h"
#include <QWidget>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <map>
#include <set>
#include <vector>

class SuperBlobWidget : public QWidget
{
  struct sblobClassInfo
  {
    QLabel* id_label;
    QLabel* sizeLabel;
    QLabel* reClassSize;
    sblobClassInfo()
    {
      id_label = sizeLabel = reClassSize = 0;
    }
    ~sblobClassInfo(){
      id_label->hide(); sizeLabel->hide(); reClassSize->hide();
      delete id_label; delete sizeLabel; delete reClassSize;
    }
  };

  struct BlobMapperInfo
  {
    BlobMapperWidget* bmw;
    QLabel* exLabel;
    QLabel* emLabel;
    QLabel* minLabel;
    QLabel* sizeLabel;
    QLabel* idLabel;
    QCheckBox* superCheck;
    std::map<int, sblobClassInfo*> classes;
    //    std::vector<sblobClassInfo*> classes;
    BlobMapperInfo()
    {
      bmw = 0;
      exLabel = emLabel = minLabel = sizeLabel = idLabel = 0;
      superCheck = 0;
    }
    ~BlobMapperInfo()
    {
      exLabel->hide(); emLabel->hide(); minLabel->hide(); sizeLabel->hide(); idLabel->hide();
      delete exLabel; delete emLabel; delete minLabel; delete sizeLabel; delete idLabel;
      delete superCheck;
      for(uint i=0; i < classes.size(); ++i)
	delete classes[i];
    }
  };

  Q_OBJECT
    public:
  SuperBlobWidget(QWidget* parent=0);
  ~SuperBlobWidget();

  void addBlobMapperWidget(BlobMapperWidget* bmw);
  void removeBlobMapperWidget(BlobMapperWidget* bmw);
  void setSuperBlobs(std::vector<SuperBlob*>& sblobs);
  void setClassCounts(std::vector<BlobClassCounts> classCounts);
  
 signals:
  void makeSuperBlobs(std::set<BlobMapperWidget*>);
  void classifySuperBlobs(std::set<BlobMapper::Param>);
  void classifyBlobs(std::set<BlobMapperWidget*>);

 private slots:
  void paramSelectionChanged();
  void reqSuperBlobs();
  void classifyBlobs();
  void trainClassifier();
  
 private:
  //  BlobClassifier* classifier;
  std::map<BlobMapper::Param, QCheckBox*> paramChecks;
  std::map<BlobMapper*, BlobMapperInfo*> blobInfo;
  int blobRowCounter;
  // std::map<BlobMapper*, std::map<uint, std::vector<blob*> > > orgBlobs;
  
  QGridLayout* grid;

  void makeParamBox(BlobMapper::Param p, QString label);
  QLabel* makeFloatLabel(float f);
  void setLabels();
  void addLabels(BlobMapperInfo* bi);
  void clearSuperBlobInfo();

};


#endif
