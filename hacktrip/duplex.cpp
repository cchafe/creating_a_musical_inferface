#include "duplex.h"
typedef signed short MY_TYPE;
#define FORMAT RTAUDIO_SINT16

double streamTimePrintIncrement = 1.0; // seconds
double streamTimePrintTime = 1.0; // seconds
double localStreamTime = 0.0; // seconds

unsigned int channelsGlobal;
#define SCALE  32767.0
#define BASE_RATE 0.05
unsigned int frameCounter = 0;
bool checkCount = false;
unsigned int nFrames = 0;
const unsigned int callbackReturnValue = 1; // 1 = stop and drain, 2 = abort

int inout( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
           double streamTime, RtAudioStreamStatus status, void *data )
{
    if ( status ) std::cout << "Stream over/underflow detected." << std::endl;

    if ( streamTime >= streamTimePrintTime ) {
        std::cout << "streamTime = " << streamTime << "\t" <<
                     sizeof((unsigned int *) data) << std::endl;
        streamTimePrintTime += streamTimePrintIncrement;
    }

    unsigned int i, j;
    MY_TYPE *inBuffer = (MY_TYPE *) inputBuffer;
    double *inValues = (double *) data;
    for ( i=0; i<nBufferFrames; i++ ) {
      for ( j=0; j<channelsGlobal; j++ ) {
//        inValues[j] = *inBuffer++ / SCALE;
//        std::cout << "frame = " << frameCounter+i << "\tchannel = " << j << "\tval = "  << inValues[j] << std::endl;
      }
    }

    unsigned int *bytes = (unsigned int *) data;
    memcpy( outputBuffer, inputBuffer, *bytes );

//    extern unsigned int channelsGlobal;
//    MY_TYPE *buffer = (MY_TYPE *) outputBuffer;
//    double *lastValues = (double *) data;
//    for ( i=0; i<nBufferFrames; i++ ) {
//      for ( j=0; j<channelsGlobal; j++ ) {
////          std::cout << "i\t" << i << "\tj\t" << j << "\tval=\t"  << lastValues[j] << std::endl;
//        *buffer++ = (MY_TYPE) (lastValues[j] * SCALE * 0.5);
//        lastValues[j] += BASE_RATE * (j+1+(j*0.1));
//        if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
//      }
//    }
    localStreamTime = streamTime;
    frameCounter += nBufferFrames;
    if ( checkCount && ( frameCounter >= nFrames ) ) return callbackReturnValue;
    return 0;
}

int saw( void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *data )
{
  unsigned int i, j;
  extern unsigned int channelsGlobal;
  MY_TYPE *buffer = (MY_TYPE *) outputBuffer;
  double *lastValues = (double *) data;

  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;

  if ( streamTime >= streamTimePrintTime ) {
    std::cout << "streamTime = " << streamTime << std::endl;
    streamTimePrintTime += streamTimePrintIncrement;
  }

  for ( i=0; i<nBufferFrames; i++ ) {
    for ( j=0; j<channelsGlobal; j++ ) {
      *buffer++ = (MY_TYPE) (lastValues[j] * SCALE * 0.5);
      lastValues[j] += BASE_RATE * (j+1+(j*0.1));
      if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
    }
  }
  localStreamTime = streamTime;

  frameCounter += nBufferFrames;
  if ( checkCount && ( frameCounter >= nFrames ) ) return callbackReturnValue;
  return 0;
}


Duplex::Duplex(QObject *parent) : QThread(parent)
{
    channels = 2;
    channelsGlobal = channels;
    fs = 48000;
    bufferBytes = 0;
    oDevice = 0;
    iDevice = 0;
    iOffset = 0;
    oOffset = 0;

    adac = new RtAudio();
    if ( adac->getDeviceCount() < 1 ) {
        std::cout << "\nNo audio devices found!\n";
        exit( 1 );
    }

    // Let RtAudio print messages to stderr.
    adac->showWarnings( true );

    // Set the same number of channels for both input and output.
    unsigned int bufferFrames = 128;
    iParams.deviceId = iDevice;
    iParams.nChannels = channels;
    iParams.firstChannel = iOffset;
    oParams.deviceId = oDevice;
    oParams.nChannels = channels;
    oParams.firstChannel = oOffset;

    if ( iDevice == 0 )
        iParams.deviceId = adac->getDefaultInputDevice();
    if ( oDevice == 0 )
        oParams.deviceId = adac->getDefaultOutputDevice();

    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_SCHEDULE_REALTIME;

    if (true) {
    bufferBytes = bufferFrames * channels * sizeof( MY_TYPE );
    adac->openStream( &oParams, &iParams, FORMAT, fs,
                      &bufferFrames, &inout, (void *)&bufferBytes,
                      &options );
} else {
    double *data = (double *) calloc( channels, sizeof( double ) );
    adac->openStream( &oParams, NULL, FORMAT, fs, &bufferFrames, &saw, (void *)data, &options );
}
        //    if ( adac->isStreamOpen() == false ) goto cleanup;
    std::cout << "\nStream latency = " << adac->getStreamLatency() << " frames" << std::endl;


    //    char input;
    std::cout << "\nRunning ... press <enter> to quit (buffer frames = " << bufferFrames << ").\n";
    //    std::cin.get(input);

    //cleanup:

}

void Duplex::run() {
    adac->startStream();
    localStreamTime = 0.0;
    while ( localStreamTime < 3.0 ) {
        msleep(100);
    };
    if ( adac->isStreamOpen() ) adac->closeStream();
    std::cout << "\nstream closed" << std::endl;
}
