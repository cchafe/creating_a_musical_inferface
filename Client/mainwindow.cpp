#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ht = nullptr;
}

MainWindow::~MainWindow()
{
    if (ht != nullptr) delete ht;
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    if (ht != nullptr) delete ht;
    ht = new HackTrip();
    ht->connect(); // grab the next free client slot from server pool
}

#include <QTimer>
void MainWindow::on_runButton_clicked()
{
    ht->run();
    UDP * udp = ht->getUdp();
    QTimer *timer = new QTimer(this);
       connect(timer, &QTimer::timeout, udp, &UDP::sendDummyData);
       timer->start(5);
}

void MainWindow::on_stopButton_clicked()
{
    if (ht != nullptr) {
        ht->stop();
        delete ht;
        ht = nullptr;
    }
    this->close();
}

