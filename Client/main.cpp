#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QAbstractSocket::SocketError>();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
