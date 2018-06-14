#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QByteArray>
#include "crc32.h"

#define         GNSS_DATA               1
#define         GNSS_HEARTBEAT          2
#define         GNSS_ENUM               3
#define         GNSS_REPORTENUM         4
#define         GNSS_SETNETINFO         5



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize( this->width (),this->height ());

    m_udpSocket = new QUdpSocket(this);

    m_udpSocket->bind(36667, QUdpSocket::ShareAddress);

    connect(m_udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));

    m_id_list = findChild<QListWidget *>("listWidget_device");
    m_le_id = findChild<QLineEdit *>("lineEdit_id");
    m_le_ip = findChild<QIPLineEdit *>("lineEdit_ip");
    m_le_mask = findChild<QIPLineEdit *>("lineEdit_mask");
    m_le_gateway = findChild<QIPLineEdit *>("lineEdit_gateway");
    m_le_dns = findChild<QIPLineEdit *>("lineEdit_dns");
    m_le_mac0 = findChild<QLineEdit *>("lineEdit_mac0");
    m_le_mac1 = findChild<QLineEdit *>("lineEdit_mac1");
    m_le_mac2 = findChild<QLineEdit *>("lineEdit_mac2");
    m_le_mac3 = findChild<QLineEdit *>("lineEdit_mac3");
    m_le_mac4 = findChild<QLineEdit *>("lineEdit_mac4");
    m_le_mac5 = findChild<QLineEdit *>("lineEdit_mac5");

    connect(&m_gnssclinet, SIGNAL(readyRead()),
            this, SLOT(socket_Read_Data()));
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
                        NetInfo *winfo = &pdi->netinfo;
                        qDebug("%08X %08X %08X", pdi->cpuid[2], pdi->cpuid[1], pdi->cpuid[0]);
                        qDebug("MAC: %02X:%02X:%02X:%02X:%02X:%02X",winfo->mac[0],winfo->mac[1],winfo->mac[2],
                               winfo->mac[3],winfo->mac[4],winfo->mac[5]);
                        qDebug("SIP: %d.%d.%d.%d", winfo->ip[0],winfo->ip[1],winfo->ip[2],winfo->ip[3]);
                        qDebug("GAR: %d.%d.%d.%d", winfo->gw[0],winfo->gw[1],winfo->gw[2],winfo->gw[3]);
                        qDebug("SUB: %d.%d.%d.%d", winfo->sn[0],winfo->sn[1],winfo->sn[2],winfo->sn[3]);
                        qDebug("DNS: %d.%d.%d.%d", winfo->dns[0],winfo->dns[1],winfo->dns[2],winfo->dns[3]);
                        qDebug("======================");

                        QString devId = "";
                        devId.sprintf("%08X%08X%08X", pdi->cpuid[2], pdi->cpuid[1], pdi->cpuid[0]);

                        if(m_devMap.find(devId) == m_devMap.end())
                            m_id_list->addItem(devId);
                            m_devMap[devId] = *pdi;
//                        for(int i=0; i<20; i++)
//                        {
//                            devId[0] = 'A'+i;
//                            qDebug(devId.toStdString().c_str());
//                            m_id_list->addItem(devId);
//                            m_devMap[devId] = *pdi;
//                        }
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
    m_id_list->clear();
    m_devMap.clear();
    m_le_id->setText("");
    m_le_ip->clearText();
    m_le_mask->clearText();
    m_le_gateway->clearText();
    m_le_dns->clearText();
    m_le_mac0->setText("");
    m_le_mac1->setText("");
    m_le_mac2->setText("");
    m_le_mac3->setText("");
    m_le_mac4->setText("");
    m_le_mac5->setText("");

    m_gnssclinet.close();
    ui->textBrowser_gnss->clear();
}

void MainWindow::on_pushButton_clicked()
{
    upComDataHead_udp head;
    uint8_t buffer[128] = "";
    devInfo di;
    int size;

    QString id;
    QStringList ip;
    QStringList mask;
    QStringList gateway;
    QStringList dns;
    QString mac0, mac1, mac2, mac3, mac4, mac5;

    id = m_le_id->text();
    if(m_devMap.find(id) == m_devMap.end())
        return;

    ip = m_le_ip->text();
    mask = m_le_mask->text();
    gateway = m_le_gateway->text();
    dns = m_le_dns->text();

    mac0 = m_le_mac0->text();
    mac1 = m_le_mac1->text();
    mac2 = m_le_mac2->text();
    mac3 = m_le_mac3->text();
    mac4 = m_le_mac4->text();
    mac5 = m_le_mac5->text();

    di = m_devMap[id];
    di.netinfo.mac[0] = mac0.toUInt(0, 16);
    di.netinfo.mac[1] = mac1.toUInt(0, 16);
    di.netinfo.mac[2] = mac2.toUInt(0, 16);
    di.netinfo.mac[3] = mac3.toUInt(0, 16);
    di.netinfo.mac[4] = mac4.toUInt(0, 16);
    di.netinfo.mac[5] = mac5.toUInt(0, 16);

    di.netinfo.ip[0] = ip[0].toUInt();
    di.netinfo.ip[1] = ip[1].toUInt();
    di.netinfo.ip[2] = ip[2].toUInt();
    di.netinfo.ip[3] = ip[3].toUInt();

    di.netinfo.sn[0] = mask[0].toUInt();
    di.netinfo.sn[1] = mask[1].toUInt();
    di.netinfo.sn[2] = mask[2].toUInt();
    di.netinfo.sn[3] = mask[3].toUInt();

    di.netinfo.gw[0] = gateway[0].toUInt();
    di.netinfo.gw[1] = gateway[1].toUInt();
    di.netinfo.gw[2] = gateway[2].toUInt();
    di.netinfo.gw[3] = gateway[3].toUInt();

    di.netinfo.dns[0] = dns[0].toUInt();
    di.netinfo.dns[1] = dns[1].toUInt();
    di.netinfo.dns[2] = dns[2].toUInt();
    di.netinfo.dns[3] = dns[3].toUInt();

//    memset(&di.netinfo, 0, sizeof(di.netinfo));

//    di.cpuid[0] = 0x0675FF49;
//    di.cpuid[1] = 0x49547875;
//    di.cpuid[2] = 0x81241039;

//    di.cpuid[0] = 0x066EFF49;
//    di.cpuid[1] = 0x49547875;
//    di.cpuid[2] = 0x81233942;

    di.crc32 = get_crc32(0, (uint8_t *)&di, sizeof(di)-sizeof(di.crc32));

    head.hdt.start[0] = 0x55;
    head.hdt.start[1] = 0xaa;
    head.hdt.type = GNSS_SETNETINFO;
    head.hdt.size = sizeof(devInfo);;
    head.crc32 = get_crc32(0, (uint8_t *)&head.hdt, sizeof(head.hdt));

    memcpy(buffer, &head, sizeof(head));
    memcpy(buffer+sizeof(head), &di, sizeof(di));
    size = sizeof(head)+sizeof(di);

    m_gnssclinet.close();
    ui->textBrowser_gnss->clear();
    m_udpSocket->writeDatagram((char *)buffer, size, QHostAddress::Broadcast, 5768);
    m_devMap[id] = di;
}

void MainWindow::on_listWidget_device_itemDoubleClicked(QListWidgetItem *item)
{
    if(m_devMap.find(item->text()) == m_devMap.end())
        return;

    devInfo *pdi = (devInfo *)(&m_devMap[item->text()]);
    NetInfo *winfo = &pdi->netinfo;
    qDebug("%08X %08X %08X", pdi->cpuid[2], pdi->cpuid[1], pdi->cpuid[0]);
    qDebug("MAC: %02X:%02X:%02X:%02X:%02X:%02X",winfo->mac[0],winfo->mac[1],winfo->mac[2],
           winfo->mac[3],winfo->mac[4],winfo->mac[5]);
    qDebug("SIP: %d.%d.%d.%d", winfo->ip[0],winfo->ip[1],winfo->ip[2],winfo->ip[3]);
    qDebug("GAR: %d.%d.%d.%d", winfo->gw[0],winfo->gw[1],winfo->gw[2],winfo->gw[3]);
    qDebug("SUB: %d.%d.%d.%d", winfo->sn[0],winfo->sn[1],winfo->sn[2],winfo->sn[3]);
    qDebug("DNS: %d.%d.%d.%d", winfo->dns[0],winfo->dns[1],winfo->dns[2],winfo->dns[3]);
    qDebug("======================");

    QString id;
    QString ip;
    QString mask;
    QString gateway;
    QString dns;
    QString mac0, mac1, mac2, mac3, mac4, mac5;

    id.sprintf("%08X%08X%08X", pdi->cpuid[2], pdi->cpuid[1], pdi->cpuid[0]);
    m_le_id->setText(id);

    ip.sprintf("%d.%d.%d.%d", winfo->ip[0],winfo->ip[1],winfo->ip[2],winfo->ip[3]);
    m_le_ip->setText(ip);

    mask.sprintf("%d.%d.%d.%d", winfo->sn[0],winfo->sn[1],winfo->sn[2],winfo->sn[3]);
    m_le_mask->setText(mask);

    gateway.sprintf("%d.%d.%d.%d", winfo->gw[0],winfo->gw[1],winfo->gw[2],winfo->gw[3]);
    m_le_gateway->setText(gateway);

    dns.sprintf("%d.%d.%d.%d", winfo->dns[0],winfo->dns[1],winfo->dns[2],winfo->dns[3]);
    m_le_dns->setText(dns);

    mac0.sprintf("%02X", winfo->mac[0]);
    mac1.sprintf("%02X", winfo->mac[1]);
    mac2.sprintf("%02X", winfo->mac[2]);
    mac3.sprintf("%02X", winfo->mac[3]);
    mac4.sprintf("%02X", winfo->mac[4]);
    mac5.sprintf("%02X", winfo->mac[5]);
    m_le_mac0->setText(mac0);
    m_le_mac1->setText(mac1);
    m_le_mac2->setText(mac2);
    m_le_mac3->setText(mac3);
    m_le_mac4->setText(mac4);
    m_le_mac5->setText(mac5);

    m_gnssclinet.close();
    ui->textBrowser_gnss->clear();
    m_gnssclinet.connectToHost(QHostAddress(ip), 5566);

//    qDebug(m_le_ip->text()[0].toStdString().c_str());
}

void MainWindow::socket_Read_Data()
{
    QByteArray buffer;
    upComDataHead head;
    qint64 size = 0;

    buffer = m_gnssclinet.readAll();
    QString str = QString::fromLocal8Bit(buffer);
    ui->textBrowser_gnss->append(str);

    head.start[0] = 0x55;
    head.start[1] = 0xaa;
    head.type = 2;
    head.size = 0;
    size = sizeof(head);
    m_gnssclinet.write((char *)&head, size);
}

