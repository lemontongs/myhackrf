#-------------------------------------------------
#
# Project created by QtCreator 2015-02-28T15:44:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GUI
TEMPLATE = app

LIBS += -lzmq -lfftw3

SOURCES += main.cpp\
        mainwindow.cpp \
    receiver.cpp

HEADERS  += mainwindow.h \
    receiver.h

FORMS    += mainwindow.ui

include( /usr/local/qwt-6.1.2/features/qwt.prf )

