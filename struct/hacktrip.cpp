#include "hacktrip.h"
#include <QtEndian>
#include <QUdpSocket>

HackTrip::HackTrip()
{
    duplex(0); // use the default audio interface
}

void HackTrip::duplex(int device)
{
    m_channels = mChannels;
    m_fs = mSampleRate;
    m_iDevice = m_oDevice = device;
    m_iOffset = m_oOffset = 0; // first channel
    // copy all setup into all stream info
    m_iParams.deviceId = m_iDevice;
    m_iParams.nChannels = m_channels;
    m_iParams.firstChannel = m_iOffset;
    m_oParams.deviceId = m_oDevice;
    m_oParams.nChannels = m_channels;
    m_oParams.firstChannel = m_oOffset;
    m_adac = new RtAudio();
    if (m_adac->getDeviceCount() < 1) {
        std::cout << "\nNo audio devices found!\n";
        exit(1);
    } else {
        std::cout << "using default audio interface device\n";
        std::cout << m_adac->getDeviceInfo(device).name
                  << "\tfor input and output\n";
        std::cout << "\tIf another is needed, either change your settings\n";
        std::cout << "\tor the choice in the code\n";
        // Let RtAudio print messages to stderr.
        m_adac->showWarnings(true);
    }
    options.flags = RTAUDIO_SCHEDULE_REALTIME; // use realtime priority if it's available
    unsigned int dummy = mFPP;
    m_adac->openStream(&m_oParams, &m_iParams,
                         FORMAT, mSampleRate, &dummy,
                         &Audio::wrapperProcessCallback,
                         (void *)this, &options);
}

void HackTrip::contactServer()
{
    mTcpClient.connectToHost();
    mTcpClient.sendToHost();
}

void HackTrip::start()
{
    mUdpSend = new UDP();
    mUdpRcv = new UDP(true);
    mUdpSend->start();
    mUdpRcv->start();
    mAudio = new Audio(m_adac);
    mAudio->start();
}

int Audio::wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                  unsigned int nBufferFrames, double streamTime,
                                  RtAudioStreamStatus status, void *arg)
{
    std::cout << "Stream xxxxxxxxxxxx" << std::endl;
    return static_cast<Audio*>(arg)->networkAudio_callback(
                outputBuffer, inputBuffer, nBufferFrames, streamTime, status, arg);
}

int Audio::networkAudio_callback
(void *outputBuffer, void *inputBuffer,
 unsigned int nBufferFrames, double streamTime,
 RtAudioStreamStatus status, void *bytesInfoFromStreamOpen) {
    std::cout << "Stream !!!!!!!!!!!!!!!!" << std::endl;
    //    if (false) { // DSP block
    //        MY_TYPE *inBuffer = mUdpRcv->mostRecentPacket(500);
    //        MY_TYPE *outBuffer = (MY_TYPE *)outputBuffer;
    //        double tmp[nBufferFrames * 2];
    //        for (unsigned int i = 0; i < nBufferFrames; i++) {
    //            for (unsigned int j = 0; j < 2; j++) {
    //                unsigned int index = i * 2 + j;
    //                        tmp[index] = *inBuffer++ / SCALE;
    //                        //      std::cout << "frame = " << frameCounter + i << "\tchannel = " <<
    //                        //      j
    //                        //                << "\tval = " << tmp[i] << std::endl;
    //                        tmp[index] *= 0.3;
    //                        tmp[index] = (j == 1) ? tmp[index - 1] : tmp[index];
    //                *outBuffer++ = (MY_TYPE)(tmp[index] * SCALE);
    //            }
    //        }
    //    }
    return 0;
}

void Audio::run() {
    if (mRTaudio->isStreamOpen() == false) {
        std::cout << "\nCouldn't open audio device streams!\n";
        exit(1);
    } else {
        std::cout << "\nStream latency = " << mRTaudio->getStreamLatency()
                  << " frames" << std::endl;
    }
    std::cout << "\nAudio stream start" << std::endl;
    mRTaudio->startStream();
    double localStreamTime = 0.0;
    mStop = false;
    while (!mStop) {
        msleep(100);
    };
    if (mRTaudio->isStreamOpen())
        mRTaudio->closeStream();
    std::cout << "\nAudio stream closed" << std::endl;
}

void Audio::stop()
{
    mStop = true;
}

void HackTrip::stop()
{
    mUdpRcv->stop();
    mUdpRcv->wait();
    mUdpSend->stop();
    mUdpSend->wait();
    mAudio->stop();
    mAudio->wait();
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
    {
        int audioDataLen = HackTrip::mFPP*2*2;
        mHeader.TimeStamp = (uint64_t)0;
        mHeader.SeqNumber = (uint16_t)0;
        mHeader.BufferSize = (uint16_t)HackTrip::mFPP;
        mHeader.SamplingRate = (uint8_t)3;
        mHeader.BitResolution = (uint8_t)16;
        mHeader.NumIncomingChannelsFromNet = (uint8_t)2;
        mHeader.NumOutgoingChannelsToNet = (uint8_t)2;
        mZeros.resize(audioDataLen);
        mZeros.fill(0,audioDataLen);
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
        ret = sock.bind(QHostAddress::Any, HackTrip::mAudioPort);
        std::cout << "UDP rcv: start = " << ret << std::endl;
    } else { // sender
        ret = sock.bind(HackTrip::mAudioPort);
        std::cout << "UDP send: start = " << ret << " " << serverHostAddress.toString().toLocal8Bit().data() <<  std::endl;
    }
    msleep(100);
    int seq = 0;
    //    int audioDataLen = gFPP*2*2;
    //    int packetDataLen = sizeof(HeaderStruct)+audioDataLen;
    mStop = false;
    while (!mStop) {
        if (mRcv) {
            //            std::cout << "UDP check incoming " << sock.pendingDatagramSize() << std::endl;
            if (sock.hasPendingDatagrams()) {
                //                std::cout << "UDP rcv: pending bytes = " << sock.pendingDatagramSize() << std::endl;
                int len = sock.readDatagram(mBuf.data(), mBuf.size());
                std::cout << "UDP rcv: bytes = " << len << std::endl;
                if (len != mBuf.size())
                    std::cout << "UDP rcv: not full packet (" << len << ") should be " << mBuf.size() << std::endl;
                else {
                    memcpy(&mHeader,mBuf.data(),sizeof(HeaderStruct));
                    seq = mHeader.SeqNumber;
                    std::cout << "UDP rcv: packet = " << seq << std::endl;
                    if (true) { // DSP block
                        inBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);                         MY_TYPE *outBuffer = (MY_TYPE *)mBuf.data() + sizeof(HeaderStruct);
                        //                        double tmp[gFPP * gChannels];

                        // from network
                        for (unsigned int i = 0; i < HackTrip::mFPP; i++) {
                            for (unsigned int j = 0; j < HackTrip::mChannels; j++) {
                                unsigned int index = i * HackTrip::mChannels + j;
                                //                                gTmpAudio[index] = *inBuffer++ / SCALE;
                            }
                        }
                        // in audio callback
                        //                        for (unsigned int i = 0; i < nBufferFrames; i++) {
                        //                          for (unsigned int j = 0; j < gChannels; j++) {
                        //                            unsigned int index = i * gChannels + j;
                        //                            *outBuffer++ = (MY_TYPE)(tmp[index] * SCALE);
                        //                          }
                        //                        }

                    }
                }
                usleep(1);
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

MY_TYPE * UDP::mostRecentPacket(int afterPacket) {
    if (mHeader.SeqNumber>afterPacket)
        return inBuffer;
    else {
        return (MY_TYPE *)mZeros.data();
    }
};
