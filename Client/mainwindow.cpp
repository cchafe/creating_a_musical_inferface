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

void MainWindow::on_runButton_clicked()
{
    if (ht != nullptr) delete ht;
    ht = new HackTrip();
    ht->run();
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

