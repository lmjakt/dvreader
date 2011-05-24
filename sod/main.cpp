#include <QApplication>
#include "SodController.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  SodController controller;
  app.setMainWidget(&controller);
  controller.show();
  return app.exec();
}
