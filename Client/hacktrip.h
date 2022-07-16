#ifndef HACKTRIP_H
#define HACKTRIP_H

#include <rtaudio/RtAudio.h>
#include <QTcpSocket>
#include <QThread>
#include <QHostInfo>
#include <QUdpSocket>
#include <QMutex>

const QString gVersion = "module-1-audio";
typedef signed short MY_TYPE; // audio interface data is 16bit ints
#define FORMAT RTAUDIO_SINT16 // which has this rtaudio name
#define SCALE 32767.0 // audio samples for processing are doubles, so this is the conversion

class TestAudio {
public:
    TestAudio(int channels);
    void printSamples(MY_TYPE *buffer);
    void sineTest(MY_TYPE *buffer);
private:
    std::vector<double> mPhasor;
};

class Audio
{
public:
    void start();
    void stop();
    int audioCallback(void *outputBuffer, void *inputBuffer,
                       unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus,
                       void *bytesInfoFromStreamOpen);
    static int wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                      unsigned int nBufferFrames, double streamTime,
                                      RtAudioStreamStatus status, void *arg);
    void setTest(int channels) { mTest = new TestAudio(channels); }
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
    TestAudio * mTest;
};

class HackTrip
{
public:
    void run();
    void stop();
private:
    static const int mFPP = 256;
    static const int mSampleRate = 48000;
    static const int mChannels = 2;
    static const int mBytesPerSample = sizeof(MY_TYPE);
    static const int mAudioDataLen = mFPP * mChannels * mBytesPerSample;
    constexpr static const double mScale = 32767.0;
    constexpr static const double mInvScale = 1.0 / 32767.0;
    static const int mNumberOfBuffersSuggestionToRtAudio = 2;
    friend class Audio;
    friend class TestAudio;
    Audio mAudio;
};

#endif // HACKTRIP_H
