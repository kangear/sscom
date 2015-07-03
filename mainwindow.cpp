#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QImageReader>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

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
    ui->openfile_lineEdit->setText(tr("文件名"));
    mFilePath = ui->openfile_lineEdit->text();

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

void MainWindow::on_openfile_pushButton_released()
{
    // 判断目前是否已经打开过的路径
    if((mFilePath == nullptr || mFilePath.length() == 0) && !QDir(mFilePath).exists()) {
        // 如果没有打开过，或者目录不存在，那么将打开应用所在目录
        mFilePath = ".";
    }

    // 打开新文件
    mFilePath = QFileDialog::getOpenFileName(this, tr("打开"), mFilePath, tr("All Files(*.* *.**)"));
    if(mFilePath.length() != 0) {
        ui->openfile_lineEdit->setText(mFilePath);
    }

}
