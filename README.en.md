# sscom

Serial port debugging assistant for Linux/Mac.

[中文文档](README.md)

# Screenshots
## Preview
![GitHub Logo](/assert/Selection_441.png)

## Unimplemented features
![GitHub Logo](/assert/sscom_for_linux_0.2_unspport.png)

# Usage
Two ways: download a prebuilt binary, or build from source.

## Download prebuilt binary
Go to https://github.com/kangear/sscom/releases to download the AppImage version, which runs on any Linux distribution. If it does not run, please open an issue.

## Build from source on Linux
```shell
sudo apt-get install qtcreator libqt5serialport5-dev libudev-dev qt5-default
qmake 
make
```
## Build from source on Mac
```shell
brew install qt5
export PATH=$PATH:/usr/local/Cellar/qt@5/5.15.2_1/bin
qmake 
make
```

# Troubleshooting
```
sscom.pro:12: Unable to find file for inclusion /src/serialport/qt4support/serialport.pr
```
This means you are building with Qt4 — you need to build with Qt5.
