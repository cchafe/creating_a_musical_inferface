#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtEndian>
#include <QHostAddress>
#include <QHostInfo>
#include <iostream>
constexpr int gMaxRemoteNameLength           = 64;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::cout << "run a test with : nc -l 4464 " << std::endl;
    mUdpRcv.setRcv();
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

void MainWindow::on_sendAutioButton_clicked()
{
    mUdpRcv.start();
    mUdpSend.start();
}

void MainWindow::on_quitButton_clicked()
{
    mUdpRcv.stop();
    mUdpRcv.wait();
    mUdpSend.stop();
    mUdpSend.wait();
    qApp->quit();
}

