#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTcpSocket>
#include <QMainWindow>
#include "tcp.h"
#include "udp.h"

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

    void on_quitButton_clicked();

    void on_sendAutioButton_clicked();

    void on_stopSendButton_clicked();

private:
    Ui::MainWindow *ui;
    TCP mTcpClient;
    UDP mUdpSend;
//    QString mRemoteClientName;  ///< Remote JackAudio Client Name for hub client mode
//    int mReceiverBindPort;  ///< Incoming (receiving) port for local machine
//    QString mPeerAddress;
};
#endif // MAINWINDOW_H
