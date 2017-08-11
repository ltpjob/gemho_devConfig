#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbScan_clicked();
    void processPendingDatagrams();

private:
    Ui::MainWindow *ui;
    QUdpSocket *m_udpSocket;
};

#endif // MAINWINDOW_H
