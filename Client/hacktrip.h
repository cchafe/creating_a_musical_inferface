#ifndef HACKTRIP_H
#define HACKTRIP_H

#include <rtaudio/RtAudio.h>
#include <QTcpSocket>
#include <QThread>
#include <QHostInfo>
#include <QUdpSocket>
#include <QMutex>
#include <QElapsedTimer>
#include <QTimer>

const QString gVersion = "clientV1";
//const QString gServer = "54.215.249.223";
const QString gServer = "jackloop256.stanford.edu";
//const QString gServer = "cmn55.stanford.edu";
//const QString gServer = "cmn9.stanford.edu";
//const QString gServer = "171.64.197.158";
//const QString gServer = "127.0.0.2"; // don't use "loopback", doesn't resolve
//const QString gServer = "localhost";
typedef signed short MY_TYPE; // audio interface data is 16bit ints

struct HeaderStruct {
public:
    // watch out for alignment...
    uint64_t TimeStamp;     ///< Time Stamp
    uint16_t SeqNumber;     ///< Sequence Number
    uint16_t BufferSize;    ///< Buffer Size in Samples
    uint8_t SamplingRate;   ///< Sampling Rate in JackAudioInterface::samplingRateT
    uint8_t BitResolution;  ///< Audio Bit Resolution
    uint8_t NumIncomingChannelsFromNet;  ///< Number of incoming Channels from the network
    uint8_t NumOutgoingChannelsToNet;    ///< Number of outgoing Channels to the network
};

class UDP : public QUdpSocket {
public:
    void start();
    void setPeerUdpPort(int port) { mPeerUdpPort = port; }
    void stop();
    void send(int8_t *audioBuf);
private:
    //    QMutex mMutex;                     ///< Mutex to protect read and write operations
    int mWptr;
    int mRptr;
    int mRing;
    std::vector<int8_t*> mRingBuffer;
    QHostAddress serverHostAddress;
    HeaderStruct mHeader;
    QHostAddress mPeerAddr;
    int mPeerUdpPort;
    QByteArray mBufSend;
    QByteArray mBufRcv;
    int mSendSeq;
    QElapsedTimer mRcvTmer;
    QTimer mRcvTimeout;
    QTimer mSendTmer;
public slots:
    void readPendingDatagrams();
    void sendDummyData();
    void rcvTimeout();
};

class TCP : public QTcpSocket {
public:
    int connectToServer();
};

class HackTrip
{
public:
    void connect();
    void run();
    void stop();
private:
    static const int mServerTcpPort = 4464;
    static const int mLocalAudioUdpPort = 4464;
    static const int mFPP = 256;
    static const int mSocketWaitMs = 1500;
    static const int mSampleRate = 48000;
    static const int mChannels = 2;
    static const int mBufferQueueLength = 3; // queue not used for localhost testing
    static const int mBytesPerSample = sizeof(MY_TYPE);
    static const int mAudioDataLen = mFPP * mChannels * mBytesPerSample;
    static const int mExitPacketSize = 63;
    static const int mTimeoutMS = 1000;
    constexpr static const double mPacketPeriodMS = (1000.0 / (double)(mSampleRate / mFPP));
    friend class TCP;
    friend class UDP;
    friend class TestAudio;
    TCP mTcp;
    UDP mUdp;
};

#endif // HACKTRIP_H
