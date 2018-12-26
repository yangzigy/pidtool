#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR += ../bin
OBJECTS_DIR = ../bin/obj

TARGET = pidsim
TEMPLATE = app
DEFINES -= UNICODE

CONFIG += c++11
CONFIG += warn_off  #不要啥都提示
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../common 

#win32 {
	#LIBS += -lwsock32
#}

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ../common/pid.cpp \
    ../common/jsoncpp.cpp 

HEADERS += \
        mainwindow.h \
    ../common/json.h \
    ../common/pid.h \
    ../common/main.h 

FORMS += \
        mainwindow.ui

#LIBS += -L$$PWD/./ 

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

