#ifndef HACKTRIP_H
#define HACKTRIP_H

#include <QObject>
#include <rtaudio/RtAudio.h>
#include <QThread>
#include <QHostInfo>
#include <QUdpSocket>
#include "regulator.h"

//const QString gServer = "54.193.131.283";
const QString gServer = "jackloop256.stanford.edu";
//const QString gServer = "cmn55.stanford.edu";
//const QString gServer = "cmn9.stanford.edu";
//const QString gServer = "171.64.197.158";
//const QString gServer = "127.0.0.1"; // don't use "loopback", doesn't resolve
typedef signed short MY_TYPE; // audio interface data is 16bit ints
#define FORMAT RTAUDIO_SINT16 // which has this rtaudio name
#define SCALE 32767.0 // audio samples for processing are doubles, so this is the conversion

#include <QTcpSocket>
class TCP
{
public:
    TCP(){};
    QTcpSocket * mSocket;
    void connectToHost();
    void sendToHost();
    void bytesWritten(qint64 bytes);
};

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

class UDP : public QThread
{
public:
    UDP(int audioDataLen, Regulator * reg = 0);

    void test();
    void stop();
    void send(int seq, int8_t *audioBuf);
private:
    int mAudioDataLen;
    QUdpSocket sendSock;
    QHostAddress serverHostAddress;
    virtual void run();
    HeaderStruct mHeader;
    QHostAddress mPeerAddr;
    Regulator * mRegFromHackTrip;
    int mPeerPort;
    QByteArray mBuf;
    bool mStop;
    bool mTest;
    bool mRcv;
    MY_TYPE *inBuffer;
    QByteArray *mZeros;
    std::vector<double> mPhasor;
};


class Audio
{
public:
    Audio( int audioDataLen, Regulator * reg, UDP * udpSend);
    ~Audio();
    void start();
    void stop();
    int api_cpp();
    int inout( void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/,
               double streamTime, RtAudioStreamStatus status, void *data );
    static int wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                      unsigned int nBufferFrames, double streamTime,
                                      RtAudioStreamStatus status, void *arg);
    unsigned int bufferFrames;
    unsigned int bufferBytes;
    //    RtAudioStreamStatus status, void *arg);
private:
    // these are identical to the rtaudio/tests/Duplex.cpp example
    // except with m_ prepended
    RtAudio *m_adac;
    double m_streamTimePrintIncrement;
    double m_streamTimePrintTime;
    unsigned int m_channels;
    unsigned int m_fs;
    unsigned int m_oDevice;
    unsigned int m_iDevice;
    unsigned int m_iOffset;
    unsigned int m_oOffset;
    RtAudio::StreamParameters m_iParams;
    RtAudio::StreamParameters m_oParams;
    RtAudio::StreamOptions options;
    RtAudio *mRTaudio;
    void duplex(int device);
    bool mStop;
    int networkAudio_callback(void *outputBuffer, void *inputBuffer,
                              unsigned int nBufferFrames, double streamTime,
                              RtAudioStreamStatus status,
                              void *bytesInfoFromStreamOpen);
    Regulator * mRegFromHackTrip;
    UDP * mUdpSend;
    std::vector<double> mPhasor;
    QByteArray *mZeros;
    int seq;
};

class HackTrip
{
public:
    HackTrip();
    ~HackTrip();
    void contactServer();
    void start();
    void stop();
private:
    Regulator * mReg;
    const QString mPort = "4464";
    static const int mAudioPort = 4464;
    static const int mFPP = 256;
    static const int mSocketWaitMs = 1500;
    static const int mSampleRate = 48000;
    static const int mChannels = 2;
    static const int mBufferQueueLength = 5;
    friend class TCP;
    friend class UDP;
    friend class Audio;
    TCP mTcpClient;
//    UDP *mUdpFake;
//    UDP *mUdpSend;
//    UDP *mUdpRcv;
    UDP *mUdp; // main thread is send, qthread is rcv
    Audio *mAudio;
    int audioDataLen;
};



#endif // HACKTRIP_H
