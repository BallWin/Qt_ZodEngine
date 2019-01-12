#-------------------------------------------------
#
# Project created by QtCreator 2019-01-08T17:02:59
#
#-------------------------------------------------
# *************************************************
include(../../common.pri)
# *************************************************
QT       -= gui
CONFIG += c++11
CONFIG += plugin
CONFIG += qt

TARGET = _qZod_DnClientServer
TEMPLATE = lib

DEFINES += QZOD_DNCLIENTSERVER_LIBRARY
###########################
###     SET .H C++11    ###
###########################
#QMAKE_CXXFLAGS += -std=gnu++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wall

###########################
###         BUILD       ###
###########################
DESTDIR = $$INSTALL_PATH_LIB

BUILD_PATH = ./
OBJECTS_DIR = $${BUILD_PATH}objects

###########################
###     DATA FILES      ###
###########################
SOURCES += \
        qzod_dnclientserver.cpp \
    client_socket.cpp \
    server_socket.cpp \
    socket_handler.cpp \
    zbot.cpp \
    zbot_events.cpp \
    zclient.cpp \
    zcore.cpp \
    zplayer.cpp \
    zplayer_events.cpp \
    zserver.cpp \
    zserver_commands.cpp \
    zserver_events.cpp \
    ztray.cpp

HEADERS += \
        qzod_dnclientserver.h \
        qzod_dnclientserver_global.h \
    client_socket.h \
    server_socket.h \
    socket_handler.h \
    zbot.h \
    zclient.h \
    zcore.h \
    zplayer.h \
    zserver.h \
    ztray.h

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
-l_qZod_DnSeparate     \
-l_qZod_DnMap          \
-l_qZod_DnSoundEngine  \
-l_qZod_DnSettings     \
-l_qZod_DnGui          \
-l_qZod_DnEffect       \
-l_qZod_DnObjects


###########################
### post copy /include  ###
###########################
for( name, HEADERS ){
    copyheaders.commands += mkdir -p $${INSTALL_PATH_INCLUDE}/lib$${TARGET};
    copyheaders.commands += cp --parents $$name $${INSTALL_PATH_INCLUDE}/lib$${TARGET};
}
QMAKE_EXTRA_TARGETS += copyheaders
POST_TARGETDEPS += copyheaders
#############################################
#############################################


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



