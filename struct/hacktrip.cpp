#include "hacktrip.h"
#include <QtEndian>
#include <QUdpSocket>
#include <netinet/in.h>
#include <sys/types.h>

HackTrip::HackTrip()
{
    mReg = new Regulator(mChannels, sizeof(MY_TYPE),
                         mFPP, mBufferQueueLength);
    mUdp = new UDP(mReg);
    mAudio = new Audio(mReg, mUdp);
}

void HackTrip::start()
{
    mAudio->start();
}

void HackTrip::stop()
{
    mUdp->stop();
    mAudio->stop();
}

HackTrip:: ~HackTrip() {
    delete mAudio;
    delete mReg;
    delete mUdp;
}

Audio::Audio(Regulator * reg, UDP *udp)
    : mRegFromHackTrip(reg)
    , mUdp(udp)
{
    m_adac = 0;
    mPhasor.resize(2, 0.0); //HackTrip::mChannels
    seq = 0;
};

Audio:: ~Audio() {
}

void HackTrip::contactServer()
{
    mTcpClient.connectToHost();
    mTcpClient.sendToHost();
}


int Audio::networkAudio_callback( void *outputBuffer, void *inputBuffer,
                                  unsigned int nBufferFrames,
                                  double streamTime, RtAudioStreamStatus /* status */,
                                  void * /* data */ ) // last arg is used for "this"
{
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

int Audio::inout( void *outputBuffer, void *inputBuffer,
                  unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status,
                  void * /* data */ ) // last arg is used for "this"
{
    if ( status ) std::cout << "Stream over/underflow detected." << std::endl;
    if ( streamTime >= m_streamTimePrintTime ) {
        std::cout << "streamTime = " << streamTime << std::endl;
        m_streamTimePrintTime += m_streamTimePrintIncrement;
    }
    unsigned int bytes = nBufferFrames*m_channels*sizeof(MY_TYPE);
    memcpy( outputBuffer, inputBuffer, bytes);
    return 0;
}

int Audio::wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                  unsigned int nBufferFrames, double streamTime,
                                  RtAudioStreamStatus status, void *arg)
{
    //        return static_cast<Audio*>(arg)->inout(
    return static_cast<Audio*>(arg)->networkAudio_callback(
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
            if ( m_adac->isStreamOpen() ) m_adac->closeStream();
            std::cout << "Audio stream closed" << std::endl;
        }
}

void TCP::connectToHost()
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
}

void TCP::sendToHost()
{
    if (mSocket->state()==QTcpSocket::ConnectedState) {
        QByteArray ba;
        qint32 tmp = 4464;
        ba.setNum(tmp);
        mSocket->write(ba);
        mSocket->waitForBytesWritten(1500);
        std::cout << "TCP: waitForBytesWritten" << std::endl;
        mSocket->waitForReadyRead();
        quint16 bytes =  mSocket->bytesAvailable();
        std::cout << "TCP: from server " << bytes << std::endl;

        uint32_t udp_port;
        int size       = sizeof(udp_port);
        char* port_buf = new char[size];
        mSocket->read(port_buf, size);
        udp_port = qFromLittleEndian<qint32>(port_buf);

        std::cout << "TCP: ephemeral port = " << udp_port << std::endl;
    } else
        std::cout << "TCP: tried to send data but not connected to server" << std::endl;
}

UDP::UDP(Regulator * reg)
    : mRegFromHackTrip(reg)
{
    mPhasor.resize(HackTrip::mChannels, 0.0);
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
    mPeerPort = 61002;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
               (void *) &optval, sizeof(optval));
    mSock = new QUdpSocket(this);
    mSock->setSocketDescriptor(sockfd, QUdpSocket::UnconnectedState);
    int ret = 0;
    ret = mSock->bind(HackTrip::mAudioPort);
    std::cout << "UDP: start send = " << ret << " " << serverHostAddress.toString().toLocal8Bit().data() <<  std::endl;
    connect(mSock, &QUdpSocket::readyRead, this, &UDP::readPendingDatagrams);
    mRing = 500;
    mWptr = 0;
    mRptr = 0;
    for (int i = 0; i < mRing; i++) {
        int8_t* tmp = new int8_t[HackTrip::mAudioDataLen];
        for (int j = 0; j < HackTrip::mAudioDataLen; j++)
            tmp[j] = 0;
        mInBuffer.push_back(tmp);
    }
};

UDP:: ~UDP() {
    delete mSock;
    for (int i = 0; i < mRing; i++) {
        delete mInBuffer[i];
    }
}

//https://stackoverflow.com/questions/40200535/c-qt-qudp-socket-not-sending-receiving-data

void UDP::readPendingDatagrams() {
    //read datagrams in a loop to make sure that all received datagrams are processed
    //since readyRead() is emitted for a datagram only when all previous datagrams are read
    QMutexLocker locker(&mMutex);
    while(mSock->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(mSock->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        mSock->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);
        memcpy(&mHeader,mBuf.data(),sizeof(HeaderStruct));
        int seq = mHeader.SeqNumber;
        if (seq%500 == 0)
            std::cout << "UDP rcv: seq = " << seq << std::endl;
        int8_t *audioBuf = (int8_t *)(mBuf.data() + sizeof(HeaderStruct));
        mRegFromHackTrip->nonILtoIL((int8_t *)audioBuf);
        memcpy(mInBuffer[mWptr],audioBuf,mBuf.size());
        mWptr++;
        mWptr %= mRing;
        //        mRegFromHackTrip->shimFPP(
        //                    (int8_t *)(mBuf.data() + sizeof(HeaderStruct)),
        //                    mBuf.size(), seq);
    }
}

void UDP::send(int seq, int8_t *audioBuf) {
    mHeader.SeqNumber = (uint16_t)seq;
    memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));

    mRegFromHackTrip->ILtoNonIL((int8_t *)audioBuf);

    memcpy(mBuf.data()+sizeof(HeaderStruct),audioBuf,HackTrip::mAudioDataLen);
    mSock->writeDatagram(mBuf, serverHostAddress, mPeerPort);
    if (seq%500 == 0)
        std::cout << "UDP send: packet = " << seq << std::endl;
}

void UDP::stop()
{
    // Send exit packet (with 1 redundant packet).
    int controlPacketSize = 63;
    std::cout << "sending exit packet" << std::endl;
    mBuf.resize(controlPacketSize);
    mBuf.fill(0xff,controlPacketSize);
    mSock->writeDatagram(mBuf, serverHostAddress, mPeerPort);
    mSock->writeDatagram(mBuf, serverHostAddress, mPeerPort);
    mSock->close();
}

/////////////////////////////////////
// unused

int Audio::api_cpp() {
    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi( apis );

    // ensure the known APIs return valid names
    std::cout << "API names by identifier (C++):\n";
    for ( size_t i = 0; i < apis.size() ; ++i ) {
        const std::string name = RtAudio::getApiName(apis[i]);
        if (name.empty()) {
            std::cout << "Invalid name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        const std::string displayName = RtAudio::getApiDisplayName(apis[i]);
        if (displayName.empty()) {
            std::cout << "Invalid display name for API " << (int)apis[i] << "\n";
            exit(1);
        }
        std::cout << "* " << (int)apis[i] << " '" << name << "': '" << displayName << "'\n";
    }

    // ensure unknown APIs return the empty string
    {
        const std::string name = RtAudio::getApiName((RtAudio::Api)-1);
        if (!name.empty()) {
            std::cout << "Bad string for invalid API '" << name << "'\n";
            exit(1);
        }
        const std::string displayName = RtAudio::getApiDisplayName((RtAudio::Api)-1);
        if (displayName!="Unknown") {
            std::cout << "Bad display string for invalid API '" << displayName << "'\n";
            exit(1);
        }
    }

    // try getting API identifier by name
    std::cout << "API identifiers by name (C++):\n";
    for ( size_t i = 0; i < apis.size() ; ++i ) {
        std::string name = RtAudio::getApiName(apis[i]);
        if ( RtAudio::getCompiledApiByName(name) != apis[i] ) {
            std::cout << "Bad identifier for API '" << name << "'\n";
            exit( 1 );
        }
        std::cout << "* '" << name << "': " << (int)apis[i] << "\n";

        for ( size_t j = 0; j < name.size(); ++j )
            name[j] = (j & 1) ? toupper(name[j]) : tolower(name[j]);
        RtAudio::Api api = RtAudio::getCompiledApiByName(name);
        if ( api != RtAudio::UNSPECIFIED ) {
            std::cout << "Identifier " << (int)api << " for invalid API '" << name << "'\n";
            exit( 1 );
        }
    }

    // try getting an API identifier by unknown name
    {
        RtAudio::Api api;
        api = RtAudio::getCompiledApiByName("");
        if ( api != RtAudio::UNSPECIFIED ) {
            std::cout << "Bad identifier for unknown API name\n";
            exit( 1 );
        }
    }

    return 0;
}
