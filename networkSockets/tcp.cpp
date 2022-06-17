#include "tcp.h"
#include <iostream>
#include <QtEndian>
#include <QHostInfo>
#include "globals.h"
TCP::TCP()
{
    mSocket = new QTcpSocket();
}

void TCP::connectToHost()
{
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(gServer)) {
        QHostInfo info = QHostInfo::fromName(gServer);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
    mSocket->connectToHost(serverHostAddress,gPort.toInt());
    mSocket->waitForConnected(2500);
}

void TCP::sendToHost()
{
    if (mSocket->state()==QTcpSocket::ConnectedState) {
        QByteArray ba;
        ba.append(gPort);
        ba.append('\n');
        mSocket->write(ba);
        mSocket->waitForBytesWritten(gSocketWaitMs);
        std::cout << "TCP: waitForBytesWritten" << std::endl;
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

