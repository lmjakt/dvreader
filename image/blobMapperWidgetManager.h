#ifndef BLOBMAPPERWIDGETMANAGER_H
#define BLOBMAPPERWIDGETMANAGER_H

#include "blobMapperWidget.h"
#include "superBlob.h"
#include "superBlobWidget.h"
#include "../linGraph/distPlotter.h"
#include "blobScatterPlot.h"
#include "blobClassifier.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QColor>
#include <set>
#include <vector>
#include <string>

class BlobMapperWidgetManager : public QWidget
{
    Q_OBJECT
      public:
    BlobMapperWidgetManager(QWidget* parent=0);
    ~BlobMapperWidgetManager();

    void addBlobMapper(BlobMapper* bm, fluorInfo& fInfo, std::string fName, QColor c);
    std::set<BlobMapperWidget*> blobMapperWidgets();

 signals:
    void newColor();
    void newRep();
    void newLimits();

    private slots:
	void setParamType(int p);
    void deleteBlobWidget();
    void makeSuperBlobs();
    void setSuperIds();
    void makeSuperBlobs(std::set<BlobMapperWidget*> bmw);
    void exportSuperBlobs();
    void replot();
    void clearPlotLimits();
    void setLimits(float l, float r);
    void filterBlobs(std::vector<std::vector<bool> > blobSelection);
    void scatterPlot(BlobMapper::Param xpar, BlobMapper::Param ypar);
    void classifySuperBlobs(std::set<BlobMapper::Param> param);
    void classifyBlobs(std::set<BlobMapperWidget*> bmw);

 private:
    std::set<BlobMapperWidget*> blobWidgets;
    std::vector<SuperBlob*> superBlobs;
    unsigned int max_mapper_id;
    SuperBlobWidget* superWidget;
    //  QComboBox* blobTypeSelector;
    DistPlotter* distPlotter;
    DistPlotter* superDistPlotter;
    BlobMapper::Param plotType;
    BlobScatterPlot* scatterPlotter;
    BlobClassifier* blobClassifier;
    QVBoxLayout* vbox;

    void plotDistributions();
    void collectSuperBlobValues(std::vector<std::vector<float > >& plotValues, 
				std::vector<QColor>& plotColors, pl_limits& plotLimits,
				BlobMapper::Param p_type);
    std::map<unsigned int, std::vector<blob*> > collectSuperBlobBlobs();
    std::map<BlobMapper*, bool> distIncludeMap();
    //    std::map<unsigned int, bool> distIncludeMap();
    std::map<unsigned int, QColor> widgetPlotColors();
    pl_limits get_plotLimits(BlobMapper::Param p_type);
    void plotSuperDistributions();
    void deleteSuperBlobs();
};

#endif
