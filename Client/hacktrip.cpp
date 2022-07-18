#include "hacktrip.h"
#include <QtEndian>
#include <QUdpSocket>
#include <netinet/in.h>
#include <sys/types.h>
#include <math.h>

void HackTrip::connect()
{
    mUdp.setPeerUdpPort( mTcp.connectToServer() );
}

int TCP::connectToServer()
{
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(gServer)) {
        QHostInfo info = QHostInfo::fromName(gServer);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
    connectToHost(serverHostAddress,HackTrip::mServerTcpPort);
    waitForConnected(HackTrip::mSocketWaitMs);
    int peerUdpPort = 0;
    char* port_buf = new char[sizeof(u_int32_t)];
    if (state()==QTcpSocket::ConnectedState) {
        QByteArray ba;
        qint32 tmp = HackTrip::mLocalAudioUdpPort;
        ba.setNum(tmp);
        write(ba);
        waitForBytesWritten(1500);
        waitForReadyRead();
        read(port_buf, sizeof(u_int32_t));
        peerUdpPort = qFromLittleEndian<qint32>(port_buf);
        std::cout << "TCP: ephemeral port = " << peerUdpPort << std::endl;
    } else
        std::cout << "TCP: not connected to server" << std::endl;
    delete[] port_buf;
    return peerUdpPort;
}

void HackTrip::run()
{
    mUdp.start();
//    QByteArray startBuf;
//    startBuf.resize(mAudioDataLen);
//    startBuf.fill(0xff,mAudioDataLen);
//    for (int i = 0; i<25000; i++) { // needs more than 5
//        QThread::msleep(5); // needs spacing
//        mUdp.send(i,(int8_t *)&startBuf);
//    }
}

void HackTrip::stop()
{
    mUdp.stop();
}

void UDP::start() {
    mHeader.TimeStamp = (uint64_t)0;
    mHeader.SeqNumber = (uint16_t)0;
    mHeader.BufferSize = (uint16_t)HackTrip::mFPP;
    mHeader.SamplingRate = (uint8_t)3;
    mHeader.BitResolution = (uint8_t)sizeof(MY_TYPE) * 8; // checked in jacktrip
    mHeader.NumIncomingChannelsFromNet = HackTrip::mChannels;
    mHeader.NumOutgoingChannelsToNet = HackTrip::mChannels;
    int packetDataLen = sizeof(HeaderStruct) + HackTrip::mAudioDataLen;
    mBufSend.resize(packetDataLen);
    mBufSend.fill(0,packetDataLen);
    memcpy(mBufSend.data(),&mHeader,sizeof(HeaderStruct));
    mBufRcv.resize(packetDataLen);
    mBufRcv.fill(0,packetDataLen);
    memcpy(mBufRcv.data(),&mHeader,sizeof(HeaderStruct));

    if (!serverHostAddress.setAddress(gServer)) {
        QHostInfo info = QHostInfo::fromName(gServer);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
    //    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    //    int optval = 1;
    //    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
    //               (void *) &optval, sizeof(optval));
    //    setSocketDescriptor(sockfd, QUdpSocket::UnconnectedState);
    int ret = 0;
    ret = bind(HackTrip::mLocalAudioUdpPort);
    std::cout << "UDP: start send = " << ret << " " << serverHostAddress.toString().toLocal8Bit().data() <<  std::endl;
    connect(this, &QUdpSocket::readyRead, this, &UDP::readPendingDatagrams);
    mRing = 50;
    mWptr = mRing / 2;
    mRptr = 0;
    for (int i = 0; i < mRing; i++) {
        int8_t* tmp = new int8_t[HackTrip::mAudioDataLen];
        for (int j = 0; j < HackTrip::mAudioDataLen; j++)
            tmp[j] = 0;
        mRingBuffer.push_back(tmp);
    }
    mSendSeq = 0;
};

//https://stackoverflow.com/questions/40200535/c-qt-qudp-socket-not-sending-receiving-data

void UDP::readPendingDatagrams() {
    //read datagrams in a loop to make sure that all received datagrams are processed
    //since readyRead() is emitted for a datagram only when all previous datagrams are read
    //    QMutexLocker locker(&mMutex);
    while(hasPendingDatagrams()) {
        QHostAddress sender;
        quint16 senderPort;
        readDatagram(mBufRcv.data(), mBufRcv.size(),
                     &sender, &senderPort);
        //        std::cout << sender.toIPv4Address() << " " << senderPort << std::endl;
        memcpy(&mHeader,mBufRcv.data(),sizeof(HeaderStruct));
        int rcvSeq = mHeader.SeqNumber;
        if (rcvSeq%500 == 0)
            std::cout << "UDP rcv: seq = " << rcvSeq << std::endl;
        int8_t *audioBuf = (int8_t *)(mBufRcv.data() + sizeof(HeaderStruct));
        //        Audio::printSamples((MY_TYPE *)audioBuf);
        memcpy(mRingBuffer[mWptr],audioBuf,HackTrip::mAudioDataLen);
        mWptr++;
        mWptr %= mRing;
    }
}

void UDP::sendDummyData()
{
    QByteArray fakeAudioBuf;
    fakeAudioBuf.resize(HackTrip::mAudioDataLen);
    fakeAudioBuf.fill(0xff,HackTrip::mAudioDataLen);
    send( (int8_t *)&fakeAudioBuf );
}

void UDP::send(int8_t *audioBuf) {
    mHeader.SeqNumber = (uint16_t)mSendSeq;
    memcpy(mBufSend.data(),&mHeader,sizeof(HeaderStruct));
    memcpy(mBufSend.data()+sizeof(HeaderStruct),audioBuf,HackTrip::mAudioDataLen);
    writeDatagram(mBufSend, serverHostAddress, mPeerUdpPort);
    if (mSendSeq%500 == 0)
        std::cout << "UDP send: packet = " << mSendSeq << std::endl;
    mSendSeq++;
    mSendSeq %= 65536;
}

void UDP::stop()
{
    disconnect(this, &QUdpSocket::readyRead, this, &UDP::readPendingDatagrams);
    // Send exit packet (with 1 redundant packet).
    int controlPacketSize = 63;
    std::cout << "sending exit packet" << std::endl;
    QByteArray stopBuf;
    stopBuf.resize(controlPacketSize);
    stopBuf.fill(0xff,controlPacketSize);
    writeDatagram(stopBuf, serverHostAddress, mPeerUdpPort);
    writeDatagram(stopBuf, serverHostAddress, mPeerUdpPort);
    close(); // stop rcv
}

