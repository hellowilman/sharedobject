
QT -= gui -core
CONFIG -= app_bundle


TARGET = sharedkv
CONFIG += console c++11 ZMQ

TEMPLATE = app

SOURCES += \
    sharedobjectdata.cpp \
    sharedobject.cpp \
    md5.cpp \
    mainapp.cpp \
    main.cpp



HEADERS += \
    cmdline.h \
    sharedobjectdata.h \
    sharedobject.h \
    blockqueue.h \
    md5.h \
    mainapp.h




ZMQ{
    #ZMQ_PATH =/usr/local/Cellar/zeromq/4.2.1 # for Mac Mini
    #ZMQ_PATH =/usr/local/Cellar/zeromq/4.2.0 # for MBP
    ZMQ_PATH="$$PWD/../3party/zmq_win64"
    INCLUDEPATH+= $${ZMQ_PATH}/include
    LIBS += -L$${ZMQ_PATH}/lib -llibzmq
    #LIBS += -L$${ZMQ_PATH}/bin/x64/Release/v120/dynamic -llibzmq
    message("$${ZMQ_PATH}/include")
}

message("Project in $$PWD")
