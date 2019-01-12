#-------------------------------------------------
#
# Project created by QtCreator 2018-12-23T16:17:24
#
#-------------------------------------------------
# *************************************************
include(../common.pri)
# *************************************************

QT       += core gui
CONFIG += c++11
CONFIG += release

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TEMPLATE = app
TARGET = QZod_Launcher

DESTDIR = $${INSTALL_PATH_BIN}


SOURCES += \
        main.cpp \
        qz_launcher.cpp

HEADERS += \
        qz_launcher.h \
    q_settings.h

FORMS += \
        qz_launcher.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
