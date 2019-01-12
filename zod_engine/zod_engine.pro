# *************************************************
include(../common.pri)
# *************************************************

QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += release

###########################
###     SET .H C++11    ###
###########################
#QMAKE_CXXFLAGS += -std=gnu++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wall

###########################
###         BUILD       ###
###########################
DESTDIR = $$INSTALL_PATH_BIN

BUILD_PATH = ./
OBJECTS_DIR = $${BUILD_PATH}objects

###########################
###     DATA FILES      ###
###########################
SOURCES += \
        main.cpp \
    main_options.cpp
HEADERS += \
    main_options.h

###########################
###     outer  LIBs     ###
###########################

# ---- wxWidgets --------------------------------------------
wxCXXFLAGS = $$system(wx-config --cxxflags --unicode=yes --debug=no)
wxLinkOptions = $$system(wx-config --debug=no --libs --unicode=yes)
LIBS += $$wxLinkOptions
QMAKE_CXXFLAGS_RELEASE += $$wxCXXFLAGS
QMAKE_CXXFLAGS_DEBUG += $$wxCXXFLAGS

## ----- SDL ------------------------------------------------
LIBS += \
-lSDL           \
-lSDL_image     \
-lSDL_ttf       \
-lSDL_mixer     \
-lmysqlclient   \
-lGL            \



LIBS += -L$$INSTALL_PATH_LIB
LIBS += \
-l_qZod_DnSeparate    \
-l_qZod_DnMap         \
-l_qZod_DnSoundEngine \
-l_qZod_DnSettings    \
-l_qZod_DnGui         \
-l_qZod_DnEffect      \
-l_qZod_DnObjects     \
-l_qZod_DnClientServer


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target


