#include <QtGui>
#include <QApplication>

#include "MKVSegmentJoiner.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MKVSegmentJoiner w;
  a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
  a.connect(&a, SIGNAL(lastWindowClosed()), &w, SLOT(close()));
  w.show();
  return a.exec();
}
