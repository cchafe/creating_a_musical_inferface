#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "duplex.h" // an rtaudio thread
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_duplexButton_clicked();

  void on_quitButton_clicked();

private:
  Ui::MainWindow *ui;
  Duplex *audio;
};
#endif // MAINWINDOW_H
