#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ht = new HackTrip();
}

MainWindow::~MainWindow()
{
    delete ht;
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    ht->mTcpClient.connectToServer();
}

void MainWindow::on_runButton_clicked()
{
    ht->run();
}

void MainWindow::on_stopButton_clicked()
{
    ht->stop();
    this->close();
}

