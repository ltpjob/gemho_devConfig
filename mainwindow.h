#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QListWidget>
#include "qiplineedit.h"
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

#pragma pack(1)
typedef struct tagupComDataHead
{
    quint8  start[2];
    qint16 type;
    qint32 size;
}upComDataHead;

typedef struct tagupComDataHead_udp
{
  upComDataHead hdt;
  uint32_t crc32;
}upComDataHead_udp;

typedef struct NetInfo_t
{
   uint8_t mac[6];  ///< Source Mac Address
   uint8_t ip[4];   ///< Source IP Address
   uint8_t sn[4];   ///< Subnet Mask
   uint8_t gw[4];   ///< Gateway IP Address
   uint8_t dns[4];  ///< DNS server IP Address
   uint8_t dhcp_mode;  ///< 1 - Static, 2 - DHCP
}NetInfo;

typedef struct tagdevInfo
{
  NetInfo netinfo;
  uint32_t cpuid[3];
  uint32_t crc32;
}devInfo;

typedef struct tagRetDevInfo
{
  upComDataHead_udp head;
  devInfo di;
}RetDevInfo;



#pragma pack()

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbScan_clicked();
    void processPendingDatagrams();

    void on_pushButton_clicked();

    void on_listWidget_device_itemDoubleClicked(QListWidgetItem *item);

    void socket_Read_Data();

private:
    Ui::MainWindow *ui;
    QUdpSocket *m_udpSocket;
    QListWidget *m_id_list;
    QMap<QString, devInfo> m_devMap;
    QLineEdit *m_le_id;
    QIPLineEdit *m_le_ip;
    QIPLineEdit *m_le_mask;
    QIPLineEdit *m_le_gateway;
    QIPLineEdit *m_le_dns;
    QLineEdit *m_le_mac0;
    QLineEdit *m_le_mac1;
    QLineEdit *m_le_mac2;
    QLineEdit *m_le_mac3;
    QLineEdit *m_le_mac4;
    QLineEdit *m_le_mac5;
    QTcpSocket m_gnssclinet;
};

#endif // MAINWINDOW_H
