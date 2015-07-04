#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>

#define FDATE (char const[]){ __DATE__[7], __DATE__[8], __DATE__[9], '\0' }

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
    void on_openserial_pushButton_pressed();

    void on_openfile_pushButton_released();

private:
    Ui::MainWindow *ui;
    QStatusBar *mStatusBar;
    /** 网址 */
    QLabel *mStatusLabel;
    /** 状态标签 */
    QLabel *mNetAddrLabel;
    /** led */
    QLabel *mLedLabel;
    QFont font;
    /** 总接收数量显示标签 */
    QLabel *mReceiveLabel;
    /** 总发送数量显示标签 */
    QLabel *mSendLabel;
    bool isOn;
    void init();
    /** 文件路径 */
    QString mFilePath;
    QPushButton *mSendButton;
    QPushButton *mSendFileButton;
    QPushButton *mOpenSerialButton;

    QLineEdit *mOpenFileLineEdit;
    QLineEdit *mTimerSendLineEdit;
};

#endif // MAINWINDOW_H
