#include "blobMapperWidgetManager.h"
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>

using namespace std;

BlobMapperWidgetManager::BlobMapperWidgetManager(QWidget* parent)
    : QWidget(parent)
{

    QComboBox* paramChooser = new QComboBox(false, this);
    paramChooser->insertItem(VOLUME, "Volume");
    paramChooser->insertItem(SUM, "Sum");
    paramChooser->insertItem(MEAN, "Mean");
    paramChooser->insertItem(MAX, "Max");
    paramChooser->insertItem(MIN, "Min");
    
    connect(paramChooser, SIGNAL(activated(int)), this, SLOT(setParamType(int)) );
    plotType = VOLUME;

    QPushButton* superButton = new QPushButton("Super", this);
    connect(superButton, SIGNAL(clicked()), this, SLOT(makeSuperBlobs()) );

    distPlotter = new DistPlotter();

    QVBoxLayout* mainBox = new QVBoxLayout(this);
    mainBox->setSpacing(3);
    mainBox->setMargin(1);
    
    vbox = new QVBoxLayout();
    mainBox->addLayout(vbox);

    QHBoxLayout* hbox = new QHBoxLayout();
    mainBox->addLayout(hbox);
    hbox->addStretch();
    hbox->addWidget(superButton);
    hbox->addWidget(paramChooser);
}

BlobMapperWidgetManager::~BlobMapperWidgetManager(){
    // we should delete the blobMappers .. 
}

void BlobMapperWidgetManager::addBlobMapper(BlobMapper* bm, fluorInfo& fInfo, string fName){
    BlobMapperWidget* bmw = new BlobMapperWidget(bm, fInfo, fName, this);
    connect(bmw, SIGNAL(newColor()), this, SIGNAL(newColor()) );
    connect(bmw, SIGNAL(newRep()), this, SIGNAL(newRep()) );
    connect(bmw, SIGNAL(deleteMe()), this, SLOT(deleteBlobWidget()) );

    blobWidgets.insert(bmw);
    vbox->addWidget(bmw);
    bmw->show();

    distPlotter->setCaption(fName.c_str());
    plotDistributions();
}

set<BlobMapperWidget*> BlobMapperWidgetManager::blobMapperWidgets(){
    return(blobWidgets);
}

void BlobMapperWidgetManager::setParamType(int p){
    // This is kind of ugly, but I don't know a better way
    switch(p){
	case VOLUME:
	    plotType = VOLUME;
	    break;
	case SUM:
	    plotType = SUM;
	    break;
	case MEAN:
	    plotType = MEAN;
	    break;
	case MAX:
	    plotType = MAX;
	    break;
	case MIN:
	    plotType = MIN;
	    break;
	default:
	    plotType = SUM;
    }
    plotDistributions();
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
		case VOLUME:
		    values[i].push_back( (*bit)->points.size() );
		    break;
		case SUM:
		    values[i].push_back( (*bit)->sum );
		    break;
		case MEAN:
		    values[i].push_back( (*bit)->sum / (*bit)->points.size() );
		    break;
		case MAX:
		    values[i].push_back( (*bit)->max );
		    break;
		case MIN:
		    values[i].push_back( (*bit)->min );
		    break;
		default:
		    values[i].push_back( (*bit)->sum );
	    }
	}
	++i;
    }
    distPlotter->setData(values, colors);
    distPlotter->show();
}

void BlobMapperWidgetManager::deleteSuperBlobs(){
    for(uint i=0; i < superBlobs.size(); ++i)
	delete superBlobs[i];
    superBlobs.resize(0);
}
