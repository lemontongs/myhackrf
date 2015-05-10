#-------------------------------------------------
#
# Project created by QtCreator 2015-02-28T15:44:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GUI
TEMPLATE = app

LIBS += -L/usr/local/lib/ -lzmq -lprotobuf -lpthread

SOURCES += main.cpp\
        mainwindow.cpp \
        receiver.cpp \
        ../protobuffers/packet.pb.cc

HEADERS  += mainwindow.h \
        receiver.h \
        zhelpers.h \
        ../protobuffers/packet.pb.h

INCLUDEPATH += ../protobuffers

FORMS    += mainwindow.ui

include( /usr/local/qwt-6.1.2/features/qwt.prf )

