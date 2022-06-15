#include "tcp.h"
#include <iostream>
#include <QtEndian>
#include <QHostInfo>

TCP::TCP()
{
    mSocket = new QTcpSocket();
}

void TCP::connectToHost()
{
    QHostAddress serverHostAddress;
//    QString server("cmn9.stanford.edu");
    QString server("171.64.197.158");
    if (!serverHostAddress.setAddress(server)) {
        QHostInfo info = QHostInfo::fromName(server);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
    mSocket->connectToHost(serverHostAddress,4464);
    mSocket->waitForConnected(500);
}

void TCP::sendToHost()
{
    if (mSocket->state()==QTcpSocket::ConnectedState) {
        QString request("4464");
        QByteArray ba;
        ba.append(request);
        ba.append('\n');
        mSocket->write(ba);
        mSocket->waitForBytesWritten(1000);
        //    fprintf(stderr,"wrote :%s\n",request.toLocal8Bit().data());
        readyRead();
    } else
        std::cout << "TCP: tried to send data but not connected to server" << std::endl;

}

void TCP::readyRead()
{
    mSocket->waitForReadyRead();
    quint16 bytes =  mSocket->bytesAvailable();
    std::cout << "TCP: from server " << bytes << std::endl;

    uint32_t udp_port;
    int size       = sizeof(udp_port);
    char* port_buf = new char[size];
    mSocket->read(port_buf, size);
    udp_port = qFromLittleEndian<qint32>(port_buf);

    std::cout << "TCP: ephemeral port = " << udp_port << std::endl;

    //    ui->outputTextEdit->append("----------------------------------------");
    //    ui->outputTextEdit->append(QString::number(bytes)+ " for you to read");
    //    ui->outputTextEdit->append("----------------------------------------");
    //    QByteArray ba= socket->readAll();
    //    QString dataString (ba);
    //    QString pos("<source id='5'><position");
    //    if (dataString.contains(pos)) {
    //        ui->outputTextEdit->append(dataString);
    //        QString request0("<request><source id='2'><position x='0.0' y='0.0'/></source></request>" );
    //        QByteArray ba;
    //        ba.append(request0);
    //        ba.append('\0');
    //        socket->write(ba);
    //    }
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
    QString errorStr=mSocket->errorString();
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
