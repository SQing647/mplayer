#-------------------------------------------------
#
# Project created by QtCreator 2022-08-30T16:31:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mplayer
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    label_roll.cpp

HEADERS  += widget.h \
    label_roll.h

FORMS    += widget.ui

RESOURCES += \
    image.qrc
