#ifndef OVERLAPEDITORWINDOW_H
#define OVERLAPEDITORWINDOW_H

#include <QWidget>

class GLImage;

struct FileSetInfo;
class OverlapEditor;
class QGraphicsView;

class OverlapEditorWindow : public QWidget
{
  Q_OBJECT

    public:
  OverlapEditorWindow(QWidget* parent=0);
  void setInfo(FileSetInfo* fsetInfo);

  private slots:
  void r_delta_y(int dy);
  void incrementScale(float ds);

 private:
  void scaleView(float scale);
  OverlapEditor* oEdit;
  QGraphicsView* view;
  float viewScale;
  GLImage* vertImage;
};


#endif
