#ifndef CLINEEDIT_H
#define CLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>

class CLineEdit : public QLineEdit
{
  Q_OBJECT
    
    public:
  CLineEdit(QWidget* parent=0)
    : QLineEdit(parent) {}

    signals :
  void arrowPressed(bool);
  
 private:
  void keyPressEvent(QKeyEvent* qke){
    if( qke->key()==Qt::Key_Up )
      { qke->accept(); emit arrowPressed( true ); }
    else if (qke->key()==Qt::Key_Down)
      { qke->accept(); emit arrowPressed( false ); }
    else QLineEdit::keyPressEvent( qke );
  }
};

#endif
