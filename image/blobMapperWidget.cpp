#include "blobMapperWidget.h"
#include <iostream>
#include <QLayout>
#include <QPalette>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QPixmap>
#include <QPainter>
#include <sstream>

using namespace std;

BlobMapperWidget::BlobMapperWidget(BlobMapper* bmapper, fluorInfo& fInfo, string fname, QWidget* parent)
    : QWidget(parent)
{
    mapper = bmapper;
    fluor = fInfo;
    fileName = fname;

    // make all static tool buttons with icons before calling makeIcons
    eatNeighborButton = new QToolButton(this);
    eatNeighborButton->setToolTip("Eat Neighbours");
    connect(eatNeighborButton, SIGNAL(clicked()), this, SLOT(eatNeighbors()));
    
    QToolButton* exportButton = new QToolButton(this);
    exportButton->setText("E");
    exportButton->setToolTip("Export Blobs");
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportBlobs()) );

    makeIcons();  // this will set the icon for the eatNeighbor button

    

    currentColor = QColor(255, 0, 255);
    QPalette palette(currentColor);
    colorButton = new QToolButton(this);
    colorButton->setPalette(palette);
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setColor()) );

    currentRep = 1; // 0, 1, 2 should refer to RepIcon::NO_REP, RepIcon::OUTLINE, RepIcon::FILLED. This is set in the makeIcons function.
    drawTypeButton = new QToolButton(this);
    drawTypeButton->setIcon(icons[currentRep].icon);
    connect(drawTypeButton, SIGNAL(clicked()), this, SLOT(changeRep()));

    QString labString;
    labString.setNum(fluor.excitation);
    QLabel* exciteLabel = new QLabel(labString, this);
    labString.setNum(fluor.emission);
    QLabel* emitLabel = new QLabel(labString, this);
    labString.setNum(mapper->minimum());
    QLabel* minLabel = new QLabel(labString, this);
    
    QPushButton* delButton = new QPushButton("del", this);
    delButton->setMaximumWidth(40);
    connect(delButton, SIGNAL(clicked()), this, SIGNAL(deleteMe()));
    // not connected yet..
    
    QHBoxLayout* hbox = new QHBoxLayout(this);
    hbox->setMargin(2);
    hbox->setSpacing(1);
    hbox->addWidget(delButton);
    hbox->addWidget(eatNeighborButton);
    hbox->addWidget(exportButton);
    hbox->addStretch();
    hbox->addWidget(exciteLabel);
    hbox->addSpacing(3);
    hbox->addWidget(emitLabel);
    hbox->addWidget(minLabel);
    hbox->addStretch();
    hbox->addWidget(colorButton);
    hbox->addWidget(drawTypeButton);
}

BlobMapperWidget::~BlobMapperWidget(){
    delete mapper;
}

set<blob*>& BlobMapperWidget::blobs(){
    return(mapper->gBlobs());
}

BlobMapper* BlobMapperWidget::blobMapper(){
    return(mapper);
}

RepIcon::BlobRepresentation BlobMapperWidget::blobRep(){
    return( icons[currentRep].rep);
}

void BlobMapperWidget::color(float& r, float& g, float& b){
    r = (float)currentColor.red() / 255.0;
    g = (float)currentColor.green() / 255.0;
    b = (float)currentColor.blue() / 255.0;
}

QColor BlobMapperWidget::color(){
    return(currentColor);
}

void BlobMapperWidget::exportBlobs(){
    ostringstream fname;
    fname << fileName << "_" << fluor.excitation << "-" << fluor.emission << "_" << mapper->minimum() << ".blobs";
    cout << "Calling exportBlobs with file name : " << fname.str() << endl;
    mapper->exportBlobs(fname.str());
}

void BlobMapperWidget::setColor(){
    currentColor = QColorDialog::getColor(currentColor);
    QPalette palette(currentColor);
    //palette.setColor(QPalette::Button, currentColor);
    colorButton->setPalette(palette);
    emit newColor();
}

void BlobMapperWidget::changeRep(){
    currentRep = (++currentRep) % icons.size();
    drawTypeButton->setIcon(icons[currentRep].icon);
    cout << "Changing representation to " << currentRep << " : " << icons[currentRep].rep << endl;
    emit newRep();
}

void BlobMapperWidget::eatNeighbors(){
    mapper->eatNeighbors();
}

void BlobMapperWidget::makeIcons(){
    int w, h;
    w = h = 32;
    int m = 4;

    QPixmap pix(w, h);
    QPainter p(&pix);
    QColor bg(QColor(0, 0, 0));
    QColor fg(QColor(255, 255, 255));

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(fg, 3));

    // first draw the representation for an empty one.
    pix.fill(bg);
    icons.push_back(RepIcon( QIcon(pix), RepIcon::NO_REP) );

    p.drawEllipse(m-1, m-1, w-2*m, h-2*m);
    icons.push_back(RepIcon( QIcon(pix), RepIcon::OUTLINE) );

    p.setBrush(fg);
    p.drawEllipse(m-1, m-1, w-2*m, h-2*m);
    icons.push_back(RepIcon( QIcon(pix), RepIcon::FILLED) );

    // And draw an icon for eating neighbors
    pix.fill(bg);
    p.drawPie(m-1, m-1, w-2*m, w-2*m, 45*16, 270*16);
    p.drawPie(m, m-1, w-2*m, w-2*m, 15*16, -30*16);
    eatNeighborButton->setIcon(QIcon(pix));
}
   
