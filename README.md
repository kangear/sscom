# sscom
Linux版本 串口调试助手
Hot To build
```
sudo apt-get install qtcreator libqt5serialport5-dev libudev-dev qt5-default
qmake 
make
```

# 预览效果
![GitHub Logo](/assert/sscom_for_linux_0.2.png)


# 未实现功能
![GitHub Logo](/assert/sscom_for_linux_0.2_unspport.png)


# 解决
```
sscom.pro:12: Unable to find file for inclusion /src/serialport/qt4support/serialport.pr，
要使用qt5编译 ，
```
