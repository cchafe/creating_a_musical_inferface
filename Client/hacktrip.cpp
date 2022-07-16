#include "hacktrip.h"
#include <QtEndian>
#include <QUdpSocket>
#include <netinet/in.h>
#include <sys/types.h>
#include <math.h>

void HackTrip::run()
{
    mAudio.setTest(HackTrip::mChannels);
    mAudio.start();
}

void HackTrip::stop()
{
    mAudio.stop();
}

int Audio::audioCallback( void *outputBuffer, void *inputBuffer,
                        unsigned int /* nBufferFrames */,
                        double /* streamTime */, RtAudioStreamStatus /* status */,
                        void * /* data */ ) // last arg is used for "this"
{
    // audio diagnostics, modify or print output and input buffers
//        memcpy(outputBuffer, inputBuffer, HackTrip::mAudioDataLen); // test straight wire
        mTest->sineTest((MY_TYPE *)outputBuffer); // output sines
        mTest->printSamples((MY_TYPE *)outputBuffer); // print audio signal

    return 0;
}
int Audio::wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                  unsigned int nBufferFrames, double streamTime,
                                  RtAudioStreamStatus status, void *arg)
{
    return static_cast<Audio*>(arg)->audioCallback(
                outputBuffer, inputBuffer, nBufferFrames, streamTime, status, arg);
}

void Audio::start() {
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
    options.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;
    options.numberOfBuffers = HackTrip::mNumberOfBuffersSuggestionToRtAudio; // Windows DirectSound, Linux OSS, and Linux Alsa APIs only.
    // value set by the user is replaced during execution of the RtAudio::openStream() function by the value actually used by the system
    std::cout << "using default audio interface device\n";
    std::cout << m_adac->getDeviceInfo(m_iDevice).name
              << "\tfor input and output\n";
    std::cout << "\tIf another is needed, either change your settings\n";
    std::cout << "\tor the choice in the code\n";
    m_adac->showWarnings(true);
    unsigned int bufferFrames = HackTrip::mFPP;
    if (m_adac->openStream( &m_oParams, &m_iParams, FORMAT, HackTrip::mSampleRate,
                            &bufferFrames, &Audio::wrapperProcessCallback,
                            (void*)this,  &options ))
        std::cout << "\nCouldn't open audio device streams!\n";
    if (m_adac->isStreamOpen() == false) {
        std::cout << "\nCouldn't open audio device streams!\n";
        exit(1);
    } else {
        std::cout << "\trunning " <<
                     m_adac->getApiDisplayName(m_adac->getCurrentApi()) << "\n";
        std::cout << "\nStream latency = " << m_adac->getStreamLatency()
                  << " frames" << std::endl;
    }
    std::cout << "\nAudio stream start" << std::endl;
    if (m_adac->startStream())
        std::cout << "\nCouldn't start streams!\n";
    std::cout << "\nAudio stream started" << std::endl;
}

void Audio::stop()
{
    if (m_adac)
        if (m_adac->isStreamRunning()) {
            std::cout << "\nAudio stream stop" << std::endl;
            m_adac->stopStream();
            if ( m_adac->isStreamOpen() ) {
                m_adac->closeStream();
                std::cout << "Audio stream closed" << std::endl;
            }
        }
}

TestAudio::TestAudio(int channels) {
    mPhasor.resize(channels, 0.0);
}

void TestAudio::sineTest(MY_TYPE *buffer) {
    for ( int ch=0; ch < HackTrip::mChannels; ch++ ) {
        for ( int i=0; i < HackTrip::mFPP; i++ ) {
            double tmp = sin(mPhasor[ch]);
            *buffer++ = (MY_TYPE) (tmp * HackTrip::mScale);
            mPhasor[ch] += ((ch) ? 0.20 : 0.22);
        }
    }
}

void TestAudio::printSamples(MY_TYPE *buffer) {
    for ( int ch=0; ch < HackTrip::mChannels; ch++ ) {
        for ( int i=0; i < HackTrip::mFPP; i++ ) {
            double tmp = ((MY_TYPE) *buffer++) * HackTrip::mInvScale;
            std::cout << "\t" << tmp << std::endl;
        }
    }
}
