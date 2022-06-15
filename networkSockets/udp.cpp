#include "udp.h"
#include <iostream>
#include <cstring>

UDP::UDP()
{
    int FPP = 128;
    int audioDataLen = FPP*2*2;
    mHeader.TimeStamp = (uint64_t)0;
    mHeader.SeqNumber = (uint16_t)0;
    mHeader.BufferSize = (uint16_t)FPP;
    mHeader.SamplingRate = (uint8_t)3;
    mHeader.BitResolution = (uint8_t)16;
    mHeader.NumIncomingChannelsFromNet = (uint8_t)2;
    mHeader.NumOutgoingChannelsToNet = (uint8_t)2;
    int packetDataLen = sizeof(HeaderStruct)+audioDataLen;
    mBuf.resize(packetDataLen);
    mBuf.fill(0,packetDataLen);
    memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));

    std::cout << "Default Packet Header:" << std::endl;
    std::cout << "Buffer Size               = " << static_cast<int>(mHeader.BufferSize)
         << std::endl;
    // Get the sample rate in Hz form the AudioInterface::samplingRateT
    int sample_rate = mHeader.SamplingRate;
    std::cout << "Sampling Rate               = " << sample_rate << "\n"
            "Audio Bit Resolutions       = " << static_cast<int>(mHeader.BitResolution) << "\n"
            "Number of Incoming Channels = " << static_cast<int>(mHeader.NumIncomingChannelsFromNet) << "\n"
            "Number of Outgoing Channels = " << static_cast<int>(mHeader.NumOutgoingChannelsToNet) << "\n"
            "Sequence Number             = " << static_cast<int>(mHeader.SeqNumber) << "\n"
            "Time Stamp                  = " << mHeader.TimeStamp << "\n"
            << "\n" << "\n";

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
    mPeerPort = 61002;
    socket.bind(mPeerAddr, mPeerPort);
    std::cout << "UDP send: start" << std::endl;
    while (true) {
        socket.writeDatagram(mBuf, mPeerAddr, mPeerPort);
        msleep(3);
    }
}

void UDP::sendPacket()
{
}
