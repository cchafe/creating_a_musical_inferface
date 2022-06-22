#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_connectButton_clicked()
{
//    hacktrip->setup();
}

void MainWindow::on_sendAutioButton_clicked()
{
//    hacktrip->start();
}

void MainWindow::on_quitButton_clicked()
{
//    hacktrip->stop();
    qApp->quit();
}
