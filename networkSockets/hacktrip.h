#ifndef HACKTRIP_H
#define HACKTRIP_H
#include "tcp.h"
#include "udp.h"
#include "../hacktrip/duplex.h"

class HackTrip
{
public:
    HackTrip();
    void setup();
    void start();
    void stop();
    void udpRcvPacket();
private:
    TCP mTcpClient;
    UDP mUdpSend;
    UDP mUdpRcv;
    Duplex *audio;
};

#endif // HACKTRIP_H
