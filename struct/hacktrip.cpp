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
    mAudio = new Audio(mReg); // calls duplex
}

void HackTrip::start()
{
    //    mUdpSend = new UDP(); // send to network
//    mUdpSend = new UDP(mReg); // push to reg, for internal test
    //    mUdpRcv = new UDP(mReg, true); // rcv from network, push to reg
//    mUdpSend->start();
    //    mUdpRcv->start();
    mAudio->start();
}

void HackTrip::stop()
{
    //    mUdpRcv->stop();
    //    mUdpRcv->wait();
//    mUdpSend->stop();
//    mUdpSend->wait();
    mAudio->stop();
}

HackTrip:: ~HackTrip() {
    delete mAudio;
    delete mReg;
    //    delete mUdpSend;
    //    delete mUdpRcv;
}

Audio::Audio(Regulator * reg)
    : mRegFromHackTrip(reg)
{
    m_adac = 0;
    mPhasor.resize(2, 0.0); //HackTrip::mChannels
    int audioDataLen = 256*2*2;
    mZeros = new QByteArray();
    mZeros->resize(audioDataLen);
    mZeros->fill(0,audioDataLen);
    seq = 0;
};

Audio:: ~Audio() {
}

void HackTrip::contactServer()
{
    //    mTcpClient.connectToHost();
    //    mTcpClient.sendToHost();
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
    {
        // this is the push
        seq++;
        seq %= 65536;
        if (seq%500 == 0)
            std::cout << "test packet = " << seq << " to shimFPP from audio callback " << std::endl;
        int dontSizeMeFromNetworkPacketYet = 0;
        // write sines to mXfr, memcpy mXfr to mZeros
        mRegFromHackTrip->sineTestPacket((int8_t *)mZeros->data());
        // write mZeros to pushPacket. memcpy to mLastSeqNumIn
        mRegFromHackTrip->shimFPP((int8_t *)mZeros->data(), dontSizeMeFromNetworkPacketYet, seq);

    }
    if (true) { // DSP block

        // this should be a pull
        mRegFromHackTrip->pullPacket((int8_t *)outputBuffer);
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
    //    unsigned int bytes = nBufferFrames*m_channels*sizeof(MY_TYPE);
    //    memcpy( outputBuffer, inputBuffer, bytes);
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
    options.numberOfBuffers = HackTrip::mFPP;
    // value set by the user is replaced during execution of the RtAudio::openStream() function by the value actually used by the system
    std::cout << "using default audio interface device\n";
    std::cout << m_adac->getDeviceInfo(m_iDevice).name
              << "\tfor input and output\n";
    std::cout << "\tIf another is needed, either change your settings\n";
    std::cout << "\tor the choice in the code\n";
    std::cout << "asking for numberOfBuffers = " << options.numberOfBuffers << "\n";
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
        if (options.numberOfBuffers) {
            std::cout << "\tgot numberOfBuffers = " << options.numberOfBuffers << "\n";
        } else {
            std::cout << "\trunning " <<
                         m_adac->getApiDisplayName(m_adac->getCurrentApi()) <<
                         "\n\twhich sets the actual numberOfBuffers and must match\n";
            std::cout << "\nStream latency = " << m_adac->getStreamLatency()
                      << " frames" << std::endl;
        }
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

void UDP::run() {
    int audioDataLen = HackTrip::mFPP * HackTrip::mChannels * sizeof(MY_TYPE);
    mHeader.TimeStamp = (uint64_t)0;
    mHeader.SeqNumber = (uint16_t)0;
    mHeader.BufferSize = (uint16_t)HackTrip::mFPP;
    mHeader.SamplingRate = (uint8_t)3;
    mHeader.BitResolution = (uint8_t)sizeof(MY_TYPE) * 8; // checked in jacktrip
    mHeader.NumIncomingChannelsFromNet = HackTrip::mChannels;
    mHeader.NumOutgoingChannelsToNet = HackTrip::mChannels;
    int packetDataLen = sizeof(HeaderStruct) + audioDataLen;
    mBuf.resize(packetDataLen);
    mBuf.fill(0,packetDataLen);
    memcpy(mBuf.data(),&mHeader,sizeof(HeaderStruct));

    std::cout << "Default Packet Header: Buffer Size = "
              << static_cast<int>(mHeader.BufferSize) << std::endl;
    // Get the sample rate in Hz from the AudioInterface::samplingRateT
    //    int sample_rate = mHeader.SamplingRate;
    // socket needs to be created here in this thread
    QUdpSocket sock;
    QHostAddress serverHostAddress;

    if(mTest) {
        std::cout << "UDP test: start " << std::endl;
        int audioDataLen = HackTrip::mFPP*HackTrip::mChannels*sizeof(MY_TYPE);
        mZeros = new QByteArray();
        mZeros->resize(audioDataLen);
        mZeros->fill(0,audioDataLen);
    } else {
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
            ret = sock.bind(QHostAddress::Any, HackTrip::mAudioPort);
            std::cout << "UDP rcv: start = " << ret << std::endl;
        } else { // sender
            ret = sock.bind(HackTrip::mAudioPort);
            std::cout << "UDP send: start = " << ret << " " << serverHostAddress.toString().toLocal8Bit().data() <<  std::endl;
        }

        msleep(100);
    }

    int seq = 0;
    //    int audioDataLen = gFPP*2*2;
    //    int packetDataLen = sizeof(HeaderStruct)+audioDataLen;
    unsigned long uSecPeriod = (222.0/48000.0) * 1000000.0;
    mStop = false;
    while (!mStop) {
        if (mTest) {
            seq++;
            seq %= 65536;
            if (seq%500 == 0)
                std::cout << "test packet = " << seq << "\tperiod(uSec) = " << uSecPeriod << std::endl;
            int dontSizeMeFromNetworkPacketYet = 0;

            // write sines to mXfr, memcpy mXfr to mZeros
            mRegFromHackTrip->sineTestPacket((int8_t *)mZeros->data());
            // write mZeros to pushPacket. memcpy to mLastSeqNumIn
            mRegFromHackTrip->shimFPP((int8_t *)mZeros->data(), dontSizeMeFromNetworkPacketYet, seq);

            usleep(uSecPeriod);
        } else {
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
                        if (seq%500 == 0)
                            std::cout << "UDP rcv: seq = " << seq << std::endl;
                        if (true) { // DSP block
                            // this should be a push
                            inBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);
                            mRegFromHackTrip->shimFPP((int8_t *)inBuffer, len, seq);
                            //MY_TYPE *outBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);

                            //                        //                        double tmp[gFPP * gChannels];

                            //                        // from network
                            //                        for (unsigned int i = 0; i < HackTrip::mFPP; i++) {
                            //                            for (unsigned int j = 0; j < HackTrip::mChannels; j++) {
                            //                                unsigned int index = i * HackTrip::mChannels + j;
                            //                                //                                gTmpAudio[index] = *inBuffer++ / SCALE;
                            //                            }
                            //                        }
                            //                        // in audio callback
                            //                        //                        for (unsigned int i = 0; i < nBufferFrames; i++) {
                            //                        //                          for (unsigned int j = 0; j < gChannels; j++) {
                            //                        //                            unsigned int index = i * gChannels + j;
                            //                        //                            *outBuffer++ = (MY_TYPE)(tmp[index] * SCALE);
                            //                        //                          }
                            //                        //                        }

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
                    double tmp[HackTrip::mFPP * HackTrip::mChannels];
                    for (unsigned int i = 0; i < HackTrip::mFPP; i++) {
                        for (unsigned int j = 0; j < HackTrip::mChannels; j++) {
                            unsigned int index = i * HackTrip::mChannels + j;
                            //                    tmp[index] = *inBuffer++ / SCALE;
                            tmp[index] = ((i%3)==0) ? 0.00 : 0.0;

                            // diagnostic output
                            /////////////////////
                            if (false) {
                                if (j) tmp[index] = (0.7 * sin(mPhasor[j]));
                                mPhasor[j] += (!j) ? 0.1 : 0.11;
                            }
                            /////////////////////

                            *outBuffer++ = (MY_TYPE)(tmp[index] * SCALE);
                        }
                    }
                }
                sock.writeDatagram(mBuf, serverHostAddress, mPeerPort);
                if (seq%500 == 0)
                    std::cout << "UDP send: packet = " << seq << std::endl;
                usleep(2666);
            }
        } // network
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

void UDP::test() {
    std::cout << "test " << mZeros->size() << std::endl;
};
