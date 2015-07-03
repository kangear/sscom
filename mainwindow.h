#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFont>

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
    /** 网址 */
    QLabel *statusLabel;
    /** 状态标签 */
    QLabel *netAddrLabel;
    QFont font;
    /** 总接收数量显示标签 */
    QLabel *receiveLabel;
    /** 总发送数量显示标签 */
    QLabel *sendLabel;
    bool isOn;
    void init();
    /** 文件路径 */
    QString mFilePath;
};

#endif // MAINWINDOW_H
