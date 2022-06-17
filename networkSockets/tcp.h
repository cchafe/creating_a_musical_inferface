#ifndef TCP_H
#define TCP_H

#include <QTcpSocket>

class TCP
{
public:
    TCP();
    QTcpSocket * mSocket;
    void connectToHost();
    void sendToHost();
    void bytesWritten(qint64 bytes);
    void readyRead();
};

#endif // TCP_H
