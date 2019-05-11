#-------------------------------------------------
#
# Project created by QtCreator 2019-05-01T21:33:47
#
#-------------------------------------------------

QT       += core gui
QT       += svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameOfLifeDemo
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

QMAKE_LFLAGS_WINDOWS += -Wl,--stack,32000000

OMP_NUM_THREADS = 4

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        golscene.cpp \
    golthread.cpp \
    renderdialog.cpp

HEADERS += \
        mainwindow.h \
        golscene.h \
    golthread.h \
    renderdialog.h

FORMS += \
        mainwindow.ui \
    renderdialog.ui
