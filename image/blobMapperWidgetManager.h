#ifndef BLOBMAPPERWIDGETMANAGER_H
#define BLOBMAPPERWIDGETMANAGER_H

#include "blobMapperWidget.h"
#include "superBlob.h"
#include "../linGraph/distPlotter.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <set>
#include <vector>
#include <string>

class BlobMapperWidgetManager : public QWidget
{
    Q_OBJECT

	enum Param {
	    VOLUME, SUM, MEAN, MAX, MIN
	};

	public:
    BlobMapperWidgetManager(QWidget* parent=0);
    ~BlobMapperWidgetManager();

    void addBlobMapper(BlobMapper* bm, fluorInfo& fInfo, std::string fName);
    std::set<BlobMapperWidget*> blobMapperWidgets();

 signals:
    void newColor();
    void newRep();

    private slots:
	void setParamType(int p);
    void deleteBlobWidget();
    void makeSuperBlobs();

 private:
    std::set<BlobMapperWidget*> blobWidgets;
    std::vector<SuperBlob*> superBlobs;
    //  QComboBox* blobTypeSelector;
    DistPlotter* distPlotter;
    DistPlotter* superDistPlotter;
    Param plotType;
    QVBoxLayout* vbox;

    void plotDistributions();
    void plotSuperDistributions();
    void deleteSuperBlobs();
};

#endif
