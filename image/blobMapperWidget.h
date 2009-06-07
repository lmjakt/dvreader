#ifndef BLOBMAPPERWIDGET_H
#define BLOBMAPPERWIDGET_H

#include "blobMapper.h"
#include "../dataStructs.h"  // for fluorInfo
#include <QWidget>
#include <QToolButton>
#include <QIcon>
#include <QColor>
#include <vector>
#include <set>
#include <map>
#include <string>


struct RepIcon {
    enum BlobRepresentation {
      NO_REP, OUTLINE, PEAK, FILLED
    };
    BlobRepresentation rep;
    QIcon icon;
    RepIcon(){
	rep = NO_REP;
    }
    RepIcon(QIcon ic, BlobRepresentation r){
	rep = r;
	icon = ic;
    }
};
    
struct pl_limits {
  float left;
  float right;
  pl_limits(){
    left = right = 0;
  }
  pl_limits(float l, float r){
    left = l; right = r;
  }
};

class BlobMapperWidget : public QWidget
{
    Q_OBJECT
	public:
  
  BlobMapperWidget(BlobMapper* bmapper, fluorInfo& fInfo, std::string fname, QColor c, QWidget* parent=0);
  ~BlobMapperWidget();

  std::set<blob*>& blobs();
  BlobMapper* blobMapper();
  RepIcon::BlobRepresentation blobRep();
  void color(float& r, float& g, float& b);
  QColor color();
  bool plotDistribution();
  std::map<BlobMapper::Param, pl_limits> pLimits();
  void setPlotLimits(BlobMapper::Param p, float l, float r);
  void clearPlotLimits();
  void clearPlotLimits(BlobMapper::Param p);
  bool filterBlob(blob* b);  // filters on plotLimits, returns true if it passes
  //  float getParameter(blob* b, Param p);
  void dimensions(int& w, int& h, int& d){
      mapper->dimensions(w, h, d);
  }

  public slots:
  void exportBlobs();

  private slots:
  void setColor();
  void changeRep();
  void eatNeighbors();
 signals:
  void newColor();
  void newRep();
  void deleteMe();
  void includeDistChanged();

 private:
    BlobMapper* mapper;
    fluorInfo fluor;
    std::string fileName;
    unsigned int currentRep;
    std::vector<RepIcon> icons;
    QToolButton* eatNeighborButton;
    QToolButton* drawDisButton;
    
    QToolButton* colorButton;
    QColor currentColor;
    QToolButton* drawTypeButton;

    std::map<BlobMapper::Param, pl_limits> plotLimits;
    void makeIcons();
};


#endif
