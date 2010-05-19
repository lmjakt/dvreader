#ifndef OVERLAPEDITORWINDOW_H
#define OVERLAPEDITORWINDOW_H

#include <QWidget>
#include <QPoint>
#include <map>
#include <set>
#include "borderInformation.h"

class GLImage;
class BorderInfo;
class QGridLayout;
class QDoubleSpinBox;
class QRadioButton;

struct FileSetInfo;
class OverlapEditor;


class OverlapEditorWindow : public QWidget
{
  Q_OBJECT

    public:
  OverlapEditorWindow(QWidget* parent=0);
  void setInfo(FileSetInfo* fsetInfo);
  void setBorderImages(BorderInfo* binfo);

 signals:
  void newFrameSelected(float, float);
  void adjustFramePos(float, float, QPoint);
  void updateFileSetInfo();

  private slots:
  void r_delta_y(int dy);
  void incrementScale(float ds);
  void offSetChanged(QPoint p);
  void frameSelected(float x, float y);
  void requestAdjustment();
  void setChannelChecks(std::set<int> channels);
  void setScale(double s);
  void setOverlapImages();

 private:
  void clearImages();
  void paintOverlap(GLImage* image, POSITION pos, int wave, color_map& t, color_map& n);
  void updateGL();
  OverlapEditor* oEdit;
  float viewScale;
  GLImage* leftImage;
  GLImage* rightImage;
  GLImage* topImage;
  GLImage* bottomImage;
  BorderInfo* borderImage;
  QGridLayout* controlGrid;
  std::map<int, QRadioButton*> channelChecks;
  QDoubleSpinBox* scaleBox;
  //  std::map<float, std::map<float, BorderInfo*> > borderImages;

  int tex_width, tex_height;
  int t_big, t_small;
};


#endif
