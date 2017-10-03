#-------------------------------------------------
#
# Project created by QtCreator 2017-09-08T15:18:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = photoDesc
TEMPLATE = app


SOURCES += main.cpp\
        photodesc.cpp

HEADERS  += photodesc.h

FORMS    += photodesc.ui

unix:!macx: LIBS += -lexiv2
