#-------------------------------------------------
#
# Project created by QtCreator 2015-07-03T10:16:56
#
#-------------------------------------------------

QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sscom
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    commonhelper.cpp

HEADERS  += mainwindow.h \
    commonhelper.h

FORMS    += mainwindow.ui

OTHER_FILES +=

RESOURCES += \
    images.qrc

QMAKE_CXXFLAGS += -std=c++0x
