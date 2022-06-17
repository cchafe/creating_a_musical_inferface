#ifndef TCP_H
#define TCP_H

#include <QTcpSocket>

class TCP
{
public:
    TCP();
    QTcpSocket * mSocket;
    void connectToHost();
    void hostFound();
    void connected();
    void sendToHost();
    void error(QAbstractSocket::SocketError socketError);
    void bytesWritten(qint64 bytes);
    void readyRead();
};

#endif // TCP_H
