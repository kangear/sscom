#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QImageReader>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>
#include <QTimer>

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

MainWindow::Settings MainWindow::doSettings(bool isWrite, Settings inSettings)
{
    Settings in =  inSettings;
    Settings out;
    QSettings settings("Yzs_think", "Application");
    if(isWrite) {
        settings.setValue("name", in.name);
        settings.setValue("baudRate", in.baudRate);
        settings.setValue("stringBaudRate", in.stringBaudRate);
        settings.setValue("dataBits", in.dataBits);
        settings.setValue("stringDataBits", in.stringDataBits);
        settings.setValue("parity", in.parity);
        settings.setValue("stringParity", in.stringParity);
        settings.setValue("stopBits", in.stopBits);
        settings.setValue("stringStopBits", in.stringStopBits);
        settings.setValue("flowControl", in.flowControl);
        settings.setValue("stringFlowControl", in.stringFlowControl);
        settings.setValue("sendNewLineEnabled", in.sendNewLineEnabled);
        settings.setValue("stringStatus", in.stringStatus);
        settings.setValue("isDtr", in.isDtr);
        settings.setValue("isRts", in.isRts);
        settings.setValue("isHexDisplay", in.isHexDisplay);
        settings.setValue("isHexSend", in.isHexSend);
        // 和Windows版本同步，不保存定时发送开关
        settings.setValue("isTimerSend", DEF_SETTINGS.isTimerSend);
        settings.setValue("timeTimerSend", in.timerLength);
        settings.setValue("sendCache", in.sendCache);
    } else {
        out.name               = settings.value("name", DEF_SETTINGS.name).toString();
        out.baudRate           = (QSerialPort::BaudRate)settings.value("baudRate", DEF_SETTINGS.baudRate).toInt();
        out.stringBaudRate     = settings.value("stringBaudRate", DEF_SETTINGS.stringBaudRate).toString();
        out.dataBits           = (QSerialPort::DataBits)settings.value("dataBits", DEF_SETTINGS.dataBits).toInt();
        out.stringDataBits     = settings.value("stringDataBits", DEF_SETTINGS.stringDataBits).toString();
        out.parity             = (QSerialPort::Parity)settings.value("parity", DEF_SETTINGS.parity).toInt();
        out.stringParity       = settings.value("stringParity", DEF_SETTINGS.stringParity).toString();
        out.stopBits           = (QSerialPort::StopBits)settings.value("stopBits", DEF_SETTINGS.stopBits).toInt();
        out.stringStopBits     = settings.value("stringStopBits", DEF_SETTINGS.stringStopBits).toString();
        out.flowControl        = (QSerialPort::FlowControl)settings.value("flowControl", DEF_SETTINGS.flowControl).toInt();
        out.stringFlowControl  = settings.value("stringFlowControl", DEF_SETTINGS.stringFlowControl).toString();
        out.sendNewLineEnabled = settings.value("sendNewLineEnabled", DEF_SETTINGS.sendNewLineEnabled).toBool();
        out.stringStatus       = settings.value("stringStatus", DEF_SETTINGS.stringStatus).toString();
        out.isDtr              = settings.value("isDtr", DEF_SETTINGS.isDtr).toBool();
        out.isRts              = settings.value("isRts", DEF_SETTINGS.isRts).toBool();
        out.isHexDisplay       = settings.value("isHexDisplay", DEF_SETTINGS.isHexDisplay).toBool();
        out.isHexSend          = settings.value("isHexSend", DEF_SETTINGS.isHexSend).toBool();
        out.isTimerSend        = settings.value("isTimerSend", DEF_SETTINGS.isTimerSend).toBool();
        out.timerLength        = settings.value("timeTimerSend", DEF_SETTINGS.timerLength).toInt();
        out.sendCache          = settings.value("sendCache", DEF_SETTINGS.sendCache).toString();
        out.sendNum            = 0;
        out.receiveNum         = 0;
    }

    return out;
}

void MainWindow::init()
{
    // 初始化定时发送定时器
    autoSendTimer = new QTimer(this);
    //将定时器超时信号与槽(功能函数)联系起来
    connect( autoSendTimer,SIGNAL(timeout()), this, SLOT(writeData()) );

    //读出上次保存Settings
    currentSettings = doSettings(false, Settings());

    mStatusBar = ui->statusBar;
    mSendButton = ui->send_pushButton;
    mSendFileButton = ui->sendfile_pushButton;
    mOpenSerialButton = ui->openserial_pushButton;
    mLedLabel = ui->led_label;
    mOpenFileLineEdit = ui->openfile_lineEdit;
    mTimerSendLineEdit = ui->timer_lineEdit;

    intValidator = new QIntValidator(0, 4000000, this);
    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    // 字体大小
    font = QFont( "Arial", 8);

    // 只能输入数字的正则
    QValidator *numberOnlyValidator; //检验器，只允许输入数字
    QRegExp regx("[0-9]+$"); //设置输入范围0～9
    numberOnlyValidator = new QRegExpValidator(regx, mTimerSendLineEdit);

    // 设置窗口标题
    QDateTime dt = QDateTime::fromTime_t( (uint)getDateFromMacro(__DATE__));
    this->setWindowTitle("sscom for linux 0.4, 作者:kangear " + dt.toString("yyyy/MM")); //

    // 状态
    isOn = false;
    mLedLabel->setPixmap(QPixmap(":/led/off"));
    mOpenFileLineEdit->setText(tr("文件名"));
    mFilePath = ui->openfile_lineEdit->text();
    mTimerSendLineEdit->setValidator(numberOnlyValidator);

    // 网址
    mNetAddrLabel = new QLabel;
    mNetAddrLabel->setMinimumSize(80, 14); // 设置标签最小大小
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
    mStatusLabel->setMinimumSize(240, 14); // 设置标签最小大小
    mStatusLabel->setAlignment(Qt::AlignLeft);
    mStatusLabel->setFont(font);

    // mFinallyLabel TODO:弄清楚意思后去实现它
    mFinallyLabel = new QLabel;
    mFinallyLabel->setMinimumSize(160, 14); // 设置标签最小大小
    mFinallyLabel->setAlignment(Qt::AlignLeft);
    mFinallyLabel->setText("CTS:0  DSR:0  RLSD:0 ");
    mFinallyLabel->setFont(font);

    mStatusBar->addWidget(mNetAddrLabel);
    mStatusBar->addWidget(mReceiveLabel);
    mStatusBar->addWidget(mSendLabel);
    mStatusBar->addWidget(mStatusLabel);
    mStatusBar->addWidget(mFinallyLabel);

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
    connect(mSendButton, SIGNAL(released()), this, SLOT(onSendButtonRelease()));
//! [3]

    // 1.先填充参数
    fillPortsParameters();
    fillPortsInfo();
    updateSettings();

    // 2.连接改变事件，一定要在填充参数之后进行连接，否则会出现无法填充已经保存参数
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

    connect(ui->dtr_checkBox, SIGNAL(stateChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->rts_checkBox, SIGNAL(stateChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->timersend_checkBox, SIGNAL(stateChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->hexdisplay_checkBox, SIGNAL(stateChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->hexsend_checkBox, SIGNAL(stateChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->newLineCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(currentIndexChanged()));
    connect(ui->timer_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(currentIndexChanged()));
    connect(ui->sendLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(currentIndexChanged()));
}

/**
 * @brief MainWindow::serialPortChanged
 * @param idx
 * handle StopBitsBox dataBitsBox stopBitsBox parityBox flowControlBox
 */
void MainWindow::serialPortChanged()
{
    if(serial->isOpen())
        closeSerialPort();
    updateSettings();
}

/** 将hex(61 62 63 64 65 66 67) to String(abcdefg) */
static QString hexToQString(bool isDebug, QString hexStr) {
    if(isDebug) qDebug() << __func__ << ": " << hexStr;
    QString ret;
    QStringList list = hexStr.split(" ", QString::SkipEmptyParts);
    for(QString qs:list) {
        bool bStatus = false;
        int a = qs.toInt(&bStatus, 16);
        if(bStatus && (qs.length() == 2)) {
            QString sA = QString(QChar(a));
            if(isDebug) qDebug() << "a:" << a << "sA:" << sA << "qs:" + qs;
            ret.append(sA);
        } else {/* 转换失败，使用默认字体串 */
            if(isDebug) qDebug() << "error!!!";
//            ret = "abcdefg";
            break;
        }
    }
    return ret;
}

/** 将String(abcdefg) to Hex(61 62 63 64 65 66 67) */
static QString stringToHex(bool isDebug, QString str) {
    if(isDebug) qDebug() << __func__ << ": " << str;
    QString ret;
    /* 将String(abcdefg) to Hex */
    for(int i=0; i<str.length(); i++) {
        /* 将字符串中字符转换成QChar */
        QChar random = str.at(i).toLatin1();
        QString hex;
        QString str1;
        /* 将QChar转换成unicode */
        hex.setNum(random.unicode(), 16);
        /**
         * 由于unicode位数随机，所以需要根据情况进行格式化
         * 只保留末两位，如果不够两位补0
         */
        if(hex.length() >= 2) {
            if(isDebug) qDebug() << "hex.length() >= 2 hex:" << hex;
            str1 = hex.mid(hex.length() - 2, hex.length());
        } else if(hex.length() == 1) {
            if(isDebug) qDebug() << "hex.length() == 1 hex:" << hex;
            str1 = hex.prepend("0");
        } else {
            if(isDebug) qDebug() << "else";
            str1 = "";
        }
        ret.append(str1.toUpper() + " ");
    }
    return ret;
}

/**
 * @brief MainWindow::currentIndexChanged
 * @param idx
 * handle StopBitsBox dataBitsBox stopBitsBox parityBox flowControlBox
 */
void MainWindow::currentIndexChanged()
{
    // qDebug() << __func__;
    Settings old = currentSettings;
    updateSettings();
    Settings now = currentSettings;
    if(serial->isOpen())
        setParameter(serial, currentSettings);

    // 更新显示方式
    if(old.isHexDisplay != now.isHexDisplay) {
        qDebug() << "is need hexDisplay:" << now.isHexDisplay;
    }

    // 输入框文字发生改变，更新缓存 解决第一次启动时缓存为空问题
    if(currentSettings.sendStringCache == "" ||
            QString::compare(old.sendCache, now.sendCache, Qt::CaseInsensitive) != 0) {
        currentSettings.sendStringCache = now.sendCache;
        /* 如果是16进制状态，需要将16进制数转换成字符串 */
        if(now.isHexSend) {
            currentSettings.sendStringCache = hexToQString(false, now.sendCache);
        }
        /* 将小写换成大写 */
        ui->sendLineEdit->setText(ui->sendLineEdit->text().toUpper());
    }

    // 更新发送方式
    if(old.isHexSend != now.isHexSend) {
        /* 需要将输入框内容有字体串改为16进制显示 */
        QString inputStr = ui->sendLineEdit->text();
        if(now.isHexSend) {
            /* 将原数据保存 */
            currentSettings.sendCache = stringToHex(false, inputStr);
            currentSettings.sendStringCache = inputStr;
        } else {
            /* 将数据保存 */
            QString tmp = hexToQString(false ,inputStr);
            currentSettings.sendCache = tmp;
            currentSettings.sendStringCache = tmp;
        }
        // qDebug() << "sendStringCache:" << currentSettings.sendStringCache;
        ui->sendLineEdit->setText(currentSettings.sendCache);
    }

    if(DEBUG) qDebug() << "sendStringCache:" << stringToHex(false, currentSettings.sendStringCache);

    // 更新AutoSend Qtimer
    if(old.isTimerSend != now.isTimerSend) {
        if(now.isTimerSend) {
            // 禁用timerLength编辑
            ui->timer_lineEdit->setDisabled(true);
            // 停止运行定时器
            if (autoSendTimer->isActive() )
                autoSendTimer->stop();
            //开始运行定时器，定时时间间隔为1000ms
            autoSendTimer->start(now.timerLength);
        } else {
            // 启用timerLength编辑
            ui->timer_lineEdit->setDisabled(false);
            // 停止运行定时器
            if (autoSendTimer->isActive() )
                autoSendTimer->stop();
        }
    }
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

void setCurrentIndex(QComboBox *comboBox, QString text)
{
    int index = comboBox->findText(text); //use default exact match
    if(index >= 0)
         comboBox->setCurrentIndex(index);
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
    ui->flowControlBox->addItem(QStringLiteral("Hardware"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(QStringLiteral("Software"), QSerialPort::SoftwareControl);

    // 还原上次保存的参数
    setCurrentIndex(ui->baudRateBox,    currentSettings.stringBaudRate);
    setCurrentIndex(ui->dataBitsBox,    currentSettings.stringDataBits);
    setCurrentIndex(ui->stopBitsBox,    currentSettings.stringStopBits);
    setCurrentIndex(ui->parityBox,      currentSettings.stringParity);
    setCurrentIndex(ui->flowControlBox, currentSettings.stringFlowControl);

    ui->dtr_checkBox->setChecked(currentSettings.isDtr);
    ui->rts_checkBox->setChecked(currentSettings.isRts);
    ui->hexdisplay_checkBox->setChecked(currentSettings.isHexDisplay);
    ui->hexsend_checkBox->setChecked(currentSettings.isHexSend);
    ui->newLineCheckBox->setChecked(currentSettings.sendNewLineEnabled);
    ui->timersend_checkBox->setChecked(currentSettings.isTimerSend);
    ui->timer_lineEdit->setText(QString::number(currentSettings.timerLength));
    ui->sendLineEdit->setText(currentSettings.sendCache);

    /* 启动后第一次更新 */
    currentIndexChanged();
}

MainWindow::~MainWindow()
{
    // 更新最终设置并保存
    updateSettings();
    doSettings(true, currentSettings);
    // 防止没有关闭串口
    closeSerialPort();
    // 删除申请的对象
    delete autoSendTimer;
    delete serial;
    delete ui;

}

void MainWindow::on_openserial_pushButton_pressed()
{
    if(serial->isOpen())
        closeSerialPort();
    else
        openSerialPort();
    currentIndexChanged();
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

    currentSettings.isDtr = ui->dtr_checkBox->isChecked();
    currentSettings.isRts = ui->rts_checkBox->isChecked();
    currentSettings.isHexDisplay = ui->hexdisplay_checkBox->isChecked();
    currentSettings.isHexSend = ui->hexsend_checkBox->isChecked();
    currentSettings.sendNewLineEnabled = ui->newLineCheckBox->isChecked();
    currentSettings.isTimerSend = ui->timersend_checkBox->isChecked();
    currentSettings.timerLength = ui->timer_lineEdit->text().toInt();
    currentSettings.sendCache = ui->sendLineEdit->text();

    // 更新状态栏
    updateUi(currentSettings);
}

void MainWindow::updateUi(Settings p)
{
    // 更新状态栏
    mStatusLabel->setText(tr("%1 %2 %3bps,%4,%5, %6, %7")
                          .arg(p.name).arg(p.stringStatus).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringStopBits).arg(p.stringParity).arg(p.stringFlowControl));

    // 更新接收发送总量
    if(DEBUG) qDebug() <<"p.receiveNum:" << p.receiveNum;
    if(DEBUG) qDebug() <<"p.sendNum:" << p.sendNum;
    mReceiveLabel->setText(tr("R:%1").arg(p.receiveNum));
    mSendLabel->setText(tr("S:%1").arg(p.sendNum));
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

void MainWindow::onSendButtonRelease()
{
    currentIndexChanged();
    writeData();
}

/** 发送数据 */
void MainWindow::writeData()
{
    if(!serial->isOpen()) {
        return;
    }

    QString text = currentSettings.sendStringCache;
    if(DEBUG) qDebug() << __func__ << ":" << text;
    if(text.length() !=0 && currentSettings.sendNewLineEnabled)
        text += "\r\n";

    QByteArray data = text.toLatin1();
    qint32 len = serial->write(data);
    // 更新显示长度
    if(len >= 0) {
        currentSettings.sendNum += len;
        updateUi(currentSettings);
    }
    if(DEBUG) qDebug() <<"currentSettings.sendNum:" << currentSettings.sendNum;
}
//! [6]
//! [7]
void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    QString str = QString::fromLatin1(data.data());
    QString res = str.toLatin1().toHex().toUpper();
    QString a = ui->receive_textBrowser->toPlainText();
    ui->receive_textBrowser->setText(a + data);

    // 更新显示长度
    qint32 len = data.length();
    if(DEBUG) qDebug() <<"len:" << len;
    if(len >= 0) {
        currentSettings.receiveNum += len;
        currentIndexChanged();
    }


    // 根据设置来判断是否需要转换成HEX

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
/**
 * @brief MainWindow::on_clear_pushButton_released
 * 清除接收区内容
 */
void MainWindow::on_clear_pushButton_released()
{
    ui->receive_textBrowser->clear();
    currentIndexChanged();
}

/** 16进制发送 checkbox按下时 */
void MainWindow::on_hexsend_checkBox_released()
{
}
