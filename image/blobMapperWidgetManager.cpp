#include "blobMapperWidgetManager.h"
#include "coordConverter.h"
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>
#include <QTextStream>
#include <QFileDialog>
#include <QSize>
#include <fstream>
#include <limits.h>

using namespace std;

BlobMapperWidgetManager::BlobMapperWidgetManager(QWidget* parent)
    : QWidget(parent)
{
    max_mapper_id = 1;
    blobClassifier = 0;
    QFont plotterFont = QFont(font().family(), font().pointSize()-0);

    QComboBox* paramChooser = new QComboBox(false, this);
    paramChooser->insertItem(BlobMapper::VOLUME, "Volume");
    paramChooser->insertItem(BlobMapper::SUM, "Sum");
    paramChooser->insertItem(BlobMapper::MEAN, "Mean");
    paramChooser->insertItem(BlobMapper::MAX, "Max");
    paramChooser->insertItem(BlobMapper::MIN, "Min");
    paramChooser->insertItem(BlobMapper::EXTENT, "Extent");
    paramChooser->insertItem(BlobMapper::SURFACE, "Surface");
    paramChooser->insertItem(BlobMapper::BACKGROUND, "Background");

    //blobTypeSelector = new QComboBox(false, this);
    //blobTypeSelector->insertItem(-1, "All");
    
    superWidget = new SuperBlobWidget();
    superWidget->setCaption("SuperBlobs");
    superWidget->setFont(plotterFont);
    connect(superWidget, SIGNAL(makeSuperBlobs(std::set<BlobMapperWidget*>)),
	    this, SLOT(makeSuperBlobs(std::set<BlobMapperWidget*>)) );
    connect(superWidget, SIGNAL(classifySuperBlobs(std::set<BlobMapper::Param>)),
	    this, SLOT(classifySuperBlobs(std::set<BlobMapper::Param>)) );
    connect(superWidget, SIGNAL(classifyBlobs(std::set<BlobMapperWidget*>)),
	    this, SLOT(classifyBlobs(std::set<BlobMapperWidget*>)) );

    connect(paramChooser, SIGNAL(activated(int)), this, SLOT(setParamType(int)) );
    plotType = BlobMapper::VOLUME;

    QPushButton* superButton = new QPushButton("Super", this);
    connect(superButton, SIGNAL(clicked()), this, SLOT(makeSuperBlobs()) );

    QPushButton* clearLimitsButton = new QPushButton("C.L.", this);
    clearLimitsButton->setToolTip("Clear Plot Limits");
    clearLimitsButton->setMaximumWidth(40);
    connect(clearLimitsButton, SIGNAL(clicked()), this, SLOT(clearPlotLimits()) );

    QPushButton* exportSuperButton = new QPushButton("E", this);
    exportSuperButton->setToolTip("Export SuperBlobs");
    QSize size = exportSuperButton->minimumSizeHint();
    cout << "SIZE THINGHY MINIMUM SIZE HINT FOR EXPORT SUPER BUTTON " << size.width() << "," << size.height() << endl; 
    exportSuperButton->setMaximumWidth(40);
    connect(exportSuperButton, SIGNAL(clicked()), this, SLOT(exportSuperBlobs()) );

    distPlotter = new DistPlotter(false);
    superDistPlotter = new DistPlotter(true);  // use this one only for setting limits

    distPlotter->setCaption("Blob distributions");
    superDistPlotter->setCaption("SuperBlob distributions");
    connect(superDistPlotter, SIGNAL(setLimits(float, float)), this, SLOT(setLimits(float, float)) );
    distPlotter->resize(500, 400);
    superDistPlotter->resize(500, 400);

    distPlotter->setFont(plotterFont);
    superDistPlotter->setFont(plotterFont);

    QPalette plotPalette;
    plotPalette.setColor(QPalette::Window, QColor(10, 10, 10));
    plotPalette.setColor(QPalette::WindowText, QColor(250, 0, 250));
    plotPalette.setColor(QPalette::Text, QColor(250, 0, 250));
    plotPalette.setColor(QPalette::Base, QColor(10, 10, 30));
    plotPalette.setColor(QPalette::Button, QColor(12, 12, 12));
    plotPalette.setColor(QPalette::ButtonText, QColor(120, 120, 120));
    distPlotter->setPalette(plotPalette);
    superDistPlotter->setPalette(plotPalette);

    distPlotter->setCaption("Blob distributions");
    superDistPlotter->setCaption("SuperBlob distributions");

    scatterPlotter = new BlobScatterPlot();
    scatterPlotter->setCaption("Blob Scatter Plot");
    connect(scatterPlotter, SIGNAL(plotPars(BlobMapper::Param, BlobMapper::Param)),
	    this, SLOT(scatterPlot(BlobMapper::Param, BlobMapper::Param)) );
    connect(scatterPlotter, SIGNAL(blobsSelected(std::vector<std::vector<bool> >)),
	    this, SLOT(filterBlobs(std::vector<std::vector<bool> >)) );
    scatterPlotter->setPalette(plotPalette);
    scatterPlotter->setFont(plotterFont);
    scatterPlotter->resize(400, 400);


    QVBoxLayout* mainBox = new QVBoxLayout(this);
    mainBox->setSpacing(1);
    mainBox->setMargin(1);
    
    vbox = new QVBoxLayout();
    mainBox->addLayout(vbox);

    QHBoxLayout* hbox = new QHBoxLayout();
    mainBox->addLayout(hbox);
    hbox->addWidget(clearLimitsButton);
    hbox->addStretch();
    hbox->addWidget(exportSuperButton);
    hbox->addWidget(superButton);
    //hbox->addWidget(blobTypeSelector);
    hbox->addWidget(paramChooser);
}

BlobMapperWidgetManager::~BlobMapperWidgetManager(){
    // we should delete the blobMappers .. 
  // and also the superBlobs
  delete blobClassifier;
}

void BlobMapperWidgetManager::addBlobMapper(BlobMapper* bm, fluorInfo& fInfo, string fName, QColor c){
    if(max_mapper_id > INT_MAX){
      cerr << "BlobMapperWidgetManager::addBlobMapper A maximum of 31 mappers allowed. max_mapper_id overflowing" << endl;
      return;
    }
    bm->setMapId(max_mapper_id);
    max_mapper_id *= 2;
    BlobMapperWidget* bmw = new BlobMapperWidget(bm, fInfo, fName, c, this);
    connect(bmw, SIGNAL(newColor()), this, SLOT(replot()) );
    connect(bmw, SIGNAL(includeDistChanged()), this, SLOT(replot()) );
  
    connect(bmw, SIGNAL(newColor()), this, SIGNAL(newColor()) );
    connect(bmw, SIGNAL(newRep()), this, SIGNAL(newRep()) );
    connect(bmw, SIGNAL(deleteMe()), this, SLOT(deleteBlobWidget()) );
  

    blobWidgets.insert(bmw);
    vbox->addWidget(bmw);
    bmw->show();

    QString label;
    QTextStream ls(&label);
    ls << fInfo.excitation << " -> " << fInfo.emission << " : " << bm->minimum();
    //    blobTypeSelector->insertItem(label);

    //distPlotter->setCaption(fName.c_str());
    plotDistributions();
    //    plotSuperDistributions();
    
    cout << "calling superwidget add blobmapperwidgtet" << endl; 
    superWidget->addBlobMapperWidget(bmw);
    cout << "add blobmapper widget returned, now calling show" << endl;
    superWidget->show();
}

set<BlobMapperWidget*> BlobMapperWidgetManager::blobMapperWidgets(){
    return(blobWidgets);
}

void BlobMapperWidgetManager::setParamType(int p){
    // This is kind of ugly, but I don't know a better way
    switch(p){
	case BlobMapper::VOLUME:
	    plotType = BlobMapper::VOLUME;
	    break;
	case BlobMapper::SUM:
	    plotType = BlobMapper::SUM;
	    break;
	case BlobMapper::MEAN:
	    plotType = BlobMapper::MEAN;
	    break;
	case BlobMapper::MAX:
	    plotType = BlobMapper::MAX;
	    break;
	case BlobMapper::MIN:
	    plotType = BlobMapper::MIN;
	    break;
        case BlobMapper::EXTENT:
            plotType = BlobMapper::EXTENT;
            break;
	case BlobMapper::SURFACE:
	    plotType = BlobMapper::SURFACE;
	    break;
	case BlobMapper::BACKGROUND:
	    plotType = BlobMapper::BACKGROUND;
	    break;
	default:
	    plotType = BlobMapper::SUM;
    }
    plotDistributions();
    plotSuperDistributions();
}

void BlobMapperWidgetManager::deleteBlobWidget(){
    BlobMapperWidget* bmw = (BlobMapperWidget*)sender();
    if(!blobWidgets.count(bmw)){
	cerr << "BlobMapperWidgetManager::deleteBlobWidget requested to delete an unknown blobWidget" << endl;
	return;
    }
    blobWidgets.erase(bmw);
    cout << "calling superwidget to remove bmw" << endl;
    superWidget->removeBlobMapperWidget(bmw);
    cout << "superwidgtet did that" << endl;
    bmw->deleteLater();
    cout << "and finally we've deleted the actualy bmw thingy" << endl;
    // if we have any superBlobs we would need to remove any that point to this particular mapper;
    // since we don't actually have a way of doing that at the moment, let's just delete all the points.
    deleteSuperBlobs();
}

void BlobMapperWidgetManager::makeSuperBlobs(){
  makeSuperBlobs(blobWidgets);
}

void BlobMapperWidgetManager::makeSuperBlobs(set<BlobMapperWidget*> bmw){
    if(bmw.size() < 2){
	cerr << "BlobMapperWidgetManager::makeSuperBlobs : Not enough blobWidgets to make superBlobs" << endl;
	return;
    }
    set<BlobMapperWidget*>::iterator it = bmw.begin();
    if(it == bmw.end())
	return;

    deleteSuperBlobs();

    BlobMapper* seedMapper = (*it)->blobMapper();
    vector<BlobMapper*> mappers;
    it++;
    while(it != bmw.end()){
	mappers.push_back((*it)->blobMapper());
	it++;
    }
    
    superBlobs = seedMapper->overlapBlobs(mappers);
    cout << "MakeSuperBlobs made a total of " << superBlobs.size() << "  superBlobs" << endl;
    plotSuperDistributions();
    cout << "calling scatterPlotter->x_param and so on" << endl;
    scatterPlot(scatterPlotter->x_param(), scatterPlotter->y_param());
    superWidget->setSuperBlobs(superBlobs);
}

void BlobMapperWidgetManager::exportSuperBlobs(){
    cout << "Export Superblobs function" << endl;
    if(!superBlobs.size() || !blobWidgets.size()){
	cerr << "BlobMapperWidgetManager::exportSuperBlobs : no blobWidgets or no superBlobs" << endl;
	return;
    }
    QString fname = QFileDialog::getSaveFileName();
    if(fname.isNull())
	return;
//    set<BlobMapperWidget*>::iterator it = blobWidgets.begin();
    int width, height, depth;
    width = height = depth = 1;
    //    BlobMapperWidget* bmw = *(blobWidgets.begin());
    //bmw->dimensions(width, height, depth);
    CoordConverter cc(width, height, depth);

    ofstream of(fname.ascii());
    if(!of){
	cerr << "BlobMapperWidgetManager unable to open file : " << fname.ascii() << endl;
	return;
    }
    int bid = 0;
    of << "id\tsuper_id\tsub_id\tmembership\tmapper_id\tx\ty\tz\tmin_x\tmax_x\tmin_y\tmax_y\tmin_z\tmax_z\tsum\tvolume\tsurface\tmean\textent\tmin\tmax\tbackground\n";
    for(uint i=0; i < superBlobs.size(); ++i){
	uint membership = superBlobs[i]->membership;
	for(uint j=0; j < superBlobs[i]->blobs.size(); ++j){
	    int x, y, z;
	    blob* b = superBlobs[i]->blobs[j].b;
	    BlobMapper* bm = superBlobs[i]->blobs[j].mapper;
	    cc.vol(b->peakPos, x, y, z);
	    of << bid++ << "\t" << i << "\t" << j << "\t" << membership << "\t" << superBlobs[i]->blobs[j].mapper_id  
	       << "\t" << x << "\t" << y << "\t" << z << "\t" << b->min_x << "\t" << b->max_x
	       << "\t" << b->min_y << "\t" << b->max_y << "\t" << b->min_z << "\t" << b->max_z
	       << "\t" << b->sum << "\t" << b->points.size()
	       << "\t" << bm->getParameter(b, BlobMapper::SURFACE)
	       << "\t" << bm->getParameter(b, BlobMapper::MEAN)
	       << "\t" << bm->getParameter(b, BlobMapper::EXTENT)
	       << "\t" << bm->getParameter(b, BlobMapper::MIN)
	       << "\t" << bm->getParameter(b, BlobMapper::MAX)
	       << "\t" << bm->getParameter(b, BlobMapper::BACKGROUND)
	       << endl;
	}
    }
        
}

void BlobMapperWidgetManager::replot(){
  plotDistributions();
  plotSuperDistributions();
  scatterPlot(scatterPlotter->x_param(), scatterPlotter->y_param());
}

void BlobMapperWidgetManager::clearPlotLimits(){
  for(set<BlobMapperWidget*>::iterator it = blobWidgets.begin(); it != blobWidgets.end(); ++it){
    if((*it)->plotDistribution())
      (*it)->clearPlotLimits();
  }
  plotSuperDistributions();
  scatterPlot(scatterPlotter->x_param(), scatterPlotter->y_param());
}

void BlobMapperWidgetManager::setLimits(float l, float r){
  for(set<BlobMapperWidget*>::iterator it = blobWidgets.begin(); it != blobWidgets.end(); ++it){
    if((*it)->plotDistribution())
      (*it)->setPlotLimits(plotType, l, r);
  }
  emit newLimits();
}

void BlobMapperWidgetManager::filterBlobs(std::vector<std::vector<bool> > blobSelection){
    // we need to set these at the blob level, so we'll need to first get an equivalent 
    // vector of blobs..
    map<unsigned int, vector<blob*> > blobs = collectSuperBlobBlobs();
    if(blobs.size() != blobSelection.size()){
	cerr << "BlobMapperWidgetManager::filterBlobs selection and blobs are of different sizes : " << blobs.size() << " : " << blobSelection.size() << endl;
	return;
    }
    uint i=0;
    for(map<unsigned int, vector<blob*> >::iterator it=blobs.begin(); it != blobs.end(); ++it){
	if((*it).second.size() != blobSelection[i].size()){
	    cerr << "BlobMapperWidgetManager::filterBlobs uneven sizes at " << i << " : " << (*it).second.size() << "  : " << blobSelection[i].size() << endl;
	    return;
	}
	for(uint j=0; j < (*it).second.size(); ++j)
	    (*it).second[j]->active = blobSelection[i][j];
	++i;
    }
    emit newRep();  // to force a redraw
}


void BlobMapperWidgetManager::scatterPlot(BlobMapper::Param xpar, BlobMapper::Param ypar)
{  
  cout << "scatterPlot what's up " << endl;
  if(!superBlobs.size() && !superDistPlotter->isVisible())
    return;

  vector<vector<float> > xvalues;
  vector<vector<float> > yvalues;
  vector<QColor> colors;
  pl_limits plotLimits;
  
  collectSuperBlobValues(xvalues, colors, plotLimits, xpar);
  colors.clear();
  collectSuperBlobValues(yvalues, colors, plotLimits, ypar);
  
  cout << "collected data for scatter plot " << xvalues.size() << " : " << yvalues.size() << endl;

  scatterPlotter->setData(xvalues, yvalues, colors);
  scatterPlotter->show();
}

void BlobMapperWidgetManager::classifySuperBlobs(set<BlobMapper::Param> param){
  cout << "classify super blobs" << endl;
  if(!blobClassifier)
    blobClassifier = new BlobClassifier();
  if(!superBlobs.size() || !param.size())
    return;
  blobClassifier->setParameters(param);
  blobClassifier->setSuperBlobs(superBlobs);
  cout << "set the superblobs and the parameters.." << endl;
  
}

void BlobMapperWidgetManager::classifyBlobs(set<BlobMapperWidget*> bmw){
  if(!blobClassifier){
    cerr << "BlobMapperWidgetManager no trained blobClassifier. Unable to classify" << endl;
    return;
  }
  cout << "This is the blobmapperwidgetmanager trying to classify a total of " << bmw.size()
       << "  mappers blobs" << endl;
  vector<BlobMapper*> mappers;
  vector<BlobMapperWidget*> widgets;  // a bit silly, but it helps
  for(set<BlobMapperWidget*>::iterator it=bmw.begin(); it != bmw.end(); ++it){
    mappers.push_back((*it)->blobMapper());
    widgets.push_back(*it);
  }
  vector<BlobClassCounts> counts = blobClassifier->classifyBlobs(mappers);
  // Then go through and do something that make sense.
  superWidget->setClassCounts(counts);
}

void BlobMapperWidgetManager::plotDistributions(){
    vector<vector<float> > values(blobWidgets.size());
    vector<QColor> colors(blobWidgets.size());
    uint i=0;
    for(set<BlobMapperWidget*>::iterator it=blobWidgets.begin(); it != blobWidgets.end(); ++it){
        BlobMapper* bm = (*it)->blobMapper();
	colors[i] = (*it)->color();
	set<blob*>& blobs = (*it)->blobs();
	values[i].reserve(blobs.size());
	cout << "BlobMapperWidget reserving size for " << i << "  : " << blobs.size() << endl;
	for(set<blob*>::iterator bit=blobs.begin(); bit != blobs.end(); ++bit){
	  values[i].push_back( bm->getParameter((*bit), plotType) );
	}
	++i;
    }
    distPlotter->setData(values, colors, true);
    distPlotter->show();
}

void BlobMapperWidgetManager::plotSuperDistributions(){
  if(!superBlobs.size() && !superDistPlotter->isVisible())
    return;
  
  vector<vector<float> > plotValues;
  vector<QColor> plotColors;
  pl_limits plotLimits;
  
  collectSuperBlobValues(plotValues, plotColors, plotLimits, plotType);

  if(plotLimits.left < plotLimits.right){
    cout << "Found a previous set of limits " << plotLimits.left << "  " << plotLimits.right << endl;
    superDistPlotter->setPlotLimits(plotLimits.left, plotLimits.right, false);
    superDistPlotter->setData(plotValues, plotColors, false);
  }else{
    superDistPlotter->setData(plotValues, plotColors, true);
  }
  superDistPlotter->show();
}
  
void BlobMapperWidgetManager::collectSuperBlobValues(vector<vector<float> >& plotValues, vector<QColor>& plotColors,
						     pl_limits& plotLimits, BlobMapper::Param p_type){
  map<unsigned int, QColor> widgetColors = widgetPlotColors();
  map<BlobMapper*, bool> includeMap = distIncludeMap();
  //  map<unsigned int, bool> includeMap = distIncludeMap();
  plotLimits = get_plotLimits(p_type);

  map<unsigned int, vector<float> > values;
  for(uint i=0; i < superBlobs.size(); ++i){
    float v;
    for(uint j=0; j < superBlobs[i]->blobs.size(); ++j){
      if(!( includeMap[ superBlobs[i]->blobs[j].mapper ] ))
	continue;
      blob* b = superBlobs[i]->blobs[j].b;
      BlobMapper* bm = superBlobs[i]->blobs[j].mapper;
      v = bm->getParameter(b, p_type);
      values[superBlobs[i]->membership].push_back(v);
    }
  }
  // then work out the colors that we'll need using  
  for(map<unsigned int, vector<float> >::iterator it=values.begin(); it != values.end(); ++it){
    int r, g, b;
    r = g = b = 0;
    //unsigned int component = 1;
    for(map<uint, QColor>::iterator cit=widgetColors.begin(); cit != widgetColors.end(); ++cit){
    //    for(uint i=0; i < widgetColors.size(); ++i){
      if((*it).first & (*cit).first){
	r += (*cit).second.red();
	g += (*cit).second.green();
	b += (*cit).second.blue();
      }
      //      component *= 2;
    }
    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;
    plotColors.push_back(QColor(r, g, b));
    plotValues.push_back((*it).second);
  }
}

// This function only exists in order to allow a collection of blob*  ordered in the
// same manner as values collected by collectSuperBlobValues. This is a hack in order
// to be able something like a vector<vector<bool> > instructing us as to which blobs
// to draw. 
map<unsigned int, vector<blob*> > BlobMapperWidgetManager::collectSuperBlobBlobs(){
    map<BlobMapper*, bool> includeMap = distIncludeMap();
    //    map<unsigned int, bool> includeMap = distIncludeMap();
    map<unsigned int, vector<blob*> > blobMap;

    for(uint i=0; i < superBlobs.size(); ++i){
	for(uint j=0; j < superBlobs[i]->blobs.size(); ++j){
	    if(!( includeMap[ superBlobs[i]->blobs[j].mapper ] ))
		continue;
	    blobMap[superBlobs[i]->membership].push_back( superBlobs[i]->blobs[j].b );
	}
    }
    return(blobMap);
}

map<BlobMapper*, bool> BlobMapperWidgetManager::distIncludeMap(){
    // A map that is dependent on the mapper ids and super blob ids
    // derived in makeSuperBlobs.
    map<BlobMapper*, bool> includeMap;
    //    map<unsigned int, bool> includeMap;
    unsigned int map_id = 1;
    for(set<BlobMapperWidget*>::iterator it=blobWidgets.begin(); it != blobWidgets.end(); ++it){
      includeMap[(*it)->blobMapper()] = (*it)->plotDistribution();
	//	includeMap[map_id] = (*it)->plotDistribution();
	map_id *= 2;
    }
    return(includeMap);
}

map<unsigned int, QColor> BlobMapperWidgetManager::widgetPlotColors(){
    map<unsigned int, QColor> widgetColors;
    for(set<BlobMapperWidget*>::iterator it=blobWidgets.begin(); it != blobWidgets.end(); ++it){
      widgetColors.insert(make_pair((*it)->blobMapper()->mapId(), (*it)->color()) );
    }
    return(widgetColors);
}

pl_limits BlobMapperWidgetManager::get_plotLimits(BlobMapper::Param p_type){
    /// Not a very good function..
    pl_limits pl;
    for(set<BlobMapperWidget*>::iterator it=blobWidgets.begin(); it != blobWidgets.end(); ++it){
	if((*it)->plotDistribution() && (*it)->pLimits().count(p_type))
	    pl = (*it)->pLimits()[p_type];
    }
    return(pl);
}

void BlobMapperWidgetManager::deleteSuperBlobs(){
    for(uint i=0; i < superBlobs.size(); ++i)
	delete superBlobs[i];
    superBlobs.resize(0);
}
