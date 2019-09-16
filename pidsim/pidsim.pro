#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR += ../bin
#OBJECTS_DIR = ../bin/obj

TARGET = pidsim
TEMPLATE = app
DEFINES -= UNICODE

CONFIG += c++11
CONFIG += warn_off  #不要啥都提示
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

LIBS += -ldl

INCLUDEPATH += ../common 

win32 {
	LIBS += -lwsock32
}

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ../common/common.cpp \
    ../common/jsoncpp.cpp \
    appflow.cpp

HEADERS += \
        mainwindow.h \
    ../common/common.h \
    ../common/json.h \
    ../common/main.h \
    appflow.h

FORMS += \
        mainwindow.ui

#LIBS += -L$$PWD/./ 

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

RESOURCES += \
    res.qrc

