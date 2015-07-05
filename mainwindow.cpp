#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QImageReader>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QtSerialPort/QSerialPortInfo>

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
    struct tm t;
    memset(&t, 0, sizeof(tm));
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

    intValidator = new QIntValidator(0, 4000000, this);

    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

//    connect(ui->applyButton, SIGNAL(clicked()),
//            this, SLOT(apply()));
    connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(serialPortChanged()));
    connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomBaudRatePolicy(int)));
    connect(ui->dataBitsBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->stopBitsBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->parityBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->flowControlBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged()));

    // 字体大小
    font = QFont( "Arial", 10);

    // 只能输入数字的正则
    QValidator *numberOnlyValidator; //检验器，只允许输入数字
    QRegExp regx("[0-9]+$"); //设置输入范围0～9
    numberOnlyValidator = new QRegExpValidator(regx, mTimerSendLineEdit);

    // 设置窗口标题
    QDateTime dt = QDateTime::fromTime_t( (uint)getDateFromMacro(__DATE__));
    this->setWindowTitle("sscom for linux 0.1, 作者:kangear, " + dt.toString("yyyy/MM"));

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

    //! [1]
    serial = new QSerialPort(this);


    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

//! [2]
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
//! [2]
    connect(mSendButton, SIGNAL(released()), this, SLOT(writeData()));
//! [3]

    fillPortsParameters();
    fillPortsInfo();
    updateSettings();
}

/**
 * @brief MainWindow::currentIndexChanged
 * @param idx
 * handle StopBitsBox dataBitsBox stopBitsBox parityBox flowControlBox
 */
void MainWindow::serialPortChanged()
{
    if(serial->isOpen())
        closeSerialPort();
    updateSettings();
}

/**
 * @brief MainWindow::currentIndexChanged
 * @param idx
 * handle StopBitsBox dataBitsBox stopBitsBox parityBox flowControlBox
 */
void MainWindow::currentIndexChanged()
{
    updateSettings();
    if(serial->isOpen())
        setParameter(serial, currentSettings);
}

void MainWindow::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
    updateSettings();
    if(serial->isOpen())
        setParameter(serial, currentSettings);
}

void MainWindow::fillPortsParameters()
{
    // fill baud rate (is not the entire list of available values,
    // desired values??, add your independently)
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(QStringLiteral("Custom"));

    // fill data bits
    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);

    // fill parity
    ui->parityBox->addItem(QStringLiteral("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(QStringLiteral("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(QStringLiteral("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(QStringLiteral("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(QStringLiteral("Space"), QSerialPort::SpaceParity);

    // fill stop bits
    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    // fill flow control
    ui->flowControlBox->addItem(QStringLiteral("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(QStringLiteral("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(QStringLiteral("XON/XOFF"), QSerialPort::SoftwareControl);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openserial_pushButton_pressed()
{
    if(serial->isOpen())
        closeSerialPort();
    else
        openSerialPort();
    updateSettings();
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

void MainWindow::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    static const QString blankString = QObject::tr("N/A");
    QString description;
    QString manufacturer;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }
}

void MainWindow::updateSettings()
{
    if(serial->isOpen())
        currentSettings.stringStatus = "已打开";
    else
        currentSettings.stringStatus = "已关闭";

    currentSettings.name = ui->serialPortInfoListBox->currentText();

    // Baud Rate
    if (ui->baudRateBox->currentIndex() == 4) {
        // custom baud rate
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    } else {
        // standard baud rate
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    // Data bits
    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    // Parity
    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    // Stop bits
    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    // Flow control
    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

    // Additional options
    currentSettings.localEchoEnabled = false; //ui->localEchoCheckBox->isChecked();

    // new line
    currentSettings.sendNewLineEnabled = ui->newLineCheckBox->isChecked();

    Settings p = currentSettings;
    mStatusLabel->setText(tr("%1 %2 %3bps,%4,%5, %6, %7")
                          .arg(p.name).arg(p.stringStatus).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringStopBits).arg(p.stringParity).arg(p.stringFlowControl));
}


bool MainWindow::setParameter(QSerialPort *serial, Settings settings)
{
    bool ret;
    Settings p = settings;
    if (serial->setBaudRate(p.baudRate)
            && serial->setDataBits(p.dataBits)
            && serial->setParity(p.parity)
            && serial->setStopBits(p.stopBits)
            && serial->setFlowControl(p.flowControl)) {
        ret = true;
    } else
        ret = false;
    return ret;
}

//! [4]
bool MainWindow::openSerialPort()
{
    bool ret = false;
    Settings p = currentSettings;
    serial->setPortName(p.name);
    if (serial->open(QIODevice::ReadWrite)) {
        if (setParameter(serial, p)) {
            mLedLabel->setPixmap(QPixmap(":/led/on"));
            // 2.更新文字
            mOpenSerialButton->setText("关闭串口");
            // 3.使能发送按键
            mSendButton->setDisabled(false);
            // 4.使能发送文件按键
            mSendFileButton->setDisabled(false);
            ret = true;
        } else {
            serial->close();
            QMessageBox::critical(this, tr("Error"), serial->errorString());
            ui->statusBar->showMessage(tr("Open error"));
        }
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        ui->statusBar->showMessage(tr("Configure error"));
    }

    return ret;
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    mLedLabel->setPixmap(QPixmap(":/led/off"));
    // 2.更新文字
    mOpenSerialButton->setText("打开串口");
    // 3.使能发送按键
    mSendFileButton->setDisabled(true);
    // 4.使能发送文件按键
    mSendButton->setDisabled(true);
    serial->close();
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData()
{
    QString text = ui->sendLineEdit->text();
    qDebug() << text;
    if(text.length() !=0 && currentSettings.sendNewLineEnabled)
        text += "\r\n";
    QByteArray data = text.toLatin1();
    serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    QString str = QString::fromLatin1(data.data());
    // 是否发送新行
//    if(str.length() !=0 && currentSettings.sendNewLineEnabled)
//        str += "\r\n";
//    qDebug() << str;
    QString a = ui->receive_textBrowser->toPlainText();
    ui->receive_textBrowser->setText(a + str);

    // 将光标移动到最后位置
    QTextCursor tmpCursor = ui->receive_textBrowser->textCursor();
    tmpCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 4);
    ui->receive_textBrowser->setTextCursor(tmpCursor);
}
//! [7]

//! [8]
//! 添加除错功能，解决强制拔出时程序崩溃。
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    QString errStr = "NO ERROR";
    switch(error) {
    case QSerialPort::DeviceNotFoundError:
        errStr = "DeviceNotFoundError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::PermissionError:
        errStr = "PermissionError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::OpenError:
        errStr = "OpenError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::ParityError:
        errStr = "ParityError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::FramingError:
        errStr = "FramingError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::BreakConditionError:
        errStr = "BreakConditionError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::WriteError:
        errStr = "WriteError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::ReadError:
        errStr = "ReadError";
        closeSerialPort();
        break;
    case QSerialPort::ResourceError:
        errStr = "ResourceError";
        qDebug() << errStr;
        on_openserial_pushButton_pressed();
        break;
    case QSerialPort::UnsupportedOperationError:
        errStr = "UnsupportedOperationError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::UnknownError:
        // do nothing!!!
        errStr = "UnknownError";
        qDebug() << errStr;
//        on_openserial_pushButton_pressed();
        break;
    case QSerialPort::TimeoutError:
        errStr = "TimeoutError";
        qDebug() << errStr;
        closeSerialPort();
        break;
    case QSerialPort::NotOpenError:
        // do nothing.
        errStr = "NotOpenError";
        qDebug() << errStr;
//        closeSerialPort();
        break;
    default:
        qDebug() << errStr;
        break;
    }
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        //closeSerialPort();
    }
}
//! [8]

//void MainWindow::initActionsConnections()
//{
//    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
//    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
//    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
//    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));
//    connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear()));
//    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
//    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
//}

/**
 * @brief MainWindow::on_clear_pushButton_released
 * 清除接收区内容
 */
void MainWindow::on_clear_pushButton_released()
{
    ui->receive_textBrowser->clear();
}
