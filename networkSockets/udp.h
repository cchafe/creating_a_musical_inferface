#ifndef UDP_H
#define UDP_H

#include <QThread>
#include <QUdpSocket>
#include <QHostInfo>
#include "globals.h"

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
    void pause();
    void stop();
    void setRcv();
    MY_TYPE *mostRecentPacket(int afterPacket);
private:
    virtual void run();
    HeaderStruct mHeader;
    QHostAddress mPeerAddr;
    int mPeerPort;
    QByteArray mBuf;
    QByteArray mZeros;
    bool mStop;
    bool mRcv; // else Send
    MY_TYPE *inBuffer;
};

#endif // UDP_H
