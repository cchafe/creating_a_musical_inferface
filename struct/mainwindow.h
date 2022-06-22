#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hacktrip.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    HackTrip * ht;

private slots:
    void on_connectButton_clicked();

    void on_sendAutioButton_clicked();

    void on_quitButton_clicked();

};
#endif // MAINWINDOW_H
