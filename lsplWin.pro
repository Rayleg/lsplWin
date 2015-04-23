#-------------------------------------------------
#
# Project created by QtCreator 2014-05-25T07:19:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lsplWin
TEMPLATE = app

#INCLUDE_DIR = D:/lspl
#BOOST_DIR = C:/boost_1_55_0/boost_1_55_0

QMAKE_CXXFLAGS += -g -std=gnu++0x

#INCLUDEPATH += D:/lspl/includes/lspl \
#    D:/lspl/includes \
#    $${BOOST_DIR}

LIBS += -llspl

# DEPENDPATH += $${INCLUDEPATH}

SOURCES += main.cpp \
        mainwindow.cpp \
    stat.cpp \
    omonimdialog.cpp

HEADERS  += mainwindow.h \
    omonimdialog.h

FORMS    += mainwindow.ui \
    omonimdialog.ui
