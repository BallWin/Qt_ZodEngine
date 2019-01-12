#-------------------------------------------------
#
# Project created by QtCreator 2019-01-07T19:35:11
#
#-------------------------------------------------
# *************************************************
include(../../common.pri)
# *************************************************
QT       -= gui
CONFIG += c++11
CONFIG += plugin
CONFIG += qt

TARGET = _qZod_DnSeparate
TEMPLATE = lib

DEFINES += QZOD_DNSEPARATE_LIBRARY
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
        qzod_dnseparate.cpp \
    common.cpp \
    event_handler.cpp \
    SDL_rotozoom.cpp \
    zencrypt_aes.cpp \
    zfont.cpp \
    zfont_engine.cpp \
    zmysql.cpp \
    zpsettings.cpp \
    zsdl.cpp \
    zsdl_opengl.cpp \
    ztime.cpp

HEADERS += \
        qzod_dnseparate.h \
        qzod_dnseparate_global.h \
    common.h \
    constants.h \
    event_handler.h \
    SDL_rotozoom.h \
    zdamagemissile.h \
    zencrypt_aes.h \
    zfont.h \
    zfont_engine.h \
    zmysql.h \
    zpsettings.h \
    zsdl.h \
    zsdl_opengl.h \
    ztime.h


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
