#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtEndian>
#include <QHostAddress>
#include <QHostInfo>
#include <iostream>
constexpr int gMaxRemoteNameLength           = 64;
constexpr int gDefaultPort  = 4464;  ///< Default JackTrip Port
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


            std::cout << "run a test with : nc -l 5000 " << std::endl;

    // Send Client Port Number to Server
    // ---------------------------------
//    char port_buf[sizeof(qint32) + gMaxRemoteNameLength];
//    mReceiverBindPort = gDefaultPort;
//    qToLittleEndian<qint32>(mReceiverBindPort, port_buf);
//    mRemoteClientName = "localhost";
//    memset(port_buf + sizeof(qint32), 0, gMaxRemoteNameLength);
//    if (!mRemoteClientName.isEmpty()) {
//        // If our remote client name is set, send it too.
//        QByteArray name = mRemoteClientName.toUtf8();
//        // Find a clean place to truncate if we're over length.
//        // (Make sure we're not in the middle of a multi-byte characetr.)
//        int length = name.length();
//        // Need to take the final null terminator into account here.
//        if (length > gMaxRemoteNameLength - 1) {
//            length = gMaxRemoteNameLength - 1;
//            while ((length > 0) && ((name.at(length) & 0xc0) == 0x80)) {
//                // We're in the middle of a multi-byte character. Work back.
//                length--;
//            }
//        }
//        name.truncate(length);
//        memcpy(port_buf + sizeof(qint32), name.data(), length + 1);
//    }
//    QHostAddress serverHostAddress;
//    mPeerAddress = "localhost";
//    if (!serverHostAddress.setAddress(mPeerAddress)) {
//        QHostInfo info = QHostInfo::fromName(mPeerAddress);
//        if (!info.addresses().isEmpty()) {
//            // use the first IP address
//            serverHostAddress = info.addresses().constFirst();
//        }
//    }
//    mTcpClient.connectToHost(serverHostAddress, gDefaultPort);
//    mTcpClient.write(port_buf, sizeof(port_buf));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    mTcpClient.connectToHost();
    mTcpClient.sendToHost();
}


void MainWindow::on_quithButton_clicked()
{
    qApp->quit();
}


void MainWindow::on_sendAutioButton_clicked()
{
    mUdpSend.start();
}

