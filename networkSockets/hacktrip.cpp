#include "hacktrip.h"

HackTrip::HackTrip()
{
    mUdpRcv.setRcv();
    audio = new Duplex(); // create a new rtaudio thread
    audio->setNetworkRcv(&mUdpRcv);
}

void HackTrip::setup()
{
    mTcpClient.connectToHost();
    mTcpClient.sendToHost();
}

void HackTrip::start()
{
    mUdpRcv.start();
    mUdpSend.start();
    audio->start();
}

void HackTrip::stop()
{
    mUdpRcv.stop();
    mUdpRcv.wait();
    mUdpSend.stop();
    mUdpSend.wait();
    audio->stop();
    audio->wait();
}
