#-------------------------------------------------
#
# Project created by QtCreator 2014-07-29T09:53:12
#
#-------------------------------------------------

QT       += core gui
RC_ICONS += wali.ico
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = musicPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    curvePlot.cpp

HEADERS  += mainwindow.h \
    curvePlot.h

FORMS    += mainwindow.ui

LIBS     += D:/Qt_MSVC/test_msvc/Audio/musicPlayer_up/musicPlayer/lib/fmodex_vc.lib

RESOURCES += \
    images.qrc
