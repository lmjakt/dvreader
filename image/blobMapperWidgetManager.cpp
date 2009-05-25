#include "blobMapperWidgetManager.h"
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>
#include <QTextStream>

using namespace std;

BlobMapperWidgetManager::BlobMapperWidgetManager(QWidget* parent)
    : QWidget(parent)
{

    QComboBox* paramChooser = new QComboBox(false, this);
    paramChooser->insertItem(BlobMapperWidget::VOLUME, "Volume");
    paramChooser->insertItem(BlobMapperWidget::SUM, "Sum");
    paramChooser->insertItem(BlobMapperWidget::MEAN, "Mean");
    paramChooser->insertItem(BlobMapperWidget::MAX, "Max");
    paramChooser->insertItem(BlobMapperWidget::MIN, "Min");

    //blobTypeSelector = new QComboBox(false, this);
    //blobTypeSelector->insertItem(-1, "All");
    
    connect(paramChooser, SIGNAL(activated(int)), this, SLOT(setParamType(int)) );
    plotType = BlobMapperWidget::VOLUME;

    QPushButton* superButton = new QPushButton("Super", this);
    connect(superButton, SIGNAL(clicked()), this, SLOT(makeSuperBlobs()) );

    QPushButton* clearLimitsButton = new QPushButton("C.L.", this);
    clearLimitsButton->setToolTip("Clear Plot Limits");
    connect(clearLimitsButton, SIGNAL(clicked()), this, SLOT(clearPlotLimits()) );

    distPlotter = new DistPlotter(false);
    superDistPlotter = new DistPlotter(true);  // use this one only for setting limits

    distPlotter->setCaption("Blob distributions");
    superDistPlotter->setCaption("SuperBlob distributions");
    connect(superDistPlotter, SIGNAL(setLimits(float, float)), this, SLOT(setLimits(float, float)) );
    distPlotter->resize(500, 400);
    superDistPlotter->resize(500, 400);

    distPlotter->setFont(font());
    superDistPlotter->setFont(font());

    QPalette plotPalette;
    plotPalette.setColor(QPalette::Window, QColor(10, 10, 10));
    plotPalette.setColor(QPalette::WindowText, QColor(250, 0, 250));
    plotPalette.setColor(QPalette::Text, QColor(250, 0, 250));
    plotPalette.setColor(QPalette::Base, QColor(10, 10, 30));
    distPlotter->setPalette(plotPalette);
    superDistPlotter->setPalette(plotPalette);

    QVBoxLayout* mainBox = new QVBoxLayout(this);
    mainBox->setSpacing(1);
    mainBox->setMargin(1);
    
    vbox = new QVBoxLayout();
    mainBox->addLayout(vbox);

    QHBoxLayout* hbox = new QHBoxLayout();
    mainBox->addLayout(hbox);
    hbox->addWidget(clearLimitsButton);
    hbox->addStretch();
    hbox->addWidget(superButton);
    //hbox->addWidget(blobTypeSelector);
    hbox->addWidget(paramChooser);
}

BlobMapperWidgetManager::~BlobMapperWidgetManager(){
    // we should delete the blobMappers .. 
}

void BlobMapperWidgetManager::addBlobMapper(BlobMapper* bm, fluorInfo& fInfo, string fName, QColor c){
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
    plotSuperDistributions();
}

set<BlobMapperWidget*> BlobMapperWidgetManager::blobMapperWidgets(){
    return(blobWidgets);
}

void BlobMapperWidgetManager::setParamType(int p){
    // This is kind of ugly, but I don't know a better way
    switch(p){
	case BlobMapperWidget::VOLUME:
	    plotType = BlobMapperWidget::VOLUME;
	    break;
	case BlobMapperWidget::SUM:
	    plotType = BlobMapperWidget::SUM;
	    break;
	case BlobMapperWidget::MEAN:
	    plotType = BlobMapperWidget::MEAN;
	    break;
	case BlobMapperWidget::MAX:
	    plotType = BlobMapperWidget::MAX;
	    break;
	case BlobMapperWidget::MIN:
	    plotType = BlobMapperWidget::MIN;
	    break;
	default:
	    plotType = BlobMapperWidget::SUM;
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
    delete bmw;
    // if we have any superBlobs we would need to remove any that point to this particular mapper;
    // since we don't actually have a way of doing that at the moment, let's just delete all the points.
    deleteSuperBlobs();
}

void BlobMapperWidgetManager::makeSuperBlobs(){
    if(blobWidgets.size() < 2){
	cerr << "BlobMapperWidgetManager::makeSuperBlobs : Not enough blobWidgets to make superBlobs" << endl;
	return;
    }
    set<BlobMapperWidget*>::iterator it = blobWidgets.begin();
    if(it == blobWidgets.end())
	return;

    deleteSuperBlobs();

    BlobMapper* seedMapper = (*it)->blobMapper();
    vector<BlobMapper*> mappers;
    it++;
    while(it != blobWidgets.end()){
	mappers.push_back((*it)->blobMapper());
	it++;
    }
    
    superBlobs = seedMapper->overlapBlobs(mappers);
    cout << "MakeSuperBlobs made a total of " << superBlobs.size() << "  superBlobs" << endl;
    plotSuperDistributions();
}

void BlobMapperWidgetManager::replot(){
  plotDistributions();
  plotSuperDistributions();
}

void BlobMapperWidgetManager::clearPlotLimits(){
  for(set<BlobMapperWidget*>::iterator it = blobWidgets.begin(); it != blobWidgets.end(); ++it){
    if((*it)->plotDistribution())
      (*it)->clearPlotLimits();
  }
}

void BlobMapperWidgetManager::setLimits(float l, float r){
  for(set<BlobMapperWidget*>::iterator it = blobWidgets.begin(); it != blobWidgets.end(); ++it){
    if((*it)->plotDistribution())
      (*it)->setPlotLimits(plotType, l, r);
  }
  emit newLimits();
}

void BlobMapperWidgetManager::plotDistributions(){
    vector<vector<float> > values(blobWidgets.size());
    vector<QColor> colors(blobWidgets.size());
    uint i=0;
    for(set<BlobMapperWidget*>::iterator it=blobWidgets.begin(); it != blobWidgets.end(); ++it){
	colors[i] = (*it)->color();
	set<blob*>& blobs = (*it)->blobs();
	values[i].reserve(blobs.size());
	cout << "BlobMapperWidget reserving size for " << i << "  : " << blobs.size() << endl;
	for(set<blob*>::iterator bit=blobs.begin(); bit != blobs.end(); ++bit){
	    switch(plotType){
		case BlobMapperWidget::VOLUME:
		    values[i].push_back( (*bit)->points.size() );
		    break;
		case BlobMapperWidget::SUM:
		    values[i].push_back( (*bit)->sum );
		    break;
		case BlobMapperWidget::MEAN:
		    values[i].push_back( (*bit)->sum / (*bit)->points.size() );
		    break;
		case BlobMapperWidget::MAX:
		    values[i].push_back( (*bit)->max );
		    break;
		case BlobMapperWidget::MIN:
		    values[i].push_back( (*bit)->min );
		    break;
		default:
		    values[i].push_back( (*bit)->sum );
	    }
	}
	++i;
    }
    distPlotter->setData(values, colors, true);
    distPlotter->show();
}


void BlobMapperWidgetManager::plotSuperDistributions(){
  cout << "plot super distributions superblobs.size() " << superBlobs.size() << endl;
  vector<QColor> widgetColors;
  map<unsigned int, bool> includeMap;
  if(!superBlobs.size() && !superDistPlotter->isVisible())
    return;

  ///////// STUPID HACK ///////////
  // since we have no way of showing more than one set of limits, we'll arbitrarily
  // show the last set of limits set.. 
  pl_limits plotLimits;

  unsigned int map_id = 1;
  for(set<BlobMapperWidget*>::iterator it=blobWidgets.begin(); it != blobWidgets.end(); ++it){
    widgetColors.push_back((*it)->color());
    includeMap[map_id] = (*it)->plotDistribution();
    map_id *= 2;
    if((*it)->plotDistribution() && (*it)->pLimits().count(plotType))
      plotLimits = (*it)->pLimits()[plotType];
  }
  
  map<unsigned int, vector<float> > values;
  for(uint i=0; i < superBlobs.size(); ++i){
    float v;
    for(uint j=0; j < superBlobs[i]->blobs.size(); ++j){
      if(!( includeMap[ superBlobs[i]->blobs[j].mapper_id ] ))
	continue;
      blob* b = superBlobs[i]->blobs[j].b;
      switch(plotType){
      case BlobMapperWidget::VOLUME:
	v = b->points.size();
	break;
      case BlobMapperWidget::SUM:
	v = b->sum;
	break;
      case BlobMapperWidget::MEAN:
	v = b->sum / (float)b->points.size();
	break;
      case BlobMapperWidget::MAX:
	v = b->max;
	break;
      case BlobMapperWidget::MIN:
	v = b->min;
	break;
      default:
	v = b->sum;
      }
      values[superBlobs[i]->membership].push_back(v);
    }
  }
  // then work out the colors that we'll need using  
  vector<QColor> plotColors;
  vector< vector<float> > plotValues;
  for(map<unsigned int, vector<float> >::iterator it=values.begin(); it != values.end(); ++it){
    int r, g, b;
    r = g = b = 0;
    unsigned int component = 1;
    for(uint i=0; i < widgetColors.size(); ++i){
      if((*it).first & component){
	r += widgetColors[i].red();
	g += widgetColors[i].green();
	b += widgetColors[i].blue();
      }
      component *= 2;
    }
    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;
    plotColors.push_back(QColor(r, g, b));
    plotValues.push_back((*it).second);
  }

  if(plotLimits.left < plotLimits.right){
    cout << "Found a previous set of limits " << plotLimits.left << "  " << plotLimits.right << endl;
    superDistPlotter->setPlotLimits(plotLimits.left, plotLimits.right, false);
    superDistPlotter->setData(plotValues, plotColors, false);
  }else{
    superDistPlotter->setData(plotValues, plotColors, true);
  }
  superDistPlotter->show();
}

void BlobMapperWidgetManager::deleteSuperBlobs(){
    for(uint i=0; i < superBlobs.size(); ++i)
	delete superBlobs[i];
    superBlobs.resize(0);
}
