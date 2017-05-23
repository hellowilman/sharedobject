
QT -= gui -core

TARGET = skv_cli
CONFIG += console ZMQ c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main_test.cpp


ZMQ{
#ZMQ_PATH =/usr/local/Cellar/zeromq/4.2.0
ZMQ_PATH="E:\zeromq-4.2.0"
INCLUDEPATH+= $${ZMQ_PATH}/include
#LIBS += -L$${ZMQ_PATH}/lib -llibzmq
LIBS += -L$${ZMQ_PATH}/bin/x64/Release/v120/dynamic -llibzmq
message("$${ZMQ_PATH}/include")
}


message("Project in $$PWD")
