#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtEndian>
#include <QHostAddress>
#include <QHostInfo>
#include <iostream>
// PIPEWIRE_LATENCY=32/48000 ./networkSockets

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //    std::cout << "run a test with : nc -l 4464 " << std::endl;
    hacktrip = new HackTrip();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete hacktrip;
}

void MainWindow::on_connectButton_clicked()
{
    hacktrip->setup();
}

void MainWindow::on_sendAutioButton_clicked()
{
    hacktrip->start();
}

void MainWindow::on_quitButton_clicked()
{
    hacktrip->stop();
    qApp->quit();
}

