#include "udp.h"
#include <iostream>
#include <cstring>
#include "globals.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
UDP::UDP()
{
    mRcv = false;
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
    mStop = false;
}

void UDP::setRcv() {
    mRcv = true;
}

void UDP::run() {
    // socket needs to be created here in this thread
    QUdpSocket sock;
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(gServer)) {
        QHostInfo info = QHostInfo::fromName(gServer);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
    mPeerPort = 61002;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
               (void *) &optval, sizeof(optval));
    sock.setSocketDescriptor(sockfd, QUdpSocket::UnconnectedState);
    int ret = 0;
    if (mRcv)  {
        ret = sock.bind(QHostAddress::Any,gAudioPort);
        std::cout << "UDP rcv: start = " << ret << std::endl;
    } else { // sender
        ret = sock.bind(gAudioPort);
        std::cout << "UDP send: start = " << ret << " " << serverHostAddress.toString().toLocal8Bit().data() <<  std::endl;
    }
    msleep(100);
    int seq = 0;
    //    int audioDataLen = gFPP*2*2;
    //    int packetDataLen = sizeof(HeaderStruct)+audioDataLen;
    while (!mStop) {
        if (mRcv) {
            //            std::cout << "UDP check incoming " << sock.pendingDatagramSize() << std::endl;
            if (sock.hasPendingDatagrams()) {
                //                std::cout << "UDP rcv: pending bytes = " << sock.pendingDatagramSize() << std::endl;
                int len = sock.readDatagram(mBuf.data(), mBuf.size());
                //                std::cout << "UDP rcv: bytes = " << len << std::endl;
                if (len != mBuf.size())
                    std::cout << "UDP rcv: not full packet (" << len << ") should be " << mBuf.size() << std::endl;
                else {
                    memcpy(&mHeader,mBuf.data(),sizeof(HeaderStruct));
                    seq = mHeader.SeqNumber;
                    std::cout << "UDP rcv: packet = " << seq << std::endl;
                    if (true) { // DSP block
                        MY_TYPE *inBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);                         MY_TYPE *outBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);
                        double tmp[gFPP * gChannels];
                        for (unsigned int i = 0; i < gFPP; i++) {
                            for (unsigned int j = 0; j < gChannels; j++) {
                                unsigned int index = i * gChannels + j;
                                tmp[index] = *inBuffer++ / SCALE;
                                if (tmp[index]>0.005)
                                    std::cout << "frame = " << seq*gFPP + i << "\t channel = " << j
                                          << "\t val = " << tmp[index] << std::endl;
                            }
                        }
                    }
                }
                msleep(1);
            }
        } else { // sender
            seq++;
            seq %= 65536;
            mHeader.SeqNumber = (uint16_t)seq;
            memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));
            if (true) { // DSP block
                //      demonstrates how to access incoming samples,
                //      print, change and set outgoing samples
                //                MY_TYPE *inBuffer = (MY_TYPE *)mBuf.data(); // sint
                MY_TYPE *outBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);
                double tmp[gFPP * gChannels];
                for (unsigned int i = 0; i < gFPP; i++) {
                    for (unsigned int j = 0; j < gChannels; j++) {
                        unsigned int index = i * gChannels + j;
                        //                    tmp[index] = *inBuffer++ / SCALE;
                        tmp[index] = ((i%30)==0) ? 0.0 : 0.0;
                         *outBuffer++ = (MY_TYPE)(tmp[index] * SCALE);
                    }
                }
            }
            sock.writeDatagram(mBuf, serverHostAddress, mPeerPort);
//            std::cout << "UDP send: packet = " << seq << std::endl;
            msleep(5);
        }
    }
    if (!mRcv) {
        // Send exit packet (with 1 redundant packet).
        int controlPacketSize = 63;
        std::cout << "sending exit packet" << std::endl;
        mBuf.resize(controlPacketSize);
        mBuf.fill(0xff,controlPacketSize);
        sock.writeDatagram(mBuf, serverHostAddress, mPeerPort);
        sock.writeDatagram(mBuf, serverHostAddress, mPeerPort);
    }
    sock.close();
}

void UDP::stop()
{
    mStop = true;
}
