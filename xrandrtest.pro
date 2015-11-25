QT += core
QT -= gui

TARGET = xrandrtest
CONFIG += console
CONFIG -= app_bundle

LIBS += -lXrandr -lX11

TEMPLATE = app

SOURCES += main.cpp \
    qxrandr.cpp \
    eventnotifier.cpp

HEADERS += \
    qxrandr.h \
    eventnotifier.h

