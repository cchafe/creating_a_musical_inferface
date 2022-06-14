#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_duplexButton_clicked() {
  audio = new Duplex(); // create a new rtaudio thread
  audio->start();       // run it
}

void MainWindow::on_quitButton_clicked() { qApp->quit(); }
