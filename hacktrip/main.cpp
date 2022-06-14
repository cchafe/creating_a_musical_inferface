#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv); // no command line arguments
  MainWindow w;               // instantiate the gui
  w.show();                   // show it
  return a.exec();            // run the event loop
  // exit when done
}
