/*
 * implementation that encapsulates rtaudio and inherits
 * multi-threading from qthread
 * includes the gAudio_callback method for audio streaming to/from
 * the audio interface
 */

#include "duplex.h"

//  run with PIPEWIRE_LATENCY=32/48000 ./hacktrip

// these are from a mix of examples in  rtaudio/tests/
// duplex.cpp
// playsaw.cpp

typedef signed short MY_TYPE; // audio interface data is 16bit ints
#define FORMAT RTAUDIO_SINT16 // which has this rtaudio name
#define SCALE                                                                  \
    32767.0 // audio samples for processing are doubles, so this is the conversion

// rtaudio callbacks are external global functions
// and some global parameters are needed for communication
// with it from this class

// all will be reset by the class constructor
double gStreamTimePrintIncrement = 0.0;
double gStreamTimePrintTime = 0.0;
double gLocalStreamTime = 0.0;
unsigned int gChannels = 0;
unsigned int frameCounter = 0;
bool checkCount = false;
unsigned int nFrames = 0;
const unsigned int callbackReturnValue = 1; // 1 = stop and drain, 2 = abort

#define BASE_RATE 0.05
int gAudio_callback(void *outputBuffer, void *inputBuffer,
                    unsigned int nBufferFrames, double streamTime,
                    RtAudioStreamStatus status, void *bytesInfoFromStreamOpen) {
    if (status)
        std::cout << "Stream over/underflow detected." << std::endl;

    if (streamTime >= gStreamTimePrintTime) {
        std::cout << "streamTime = " << streamTime << std::endl;
        gStreamTimePrintTime += gStreamTimePrintIncrement;
    }

    // simple straight wire with memcpy
    unsigned int *bytes = (unsigned int *)bytesInfoFromStreamOpen;
    memcpy(outputBuffer, inputBuffer, *bytes);
    //  std::cout << "\n bytesInfoFromStreamOpen " << *bytes << "\n";

    if (false) { // DSP block

        //      demonstrates how to access incoming samples,
        //      print, change and set outgoing samples
        MY_TYPE *inBuffer = (MY_TYPE *)inputBuffer; // sint
        MY_TYPE *outBuffer = (MY_TYPE *)outputBuffer;
        double tmp[nBufferFrames * gChannels];
        for (unsigned int i = 0; i < nBufferFrames; i++) {
            for (unsigned int j = 0; j < gChannels; j++) {
                unsigned int index = i * gChannels + j;
                tmp[index] = *inBuffer++ / SCALE;
                //      std::cout << "frame = " << frameCounter + i << "\tchannel = " <<
                //      j
                //                << "\tval = " << tmp[i] << std::endl;
                tmp[index] *= 3.0;
                tmp[index] = (j == 1) ? tmp[index - 1] : tmp[index];
                *outBuffer++ = (MY_TYPE)(tmp[index] * SCALE);
            }
        }
    }

    gLocalStreamTime = streamTime;
    frameCounter += nBufferFrames;
    if (checkCount && (frameCounter >= nFrames))
        return callbackReturnValue;
    return 0;
}

void Duplex::resetGlobals() {
    gStreamTimePrintIncrement = 1.0;
    gStreamTimePrintTime = 1.0;
    gLocalStreamTime = 0.0;
    gChannels = m_channels;
    frameCounter = 0;
    checkCount = false;
    nFrames = 0;
}

Duplex::Duplex(QObject *parent) : QThread(parent) {
    // default values
    unsigned int bufferFrames = 32; // depends on what the operating system can do
    unsigned int device = 0;        // use the default audio interface
    m_channels = 2;
    m_fs = 48000;
    m_bufferBytes = 0;
    m_oDevice = m_iDevice = device;
    m_iOffset = m_oOffset = 0;
    resetGlobals();

    m_adac = new RtAudio();
    if (m_adac->getDeviceCount() < 1) {
        std::cout << "\nNo audio devices found!\n";
        exit(1);
    } else {

        std::cout << "run with PIPEWIRE_LATENCY=32/48000 ./hacktrip\n";
        std::cout << "using default audio interface device\n";
        std::cout << m_adac->getDeviceInfo(device).name
                  << "\tfor input and output\n";
        std::cout << "\tIf another is needed, either change your settings\n";
        std::cout << "\tor the choice in the code\n";
        // Let RtAudio print messages to stderr.
        m_adac->showWarnings(true);
    }

    RtAudio::StreamOptions options;
    options.flags =
            RTAUDIO_SCHEDULE_REALTIME; // use realtime priority if it's available

    // copy all setup into all stream info
    m_iParams.deviceId = m_iDevice;
    m_iParams.nChannels = m_channels;
    m_iParams.firstChannel = m_iOffset;
    m_oParams.deviceId = m_oDevice;
    m_oParams.nChannels = m_channels;
    m_oParams.firstChannel = m_oOffset;

    if (device == 0) {
        m_iParams.deviceId = m_adac->getDefaultInputDevice();
        m_oParams.deviceId = m_adac->getDefaultOutputDevice();
    } else {
        std::cout << "only the default device can be chosen\n";
    }

    m_bufferBytes = bufferFrames * m_channels *
            sizeof(MY_TYPE); // to be passed to callback function
    m_adac->openStream(&m_oParams, &m_iParams, FORMAT, m_fs, &bufferFrames,
                       &gAudio_callback, (void *)&m_bufferBytes, &options);
    if (m_adac->isStreamOpen() == false) {
        std::cout << "\nCouldn't open audio device streams!\n";
        exit(1);
    } else {
        std::cout << "\nStream latency = " << m_adac->getStreamLatency()
                  << " frames" << std::endl;
    }
    //    char input;
    std::cout << "\nRunning ... press <enter> to quit (buffer frames = "
              << bufferFrames << ").\n";
    //    std::cin.get(input);

    // cleanup:
}

void Duplex::run() {
    m_adac->startStream();
    gLocalStreamTime = 0.0;
    while (gLocalStreamTime < 30.0) {
        msleep(100);
    };
    if (m_adac->isStreamOpen())
        m_adac->closeStream();
    std::cout << "\nstream closed" << std::endl;
}
