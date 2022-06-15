#include "tcp.h"

TCP::TCP()
{
    socket = new QTcpSocket();
}

void TCP::connectToHost()
{
    socket->connectToHost(QString("127.0.0.1"),5000);
}

void TCP::sendToHost()
{
    QString request("hi");
    QByteArray ba;
    ba.append(request);
    ba.append('\n');
    socket->write(ba);
    socket->waitForBytesWritten(1000);
    //    fprintf(stderr,"wrote :%s\n",request.toLocal8Bit().data());
}

void TCP::readyRead()
{
    quint16 bytes =  socket->bytesAvailable();
}


void TCP::connected()
{
    fprintf(stderr,"Connected to server\n");
}

void TCP::disconnected()
{
    fprintf(stderr,"Disconnected from server\n");
}

void TCP::error(QAbstractSocket::SocketError socketError)
{
    QString errorStr=socket->errorString();
    fprintf(stderr,"An error occured :%s\n",errorStr.toLocal8Bit().data());
}

void TCP::hostFound()
{
    fprintf(stderr,"Host found\n");
}

void TCP::bytesWritten(qint64 bytes)
{
    QString outString = QString::number(bytes) + " bytes writen.";
    fprintf(stderr,"%s\n",outString.toLocal8Bit().data());

}
