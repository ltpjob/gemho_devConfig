#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QByteArray>
#include "crc32.h"

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

#pragma pack()

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_udpSocket = new QUdpSocket(this);

    m_udpSocket->bind(45454, QUdpSocket::ShareAddress);

    connect(m_udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processPendingDatagrams()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        upComDataHead_udp *phead;

        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());

        if(datagram.size() < sizeof(upComDataHead_udp))
            continue;

        phead = (upComDataHead_udp *)datagram.data();
        if(get_crc32(0, (uint8_t *)&phead->hdt, sizeof(phead->hdt)) == phead->crc32)
        {
            if(phead->hdt.start[0]==0x55&&phead->hdt.start[1]==0xaa
                    &&phead->hdt.size==datagram.size()-sizeof(*phead))
            {
                if(phead->hdt.type==4)
                {
                    devInfo *pdi = (devInfo *)(datagram.data()+sizeof(*phead));
                    if(get_crc32(0, (uint8_t *)pdi, sizeof(*pdi)-sizeof(pdi->crc32)) == pdi->crc32)
                    {
                        qDebug("!!!!!!!!!!!");
                    }

                }
            }
        }
    }
}

void MainWindow::on_pbScan_clicked()
{
    upComDataHead_udp head;

    head.hdt.start[0] = 0x55;
    head.hdt.start[1] = 0xaa;
    head.hdt.type = 3;
    head.hdt.size = 0;
    head.crc32 = get_crc32(0, (uint8_t *)&head.hdt, sizeof(head.hdt));

    m_udpSocket->writeDatagram((char *)&head, sizeof(head), QHostAddress::Broadcast, 5768);
}
