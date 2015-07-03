#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QImageReader>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

void MainWindow::init()
{
    // 字体大小
    font = QFont( "Arial", 10);

    // 状态
    isOn = false;
    ui->led_label->setPixmap(QPixmap(":/led/off"));

    // 网址
    netAddrLabel = new QLabel;
    netAddrLabel->setMinimumSize(20, 14); // 设置标签最小大小
    netAddrLabel->setText("www.daxia.com");
    netAddrLabel->setAlignment(Qt::AlignHCenter);
    netAddrLabel->setFont(font);

    // 发送数量
    sendLabel = new QLabel;
    sendLabel->setMinimumSize(100, 14); // 设置标签最小大小
    sendLabel->setText("S:0");
    sendLabel->setAlignment(Qt::AlignLeft);
    sendLabel->setFont(font);

    // 接收数量
    receiveLabel = new QLabel;
    receiveLabel->setMinimumSize(100, 14); // 设置标签最小大小
    receiveLabel->setText("R:0");
    receiveLabel->setAlignment(Qt::AlignLeft);
    receiveLabel->setFont(font);

    // 状态栏
    statusLabel = new QLabel;
    statusLabel->setMinimumSize(100, 14); // 设置标签最小大小
    statusLabel->setText("ttyUSB0 已关闭 115200bps,8,1 无检验 无流控");
    statusLabel->setAlignment(Qt::AlignHCenter);
    statusLabel->setFont(font);

    ui->statusBar->addWidget(netAddrLabel);
    ui->statusBar->addWidget(receiveLabel);
    ui->statusBar->addWidget(sendLabel);
    ui->statusBar->addWidget(statusLabel);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openserial_pushButton_pressed()
{
    if(isOn) {
        isOn = false;
        ui->led_label->setPixmap(QPixmap(":/led/off"));
    } else {
        isOn = true;
        ui->led_label->setPixmap(QPixmap(":/led/on"));
    }
}
