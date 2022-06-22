#ifndef DUPLEX_H
#define DUPLEX_H

#include <QObject>
#include <QThread>
#include <rtaudio/RtAudio.h>
#include "udp.h"

/*
 * encapsulate rtaudio and inherit
 * multi-threading from qthread
 */

class Duplex : public QThread {
public:
  explicit Duplex(QObject *parent = nullptr);
  void stop();
void setNetworkRcv(UDP *rcv);
private:
  // these are identical to the rtaudio/tests/Duplex.cpp example
  // except with m_ prepended
  unsigned int m_channels;
  unsigned int m_fs;
  unsigned int m_bufferBytes;
  unsigned int m_oDevice, m_iDevice, m_iOffset, m_oOffset;
  RtAudio *m_adac;
  RtAudio::StreamParameters m_iParams, m_oParams;
  static int wrapperProcessCallback(void *outputBuffer, void *inputBuffer,
                                    unsigned int nBufferFrames, double streamTime,
                                    RtAudioStreamStatus status, void *arg);
  int networkAudio_callback(void *outputBuffer, void *inputBuffer,
                      unsigned int nBufferFrames, double streamTime,
                      RtAudioStreamStatus status, void *bytesInfoFromStreamOpen);
  void resetGlobals();
  // QThreads have a run method which starts a separate thread
  // when start() is called
  virtual void run();
  bool mStop;
  UDP *mUdpRcv;
};

#endif // DUPLEX_H
