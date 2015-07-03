#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    font = QFont( "Arial", 8);

    // 状态栏
    statusLabel = new QLabel;
    statusLabel->setMinimumSize(20, 14); // 设置标签最小大小
    statusLabel->setText("UDP通信停止");
    statusLabel->setAlignment(Qt::AlignHCenter);
    statusLabel->setFont(font);


    // 接收数量
    receiveLabel = new QLabel;
    receiveLabel->setMinimumSize(20, 14); // 设置标签最小大小
    receiveLabel->setText("UDP通信停止");
    receiveLabel->setAlignment(Qt::AlignHCenter);
    receiveLabel->setFont(font);




    // 发送数量
    sendLabel = new QLabel;
    sendLabel->setMinimumSize(20, 14); // 设置标签最小大小
    sendLabel->setText("UDP通信停止");
    sendLabel->setAlignment(Qt::AlignHCenter);
    sendLabel->setFont(font);

    ui->statusBar->addWidget(statusLabel);
    ui->statusBar->addWidget(receiveLabel);
    ui->statusBar->addWidget(sendLabel);
}

MainWindow::~MainWindow()
{
    delete ui;
}
