
QT -= gui -core

TARGET = sharedkv
CONFIG += console c++11 ZMQ
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    sharedobjectdata.cpp \
    sharedobject.cpp


ZMQ{
    ZMQ_PATH =/usr/local/Cellar/zeromq/4.2.1
    #ZMQ_PATH="E:\zeromq-4.2.0"
    INCLUDEPATH+= $${ZMQ_PATH}/include
    LIBS += -L$${ZMQ_PATH}/lib -lzmq
    #LIBS += -L$${ZMQ_PATH}/bin/x64/Release/v120/dynamic -llibzmq
    message("$${ZMQ_PATH}/include")
}

HEADERS += main.h \
     cmdline.h \
    sharedobjectdata.h \
    sharedobject.h \
    blockqueue.h

message("Project in $$PWD")
