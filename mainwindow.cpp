#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QImageReader>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

/**
 * @brief getDateFromMacro
 * @param time __DATE__
 * @return
 */
static time_t getDateFromMacro(char const *time) {
    char s_month[5];
    int month, day, year;
    struct tm t = {0};
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    sscanf(time, "%s %d %d", s_month, &day, &year);

    month = (strstr(month_names, s_month)-month_names)/3;

    t.tm_mon = month;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}

void MainWindow::init()
{
    mStatusBar = ui->statusBar;
    mSendButton = ui->send_pushButton;
    mSendFileButton = ui->sendfile_pushButton;
    mOpenSerialButton = ui->openserial_pushButton;
    mLedLabel = ui->led_label;
    mOpenFileLineEdit = ui->openfile_lineEdit;
    mTimerSendLineEdit = ui->timer_lineEdit;

    // 字体大小
    font = QFont( "Arial", 10);

    // 只能输入数字的正则
    QValidator *numberOnlyValidator; //检验器，只允许输入数字
    QRegExp regx("[0-9]+$"); //设置输入范围0～9
    numberOnlyValidator = new QRegExpValidator(regx, mTimerSendLineEdit);

    // 设置窗口标题
    QDateTime dt = QDateTime::fromTime_t( (uint)getDateFromMacro(__DATE__));
    this->setWindowTitle("sscom for linux 作者:kangear" + tr("(") + dt.toString("yyyy/MM") + tr(")"));

    // 状态
    isOn = false;
    mLedLabel->setPixmap(QPixmap(":/led/off"));
    mOpenFileLineEdit->setText(tr("文件名"));
    mFilePath = ui->openfile_lineEdit->text();
    mTimerSendLineEdit->setValidator(numberOnlyValidator);

    // 网址
    mNetAddrLabel = new QLabel;
    mNetAddrLabel->setMinimumSize(20, 14); // 设置标签最小大小
    mNetAddrLabel->setText("www.daxia.com");
    mNetAddrLabel->setAlignment(Qt::AlignHCenter);
    mNetAddrLabel->setFont(font);

    // 发送数量
    mSendLabel = new QLabel;
    mSendLabel->setMinimumSize(60, 14); // 设置标签最小大小
    mSendLabel->setText("S:0");
    mSendLabel->setAlignment(Qt::AlignLeft);
    mSendLabel->setFont(font);

    // 接收数量
    mReceiveLabel = new QLabel;
    mReceiveLabel->setMinimumSize(60, 14); // 设置标签最小大小
    mReceiveLabel->setText("R:0");
    mReceiveLabel->setAlignment(Qt::AlignLeft);
    mReceiveLabel->setFont(font);

    // 状态栏
    mStatusLabel = new QLabel;
    mStatusLabel->setMinimumSize(100, 14); // 设置标签最小大小
    mStatusLabel->setText("ttyUSB0 已关闭 115200bps,8,1 无检验 无流控");
    mStatusLabel->setAlignment(Qt::AlignHCenter);
    mStatusLabel->setFont(font);

    mStatusBar->addWidget(mNetAddrLabel);
    mStatusBar->addWidget(mReceiveLabel);
    mStatusBar->addWidget(mSendLabel);
    mStatusBar->addWidget(mStatusLabel);

    // 2.更新文字
    mOpenSerialButton->setText("打开串口");
    // 3.使能发送按键
    mSendButton->setDisabled(true);
    // 4.使能发送文件按键
    mSendFileButton->setDisabled(true);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openserial_pushButton_pressed()
{
    if(isOn) {
        // 1.更新状态
        isOn = false;
        mLedLabel->setPixmap(QPixmap(":/led/off"));
        mStatusLabel->setText("ttyUSB0 已关闭 115200bps,8,1 无检验 无流控");
        // 2.更新文字
        mOpenSerialButton->setText("打开串口");
        // 3.使能发送按键
        mSendFileButton->setDisabled(true);
        // 4.使能发送文件按键
        mSendButton->setDisabled(true);
    } else {
        // 1.更新状态
        isOn = true;
        mLedLabel->setPixmap(QPixmap(":/led/on"));
        mStatusLabel->setText("ttyUSB0 已打开 115200bps,8,1 无检验 无流控");
        // 2.更新文字
        mOpenSerialButton->setText("关闭串口");
        // 3.使能发送按键
        mSendButton->setDisabled(false);
        // 4.使能发送文件按键
        mSendFileButton->setDisabled(false);
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
        mOpenFileLineEdit->setText(mFilePath);
    }

}
