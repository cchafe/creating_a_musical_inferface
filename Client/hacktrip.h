#ifndef HACKTRIP_H
#define HACKTRIP_H

#include <rtaudio/RtAudio.h>
#include <QTcpSocket>
#include <QThread>
#include <QHostInfo>
#include <QUdpSocket>
#include <QMutex>

//const QString gServer = "54.193.131.283";
//const QString gServer = "jackloop256.stanford.edu";
//const QString gServer = "cmn55.stanford.edu";
//const QString gServer = "cmn9.stanford.edu";
//const QString gServer = "171.64.197.158";
const QString gServer = "127.0.0.1"; // don't use "loopback", doesn't resolve
typedef signed short MY_TYPE; // audio interface data is 16bit ints
#define FORMAT RTAUDIO_SINT16 // which has this rtaudio name
#define SCALE 32767.0 // audio samples for processing are doubles, so this is the conversion


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

class UDP : public QUdpSocket
{
public:
    UDP();
    ~UDP();
    void start();
    void stop();
    void send(int seq, int8_t *audioBuf);
    std::vector<int8_t*> mInBuffer;
    bool quit;
    int mWptr;
    int mRptr;
    int mRing;
    QMutex mMutex;                     ///< Mutex to protect read and write operations
    int mPeerPort;
private:
    QHostAddress serverHostAddress;
    HeaderStruct mHeader;
    QHostAddress mPeerAddr;
    QByteArray mBuf;
public slots:
    void readPendingDatagrams();
};

class TCP {
public:
    TCP(UDP * udp);
    ~TCP();
    void connectToServer();
    void sendToServer();
private:
    QTcpSocket * mSocket;
    UDP * mUdp;
};

class Audio
{
public:
    Audio(UDP * udp);
    ~Audio();
    void start();
    void stop();
    static int wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                      unsigned int nBufferFrames, double streamTime,
                                      RtAudioStreamStatus status, void *arg);
    unsigned int bufferFrames;
    unsigned int bufferBytes;
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
    int audio_callback(void *outputBuffer, void *inputBuffer,
                              unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus,
                              void *bytesInfoFromStreamOpen);
    UDP * mUdp;
    int seq;
};

class HackTrip
{
public:
    HackTrip();
    ~HackTrip();
    void connect();
    void run();
    void stop();
private:
    const QString mPort = "4464";
    static const int mBytesPerSample = sizeof(MY_TYPE);
    static const int mAudioPort = 4464;
    static const int mFPP = 256;
    static const int mSocketWaitMs = 1500;
    static const int mSampleRate = 48000;
    static const int mChannels = 2;
    static const int mBufferQueueLength = 3;
    static const int mNumberOfBuffersRtAudio = 2;
    static const int mAudioDataLen = mFPP * mChannels * mBytesPerSample;
    friend class TCP;
    friend class UDP;
    friend class Audio;
    TCP *mTcp;
    UDP *mUdp;
    Audio *mAudio;
};

#endif // HACKTRIP_H
