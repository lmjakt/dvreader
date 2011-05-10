#ifndef BLOBSETMAPPER_H
#define BLOBSETMAPPER_H

#include <QWidget>
#include <QString>
#include <vector>

class LinePlotter;
class QSpinBox;
class QLabel;
class blob_set_space;
struct blob_space;

class BlobSetPlotter : public QWidget
{
  Q_OBJECT
    public:
  BlobSetPlotter(QWidget* parent=0);
  ~BlobSetPlotter();
  
  void setBlobSet(QString collection_name, unsigned int sub_id, std::vector<blob_set_space*> blobs);

 signals:
  // cell collection name, cell id, blob_set id, blob id
  void plotBlob(QString, unsigned int, unsigned int, unsigned int); 
  
 private slots:
  void plot(int ignored);
  void setChanged(int bset);
  void blobChanged(int bid);
  void setMousePos(int xp, float yp);

 private:
  std::vector<blob_set_space*> blobSpaces;
  LinePlotter* plotter;

  QString coll_name;
  unsigned int id;
  
  QSpinBox* blobSetSelector;
  QSpinBox* blobSelector;
  QSpinBox* z_selector;

  QLabel* y_label;
  QLabel* x_label;

  QLabel* pos_label;

  void deleteBlobSpaces();
  void blockSelectorSignals(bool block);
  void getRange(blob_space* space, float& min, float& max);
};
#endif
