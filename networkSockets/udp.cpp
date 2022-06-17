#include "udp.h"
#include <iostream>
#include <cstring>
#include "globals.h"

UDP::UDP()
{
    int audioDataLen = gFPP*2*2;
    mHeader.TimeStamp = (uint64_t)0;
    mHeader.SeqNumber = (uint16_t)0;
    mHeader.BufferSize = (uint16_t)gFPP;
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
stop();
}

void UDP::run() {
    // socket needs to be created here in this thread
    QUdpSocket socket;
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(gServer)) {
        QHostInfo info = QHostInfo::fromName(gServer);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
    mPeerPort = 61002;
    socket.bind(serverHostAddress, mPeerPort);
    std::cout << "UDP send: start" << std::endl;
    int seq = 0;
//    int audioDataLen = gFPP*2*2;
//    int packetDataLen = sizeof(HeaderStruct)+audioDataLen;
    mStream = true;
    while (mStream) {
        seq++;
        seq %= 65536;
        mHeader.SeqNumber = (uint16_t)seq;
        memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));
        socket.writeDatagram(mBuf, serverHostAddress, mPeerPort);
        msleep(5);
    }
    // Send exit packet (with 1 redundant packet).
    int controlPacketSize = 63;
    std::cout << "sending exit packet" << std::endl;
    mBuf.resize(controlPacketSize);
    mBuf.fill(0xff,controlPacketSize);
    socket.writeDatagram(mBuf, serverHostAddress, mPeerPort);
    socket.writeDatagram(mBuf, serverHostAddress, mPeerPort);
}

void UDP::stop()
{
    mStream = false;
}
