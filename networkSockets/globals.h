#ifndef GLOBALS_H
#define GLOBALS_H
#include <QString>
//const QString gServer = "jackloop256.stanford.edu";
//const QString gServer = "cmn55.stanford.edu";
const QString gServer = "cmn9.stanford.edu";
//const QString gServer = "171.64.197.158";
//const QString gServer = "localhost";
const QString gPort = "4464";
const int gAudioPort = 4465;
const int gFPP = 256;
const int gSocketWaitMs = 1000;
const int gChannels = 2;
#define SCALE                                                                  \
    32767.0 // audio samples for processing are doubles, so this is the conversion
typedef signed short MY_TYPE; // audio interface data is 16bit ints

#endif // GLOBALS_H
