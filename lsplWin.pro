#-------------------------------------------------
#
# Project created by QtCreator 2014-05-25T07:19:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lsplWin
TEMPLATE = app

QMAKE_CXXFLAGS += -g -std=gnu++0x

LIBS += -llspl

SOURCES += main.cpp \
        mainwindow.cpp \
    omonimdialog.cpp \
    matchtablemodel.cpp

HEADERS  += mainwindow.h \
    omonimdialog.h \
    matchtablemodel.h

FORMS    += mainwindow.ui \
    omonimdialog.ui
