#-------------------------------------------------
#
# Project created by QtCreator 2015-02-28T15:44:33
#
#-------------------------------------------------

QT       += core gui

CONFIG += qwt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GUI
TEMPLATE = app

LIBS += -L/usr/local/lib/ -lzmq -lprotobuf -lpthread

SOURCES += main.cpp\
        mainwindow.cpp \
        receiver.cpp \
        ../../build/src/protobuffers/packet.pb.cc

HEADERS  += mainwindow.h \
        receiver.h

INCLUDEPATH += ../protobuffers ../utilities ../../build/src/protobuffers/

FORMS    += mainwindow.ui

#include( /usr/local/qwt-6.1.2/features/qwt.prf )

