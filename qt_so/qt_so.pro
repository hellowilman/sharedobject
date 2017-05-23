#-------------------------------------------------
#
# Project created by QtCreator 2017-05-23T16:38:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_so
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    ../src/sharedobject.cpp \
    ../src/sharedobjectdata.cpp \
    ../src/md5.cpp

HEADERS  += mainwindow.h \
    ../src/sharedobject.h \
    ../src/sharedobjectdata.h \
    ../src/md5.h

FORMS    += mainwindow.ui

CONFIG += ZMQ console

ZMQ{
    #ZMQ_PATH =/usr/local/Cellar/zeromq/4.2.0
    ZMQ_PATH=$$PWD"/../3party/zmq_win64"
    INCLUDEPATH+= $${ZMQ_PATH}/include
    LIBS += -L$${ZMQ_PATH}/lib  -llibzmq
    #LIBS += -L$${ZMQ_PATH}/bin/x64/Release/v120/dynamic -llibzmq
    message("$${ZMQ_PATH}/include")
}

INCLUDEPATH += $$PWD/../include
INCLUDEPATH += $$PWD/../src
