#ifndef HACKTRIP_H
#define HACKTRIP_H

#include <QObject>
#include <rtaudio/RtAudio.h>
#include <QThread>
#include <QHostInfo>

const QString gServer = "15.181.16.36";
//const QString gServer = "jackloop64.stanford.edu";
//const QString gServer = "cmn55.stanford.edu";
//const QString gServer = "cmn9.stanford.edu";
//const QString gServer = "171.64.197.158";
//const QString gServer = "localhost";
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
    UDP(bool rcv = false):mRcv(rcv){
        int audioDataLen = 64*2*2;
        mZeros = new QByteArray();
        mZeros->resize(audioDataLen);
        mZeros->fill(0,audioDataLen);

    };
    void test();
    void stop();
    MY_TYPE *mostRecentPacket(int afterPacket);
private:
    virtual void run();
    HeaderStruct mHeader;
    QHostAddress mPeerAddr;
    int mPeerPort;
    QByteArray mBuf;
    bool mStop;
    bool mRcv; // else Send
    MY_TYPE *inBuffer;
    QByteArray *mZeros;
};

class Audio
{
public:
    Audio(RtAudio *rtaudio, UDP * udpRcv) : mRTaudio(rtaudio)
      ,  mUdpRcv(udpRcv) {
        mUdpRcv->test();
        int audioDataLen = 64*2*2;
        mZeros.resize(audioDataLen);
        mZeros.fill(0,audioDataLen);
    };
    void start(UDP *udpRcv);
    void stop();
    MY_TYPE *mostRecentPacket(int afterPacket);
    static int wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                      unsigned int nBufferFrames, double streamTime,
                                      RtAudioStreamStatus status, void *arg);
private:
    RtAudio *mRTaudio;
    bool mStop;
    int networkAudio_callback(void *outputBuffer, void *inputBuffer,
                        unsigned int nBufferFrames, double streamTime,
                        RtAudioStreamStatus status, void *bytesInfoFromStreamOpen);
    QByteArray mZeros;
    UDP * mUdpRcv;
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
    void duplex(int device);
    // these are identical to the rtaudio/tests/Duplex.cpp example
    // except with m_ prepended
    unsigned int m_channels;
    unsigned int m_fs;
    unsigned int m_oDevice, m_iDevice, m_iOffset, m_oOffset;
    RtAudio *m_adac;
    RtAudio::StreamParameters m_iParams, m_oParams;
    RtAudio::StreamOptions options;
    const QString mPort = "4464";
    static const int mAudioPort = 4465;
    static const int mFPP = 64;
    static const int mSocketWaitMs = 1500;
    static const int mSampleRate = 48000;
    static const int mChannels = 2;
    friend class TCP;
    friend class UDP;
    friend class Audio;
    TCP mTcpClient;
    UDP *mUdpSend;
    UDP *mUdpRcv;
    Audio *mAudio;
};



#endif // HACKTRIP_H
