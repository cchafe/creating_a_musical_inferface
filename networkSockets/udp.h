#ifndef UDP_H
#define UDP_H

#include <QThread>
#include <QUdpSocket>
#include <QHostInfo>

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
    UDP();
    void stop();
    void setRcv();
private:
    virtual void run();
    HeaderStruct mHeader;
    QHostAddress mPeerAddr;
    int mPeerPort;
    QByteArray mBuf;
    bool mStream;
    bool mRcv; // else Send
};

#endif // UDP_H
