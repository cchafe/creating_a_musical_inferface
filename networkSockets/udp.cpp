#include "udp.h"
#include <iostream>
#include <cstring>

UDP::UDP()
{
}

void UDP::run() {
    // socket needs to be created here in this thread
    QUdpSocket socket;
    QHostAddress mPeerAddr;
//    QString server("cmn9.stanford.edu");
    QString server("171.64.197.158");
    if (!mPeerAddr.setAddress(server)) {
        QHostInfo info = QHostInfo::fromName(server);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            mPeerAddr = info.addresses().constFirst();
        }
    }
   socket.bind(mPeerAddr, 61002);
   int yyy = 128*2*2;
   buf.resize(sizeof(HeaderStruct)+yyy);
   buf.fill(0,sizeof(HeaderStruct)+yyy);
   std::memcpy(buf.data(), &mHeader, sizeof(HeaderStruct));
    std::cout << "UDP send: start" << std::endl;
    socket.writeDatagram(buf, mPeerAddr, mPeerPort);
}

void UDP::sendPacket()
{
}
