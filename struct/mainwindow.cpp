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
    ht->contactServer();
}

void MainWindow::on_sendAutioButton_clicked()
{
    ht->start();
}

void MainWindow::on_quitButton_clicked()
{
    ht->stop();
    this->close();
}
