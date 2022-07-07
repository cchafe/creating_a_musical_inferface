#include "hacktrip.h"
#include <QtEndian>
#include <QUdpSocket>

// test copied from rtaudio/tests/duplex.cpp
//double streamTimePrintIncrement = 1.0; // seconds
//double streamTimePrintTime = 1.0; // seconds

//int inoutGlobal( void *outputBuffer, void *inputBuffer,
//                 unsigned int /*nBufferFrames*/,
//double streamTime, RtAudioStreamStatus status,
//void *data )
//{
//    // Since the number of input and output channels is equal, we can do
//    // a simple buffer copy operation here.
//    if ( status ) std::cout << "Stream over/underflow detected." << std::endl;

//    if ( streamTime >= streamTimePrintTime ) {
//        std::cout << "streamTime = " << streamTime << std::endl;
//        streamTimePrintTime += streamTimePrintIncrement;
//    }

//    unsigned int *bytes = (unsigned int *) data;
//    memcpy( outputBuffer, inputBuffer, *bytes );
//    return 0;
//}


HackTrip::HackTrip()
{
    mReg = new Regulator(mChannels, sizeof(MY_TYPE),
                         mFPP, mBufferQueueLength);
    mUdp = new UDP(mReg);
    mAudio = new Audio(mReg, mUdp);
}

void HackTrip::start()
{
//    mUdp->start();
    mAudio->start();
}

void HackTrip::stop()
{
//    mUdp->stop();
//    mUdp->wait();
    mAudio->stop();
}

HackTrip:: ~HackTrip() {
    delete mAudio;
    delete mReg;
    delete mUdp;
}

Audio::Audio(Regulator * reg, UDP *udpSend)
    : mRegFromHackTrip(reg)
    , mUdpSend(udpSend)
{
    m_adac = 0;
    mPhasor.resize(2, 0.0); //HackTrip::mChannels
    mZeros = new QByteArray();
    mZeros->resize(HackTrip::mAudioDataLen);
    mZeros->fill(0,HackTrip::mAudioDataLen);
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
                                  double streamTime, RtAudioStreamStatus status,
                                  void * /* data */ ) // last arg is used for "this"
{
    if ( streamTime >= m_streamTimePrintTime ) {
        std::cout << " networkAudio_callback" << " nBufferFrames " << nBufferFrames << " streamTime " << streamTime << std::endl;
        m_streamTimePrintTime += m_streamTimePrintIncrement;
    }
    if (true) {
        // this is network to reg
        seq++;
        seq %= 65536;
        if (seq%500 == 0)
            std::cout << "packet to network = " << seq << std::endl;

        // push internally for test
        //        if (seq%500 == 0)
        //            std::cout << "test packet = " << seq << " to shimFPP from audio callback " << std::endl;
        //        int dontSizeMeFromNetworkPacketYet = 0;
        //        mRegFromHackTrip->shimFPP((int8_t *)mZeros->data(), dontSizeMeFromNetworkPacketYet, seq);

        // write sines to mXfr, memcpy mXfr to mZeros
                mRegFromHackTrip->sineTestPacket((int8_t *)mZeros->data());
//                mUdpSend->send(seq,(int8_t *)mZeros->data());
//        mUdpSend->send(seq,(int8_t *)inputBuffer);

    }
    if (true) { // DSP block

        // this should be a pull
//        mRegFromHackTrip->pullPacket((int8_t *)outputBuffer);
        //        mRegFromHackTrip->dummyPacket((int8_t *)outputBuffer);

        // write sines to mXfr, memcpy mXfr to outputBuffer
        //        mRegFromHackTrip->sineTestPacket((int8_t *)outputBuffer);

        //        // diagnostic output
        //        //        std::cout << "Stream xxxxxxxxxxxx" << std::endl;
        //                MY_TYPE *inBuffer = (MY_TYPE *)inputBuffer;
        //                MY_TYPE *outBuffer = (MY_TYPE *)outputBuffer;
        //                double tmp[nBufferFrames * m_channels];
        //                for (unsigned int i = 0; i < nBufferFrames; i++) {
        //                    for (unsigned int j = 0; j < m_channels; j++) {
        //                        unsigned int index = i * m_channels + j;
        //                        tmp[index] = *inBuffer++ / SCALE; // input signals
        //                                        tmp[index] = (0.7 * sin(mPhasor[j])); // sine output
        //                                        mPhasor[j] += (!j) ? 0.1 : 0.11;
        //                        tmp[index] *= 1.1;
        //        //                tmp[index] = (j == 1) ? tmp[index - 1] : tmp[index]; // left overwrites right
        //                        *outBuffer++ = (MY_TYPE)(tmp[index] * SCALE);
        //                    }
        //                }
    }
//        unsigned int bytes = nBufferFrames*m_channels*sizeof(MY_TYPE);
        memcpy( inputBuffer, mZeros->data(), bufferBytes);
        memcpy( outputBuffer, inputBuffer, bufferBytes);
    return 0;
}

int Audio::inout( void *outputBuffer, void *inputBuffer,
                  unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status,
                  void * /* data */ ) // last arg is used for "this"
{
    // Since the number of input and output channels is equal, we can do
    // a simple buffer copy operation here.
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
    //    std::cout << "Stream xxxxxxxxxxxx" << std::endl;
    //    return static_cast<Audio*>(arg)->inout(
    return static_cast<Audio*>(arg)->networkAudio_callback(
                outputBuffer, inputBuffer, nBufferFrames, streamTime, status, arg);
}

void Audio::duplex(int device)
{
}

void Audio::start() {
    //    api_cpp();
    //    duplex(0); // use the default audio interface
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
    //    if ( m_iDevice == 0 )
    //      m_iParams.deviceId = m_adac->getDefaultInputDevice();
    //    if ( m_oDevice == 0 )
    //      m_oParams.deviceId = m_adac->getDefaultOutputDevice();

    options.flags = RTAUDIO_SCHEDULE_REALTIME; // use realtime priority if it's available
    options.numberOfBuffers = HackTrip::mNumberOfBuffersRtAudio; // Windows DirectSound, Linux OSS, and Linux Alsa APIs only.
    // value set by the user is replaced during execution of the RtAudio::openStream() function by the value actually used by the system
    std::cout << "using default audio interface device\n";
    std::cout << m_adac->getDeviceInfo(m_iDevice).name
              << "\tfor input and output\n";
    std::cout << "\tIf another is needed, either change your settings\n";
    std::cout << "\tor the choice in the code\n";
//    std::cout << "asking for numberOfBuffers = " << options.numberOfBuffers << "\n";
    // Let RtAudio print messages to stderr.
    m_adac->showWarnings(true);

    bufferFrames = HackTrip::mFPP;
    bufferBytes = HackTrip::mFPP*HackTrip::mChannels*sizeof(MY_TYPE);
    //    if (m_adac->openStream( &m_oParams, &m_iParams, FORMAT, HackTrip::mSampleRate,
    //                            &bufferFrames, &inoutGlobal, (void*)&bufferBytes, &options ))
    if (m_adac->openStream( &m_oParams, &m_iParams, FORMAT, HackTrip::mSampleRate,
                            &bufferFrames, &Audio::wrapperProcessCallback,
                            (void*)this,  &options ))
    {
        std::cout << "\nCouldn't open audio device streams!\n";
    }
    if (m_adac->isStreamOpen() == false) {
        std::cout << "\nCouldn't open audio device streams!\n";
        exit(1);
    } else {
//        if (options.numberOfBuffers) {
//            std::cout << "\tgot numberOfBuffers = " << options.numberOfBuffers << "\n";
//        } else {
            std::cout << "\trunning " <<
                         m_adac->getApiDisplayName(m_adac->getCurrentApi()) << "\n";
            std::cout << "\nStream latency = " << m_adac->getStreamLatency()
                      << " frames" << std::endl;
//        }
    }
    std::cout << "\nAudio stream start" << std::endl;
    if (m_adac->startStream())
    {
        std::cout << "\nCouldn't start streams!\n";
    }
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

//#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
UDP::UDP(Regulator * reg)
    : mRegFromHackTrip(reg)
{
    mZeros = new QByteArray();
    mZeros->resize(HackTrip::mAudioDataLen);
    mZeros->fill(0,HackTrip::mAudioDataLen);
    mPhasor.resize(2, 0.0); //HackTrip::mChannels

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
//    ret = rcvSock.bind(QHostAddress::Any, HackTrip::mAudioPort);
    std::cout << "UDP: start = " << ret << " " << serverHostAddress.toString().toLocal8Bit().data() <<  std::endl;

    connect(mSock, &QUdpSocket::readyRead, this, &UDP::readPendingDatagrams);
};

void UDP::readPendingDatagrams() {
    //read datagrams in a loop to make sure that all received datagrams are processed
    //since readyRead() is emitted for a datagram only when all previous datagrams are read
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
        // this should be a push
        inBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);
        mRegFromHackTrip->shimFPP((int8_t *)inBuffer, mBuf.size(), seq);
    }
}

//void UDP::run() {
//    int audioDataLen = HackTrip::mFPP * HackTrip::mChannels * sizeof(MY_TYPE);

//    std::cout << "Default Packet Header: Buffer Size = "
//              << static_cast<int>(mHeader.BufferSize) << std::endl;
//    // Get the sample rate in Hz from the AudioInterface::samplingRateT
//    //    int sample_rate = mHeader.SamplingRate;
//    // socket needs to be created here in this thread
//    QUdpSocket rcvSock;
//    //    QHostAddress serverHostAddress;

//    if(false) {
//        std::cout << "UDP test: start " << std::endl;
//        int audioDataLen = HackTrip::mFPP*HackTrip::mChannels*sizeof(MY_TYPE);
//        mZeros = new QByteArray();
//        mZeros->resize(audioDataLen);
//        mZeros->fill(0,audioDataLen);
//    } else {
//        //        if (!serverHostAddress.setAddress(gServer)) {
//        //            QHostInfo info = QHostInfo::fromName(gServer);
//        //            if (!info.addresses().isEmpty()) {
//        //                // use the first IP address
//        //                serverHostAddress = info.addresses().constFirst();
//        //            }
//        //        }
//        std::cout << "UDP rcv: start = " << ret << std::endl;


//        msleep(100);
//    }

//    int seq = 0;
//    //    int audioDataLen = gFPP*2*2;
//    //    int packetDataLen = sizeof(HeaderStruct)+audioDataLen;
//    unsigned long uSecPeriod = (222.0/48000.0) * 1000000.0;
//    mStop = false;
//    while (!mStop) {
//        //            std::cout << "UDP check incoming " << rcvSock.pendingDatagramSize() << std::endl;
//        if (rcvSock.hasPendingDatagrams()) {
//            //                std::cout << "UDP rcv: pending bytes = " << sock.pendingDatagramSize() << std::endl;
//            int len = rcvSock.readDatagram(mBuf.data(), mBuf.size());
//            //                std::cout << "UDP rcv: bytes = " << len << std::endl;
//            if (len != mBuf.size())
//                std::cout << "UDP rcv: not full packet (" << len << ") should be " << mBuf.size() << std::endl;
//            else {
//                memcpy(&mHeader,mBuf.data(),sizeof(HeaderStruct));
//                seq = mHeader.SeqNumber;
//                if (seq%500 == 0)
//                    std::cout << "UDP rcv: seq = " << seq << std::endl;
//                // this should be a push
//                inBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);
//                mRegFromHackTrip->shimFPP((int8_t *)inBuffer, len, seq);
//            }
//        } // network
////        msleep(1);
//    }
//    //    if (!mRcv) {
//    //        // Send exit packet (with 1 redundant packet).
//    //        int controlPacketSize = 63;
//    //        std::cout << "sending exit packet" << std::endl;
//    //        mBuf.resize(controlPacketSize);
//    //        mBuf.fill(0xff,controlPacketSize);
//    //        sendSock.writeDatagram(mBuf, serverHostAddress, mPeerPort);
//    //        sendSock.writeDatagram(mBuf, serverHostAddress, mPeerPort);
//    //        sendSock.close();
//    //    }
//    rcvSock.close();
//    std::cout << "after rcvSock.close() " << std::endl;
//}

void UDP::send(int seq, int8_t *audioBuf) {
    mHeader.SeqNumber = (uint16_t)seq;
    memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));
    memcpy(mBuf.data()+sizeof(HeaderStruct),audioBuf,HackTrip::mAudioDataLen);
    mSock->writeDatagram(mBuf, serverHostAddress, mPeerPort);
    if (seq%500 == 0)
        std::cout << "UDP send: packet = " << seq << std::endl;
}

void UDP::stop()
{
    mStop = true;
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
}

void UDP::test() {
    std::cout << "test " << mZeros->size() << std::endl;
};
