#include "hacktrip.h"
#include <QtEndian>
#include <QUdpSocket>
#include <netinet/in.h>
#include <sys/types.h>

HackTrip::HackTrip()
{
    mUdp = new UDP();
    //    mAudio = new Audio(mUdp);
    mTcp = new TCP(mUdp);
}

void HackTrip::connect()
{
    mTcp->connectToServer();
}

TCP::TCP(UDP *udp)
    : mUdp(udp)
{};

void TCP::connectToServer()
{
    mSocket = new QTcpSocket();
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(gServer)) {
        QHostInfo info = QHostInfo::fromName(gServer);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
    mSocket->connectToHost(serverHostAddress,4464);
    mSocket->waitForConnected(HackTrip::mSocketWaitMs);
    sendToServer();
}

void TCP::sendToServer()
{
    char* port_buf = new char[sizeof(u_int32_t)];
    if (mSocket->state()==QTcpSocket::ConnectedState) {
        QByteArray ba;
        qint32 tmp = 4464;
        ba.setNum(tmp);
        mSocket->write(ba);
        mSocket->waitForBytesWritten(1500);
        mSocket->waitForReadyRead();
        mSocket->read(port_buf, sizeof(u_int32_t));
        mUdp->mPeerPort = qFromLittleEndian<qint32>(port_buf);
        std::cout << "TCP: ephemeral port = " << mUdp->mPeerPort << std::endl;
    } else
        std::cout << "TCP: not connected to server" << std::endl;
    delete[] port_buf;
}

TCP:: ~TCP() {
    delete mSocket;
}

void HackTrip::run()
{
    mUdp->start();
    QByteArray startBuf;
    startBuf.resize(mAudioDataLen);
    startBuf.fill(0xff,mAudioDataLen);
    for (int i = 0; i<25; i++) { // needs more than 5
        QThread::msleep(5); // needs spacing
        mUdp->send(i,(int8_t *)&startBuf);
    }
    //    mAudio->start();
}

void HackTrip::stop()
{
    //    mUdp->quit = true;
    //    QThread::msleep(100);
    mUdp->stop();
    //    QThread::msleep(100);
    //    mAudio->stop();
    //    QThread::msleep(100);
}

HackTrip:: ~HackTrip() {
    //    delete mAudio;
    //    delete mUdp;
}

UDP::UDP() {
    mHeader.TimeStamp = (uint64_t)0;
    mHeader.SeqNumber = (uint16_t)0;
    mHeader.BufferSize = (uint16_t)HackTrip::mFPP;
    mHeader.SamplingRate = (uint8_t)3;
    mHeader.BitResolution = (uint8_t)sizeof(MY_TYPE) * 8; // checked in jacktrip
    mHeader.NumIncomingChannelsFromNet = HackTrip::mChannels;
    mHeader.NumOutgoingChannelsToNet = HackTrip::mChannels;
    int packetDataLen = sizeof(HeaderStruct) + HackTrip::mAudioDataLen;
    mBuf.resize(packetDataLen);
    mBuf.fill(0,packetDataLen);
    memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));
    if (!serverHostAddress.setAddress(gServer)) {
        QHostInfo info = QHostInfo::fromName(gServer);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }
}

void UDP::start() {
    quit = false;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
               (void *) &optval, sizeof(optval));
    setSocketDescriptor(sockfd, QUdpSocket::UnconnectedState);
    int ret = 0;
    ret = bind(HackTrip::mAudioPort);
    std::cout << "UDP: start send = " << ret << " " << serverHostAddress.toString().toLocal8Bit().data() <<  std::endl;
    //    connect(this, &QUdpSocket::readyRead, this, &UDP::readPendingDatagrams);
    //    mRing = 500;
    //    mWptr = 0;
    //    mRptr = 0;
    //    for (int i = 0; i < mRing; i++) {
    //        int8_t* tmp = new int8_t[HackTrip::mAudioDataLen];
    //        for (int j = 0; j < HackTrip::mAudioDataLen; j++)
    //            tmp[j] = 0;
    //        mInBuffer.push_back(tmp);
    //    }
};

UDP:: ~UDP() {
    std::cout << "2\n";
}

//https://stackoverflow.com/questions/40200535/c-qt-qudp-socket-not-sending-receiving-data

void UDP::readPendingDatagrams() {
    //read datagrams in a loop to make sure that all received datagrams are processed
    //since readyRead() is emitted for a datagram only when all previous datagrams are read
    if (quit) return;
    QMutexLocker locker(&mMutex);
    while(hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        readDatagram(datagram.data(), datagram.size(),
                     &sender, &senderPort);
        memcpy(&mHeader,mBuf.data(),sizeof(HeaderStruct));
        int seq = mHeader.SeqNumber;
        if (seq%500 == 0)
            std::cout << "UDP rcv: seq = " << seq << std::endl;
        int8_t *audioBuf = (int8_t *)(mBuf.data() + sizeof(HeaderStruct));
        //        mRegFromHackTrip->nonILtoIL((int8_t *)audioBuf);
        memcpy(mInBuffer[mWptr],audioBuf,mBuf.size());
        mWptr++;
        mWptr %= mRing;
    }
}

void UDP::send(int seq, int8_t *audioBuf) {
    mHeader.SeqNumber = (uint16_t)seq;
    memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));

    //    mRegFromHackTrip->ILtoNonIL((int8_t *)audioBuf);

    memcpy(mBuf.data()+sizeof(HeaderStruct),audioBuf,HackTrip::mAudioDataLen);
    writeDatagram(mBuf, serverHostAddress, mPeerPort);
    if (seq%500 == 0)
        std::cout << "UDP send: packet = " << seq << std::endl;
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
    writeDatagram(stopBuf, serverHostAddress, mPeerPort);
    writeDatagram(stopBuf, serverHostAddress, mPeerPort);
    std::cout << "-2\n";
    close(); // stop rcv
    for (int i = 0; i < mRing; i++) {
        delete[] mInBuffer[i];
    }
}

Audio::Audio(UDP *udp)
    : mUdp(udp)
{
    m_adac = 0;
    seq = 0;
};

Audio:: ~Audio() {
    std::cout << "0\n";
    delete m_adac;
}

int Audio::audio_callback( void *outputBuffer, void *inputBuffer,
                           unsigned int nBufferFrames,
                           double streamTime, RtAudioStreamStatus /* status */,
                           void * /* data */ ) // last arg is used for "this"
{
    if(mUdp->quit) {
        std::cout << " quit!!! \n";
        return 1;
    }
    if ( streamTime >= m_streamTimePrintTime ) {
        std::cout << " networkAudio_callback" << " nBufferFrames " << nBufferFrames << " streamTime " << streamTime << std::endl;
        m_streamTimePrintTime += m_streamTimePrintIncrement;
    }
    seq++;
    seq %= 65536;
    if (seq%500 == 0)
        std::cout << "packet to network = " << seq << std::endl;
    mUdp->send(seq,(int8_t *)inputBuffer);

    QMutexLocker locker(&mUdp->mMutex);
    if(mUdp->mRptr == mUdp->mWptr) mUdp->mRptr = mUdp->mWptr-10;
    if (mUdp->mRptr < 0) mUdp->mRptr = 0;
    mUdp->mRptr %= mUdp->mRing;
    memcpy(outputBuffer, mUdp->mInBuffer[mUdp->mRptr],
            HackTrip::mAudioDataLen);
    mUdp->mRptr++;
    return 0;
}

int Audio::wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                  unsigned int nBufferFrames, double streamTime,
                                  RtAudioStreamStatus status, void *arg)
{
    return static_cast<Audio*>(arg)->audio_callback(
                outputBuffer, inputBuffer, nBufferFrames, streamTime, status, arg);
}

void Audio::start() {
    m_streamTimePrintIncrement = 1.0; // seconds
    m_streamTimePrintTime = 1.0; // seconds
    m_adac = new RtAudio();
    if (m_adac->getDeviceCount() < 1) {
        std::cout << "\nNo audio devices found!\n";
        exit(1);
    }
    m_channels = HackTrip::mChannels;
    m_fs = HackTrip::mSampleRate;
    m_iDevice = m_oDevice = 0;
    m_iOffset = m_oOffset = 0; // first channel
    // copy all setup into all stream info
    m_iParams.deviceId = m_iDevice;
    m_iParams.nChannels = m_channels;
    m_iParams.firstChannel = m_iOffset;
    m_oParams.deviceId = m_oDevice;
    m_oParams.nChannels = m_channels;
    m_oParams.firstChannel = m_oOffset;
    options.flags = RTAUDIO_SCHEDULE_REALTIME; // use realtime priority if it's available
    options.numberOfBuffers = HackTrip::mNumberOfBuffersRtAudio; // Windows DirectSound, Linux OSS, and Linux Alsa APIs only.
    // value set by the user is replaced during execution of the RtAudio::openStream() function by the value actually used by the system
    std::cout << "using default audio interface device\n";
    std::cout << m_adac->getDeviceInfo(m_iDevice).name
              << "\tfor input and output\n";
    std::cout << "\tIf another is needed, either change your settings\n";
    std::cout << "\tor the choice in the code\n";
    m_adac->showWarnings(true);
    bufferFrames = HackTrip::mFPP;
    bufferBytes = HackTrip::mFPP*HackTrip::mChannels*sizeof(MY_TYPE);
    if (m_adac->openStream( &m_oParams, &m_iParams, FORMAT, HackTrip::mSampleRate,
                            &bufferFrames, &Audio::wrapperProcessCallback,
                            (void*)this,  &options ))
        std::cout << "\nCouldn't open audio device streams!\n";
    if (m_adac->isStreamOpen() == false) {
        std::cout << "\nCouldn't open audio device streams!\n";
        exit(1);
    } else {
        std::cout << "\trunning " <<
                     m_adac->getApiDisplayName(m_adac->getCurrentApi()) << "\n";
        std::cout << "\nStream latency = " << m_adac->getStreamLatency()
                  << " frames" << std::endl;
    }
    std::cout << "\nAudio stream start" << std::endl;
    if (m_adac->startStream())
        std::cout << "\nCouldn't start streams!\n";
}

void Audio::stop()
{
    if (m_adac)
        if (m_adac->isStreamRunning()) {
            std::cout << "\nAudio stream stop" << std::endl;
            m_adac->stopStream();
            if ( m_adac->isStreamOpen() ) {
                m_adac->closeStream();
                std::cout << "Audio stream closed" << std::endl;
            }
        }
    std::cout << "-1\n";
}
