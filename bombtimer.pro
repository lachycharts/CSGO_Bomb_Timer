#-------------------------------------------------
#
# Project created by QtCreator 2015-12-12T15:00:08
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bombtimer
TEMPLATE = app
CONFIG += c++11 console


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    dialog.cpp \

HEADERS  += mainwindow.h \
    server.h \
    dialog.h \

FORMS    += mainwindow.ui \
    dialog.ui

RC_FILE += icon.rc

DISTFILES +=

RESOURCES += \
    icons.qrc \
    sounds.qrc

