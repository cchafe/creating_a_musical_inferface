#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTcpSocket>
#include <QMainWindow>
#include "tcp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();

private:
    Ui::MainWindow *ui;
    TCP mTcpClient;
//    QString mRemoteClientName;  ///< Remote JackAudio Client Name for hub client mode
//    int mReceiverBindPort;  ///< Incoming (receiving) port for local machine
//    QString mPeerAddress;
};
#endif // MAINWINDOW_H
