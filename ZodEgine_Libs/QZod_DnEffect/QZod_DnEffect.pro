#-------------------------------------------------
#
# Project created by QtCreator 2019-01-08T12:16:09
#
#-------------------------------------------------
# *************************************************
include(../../common.pri)
# *************************************************
QT       -= gui
CONFIG += c++11
CONFIG += plugin
CONFIG += qt

TARGET = _qZod_DnEffect
TEMPLATE = lib

DEFINES += QZOD_DNEFFECT_LIBRARY
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
        qzod_dneffect.cpp \
    ebridgeturrent.cpp \
    ebullet.cpp \
    ecannondeath.cpp \
    ecraneconco.cpp \
    edeath.cpp \
    edeathsparks.cpp \
    eflame.cpp \
    elaser.cpp \
    elightinitfire.cpp \
    elightrocket.cpp \
    emapobjectturrent.cpp \
    emissilecrockets.cpp \
    emomissilerockets.cpp \
    epyrofire.cpp \
    erobotdeath.cpp \
    erobotturrent.cpp \
    erockparticle.cpp \
    erockturrent.cpp \
    esideexplosion.cpp \
    estandard.cpp \
    etankdirt.cpp \
    etankoil.cpp \
    etanksmoke.cpp \
    etankspark.cpp \
    etoughmushroom.cpp \
    etoughrocket.cpp \
    etoughsmoke.cpp \
    etrack.cpp \
    eturrentmissile.cpp \
    eunitparticle.cpp \
    qzod_effect_old.cpp

HEADERS += \
        qzod_dneffect.h \
        qzod_dneffect_global.h \
    ebridgeturrent.h \
    ebullet.h \
    ecannondeath.h \
    ecraneconco.h \
    edeath.h \
    edeathsparks.h \
    eflame.h \
    elaser.h \
    elightinitfire.h \
    elightrocket.h \
    emapobjectturrent.h \
    emissilecrockets.h \
    emomissilerockets.h \
    epyrofire.h \
    erobotdeath.h \
    erobotturrent.h \
    erockparticle.h \
    erockturrent.h \
    esideexplosion.h \
    estandard.h \
    etankdirt.h \
    etankoil.h \
    etanksmoke.h \
    etankspark.h \
    etoughmushroom.h \
    etoughrocket.h \
    etoughsmoke.h \
    etrack.h \
    eturrentmissile.h \
    eunitparticle.h \
    qzod_effect_old.h \
    zod_effect.hpp



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
-l_qZod_DnSettings


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



