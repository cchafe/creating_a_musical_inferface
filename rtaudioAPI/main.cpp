#include <QCoreApplication>
#include "api.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
API api;
api.test_cpp();

//    return a.exec();
}
