#-------------------------------------------------
#
# Project created by QtCreator 2016-01-19T18:27:52
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = convertor
TEMPLATE = app


SOURCES += main.cpp \
    ticket.cpp \
    sqlconnectdialog.cpp \
    mainwindow.cpp \
    parser.cpp \
    ticketpreview.cpp \
    finddialog.cpp

HEADERS  += \
    ticket.h \
    sqlconnectdialog.h \
    mainwindow.h \
    parser.h \
    ticketpreview.h \
    finddialog.h

FORMS += \
    sqlconnectdialog.ui \
    ticketpreview.ui \
    finddialog.ui
