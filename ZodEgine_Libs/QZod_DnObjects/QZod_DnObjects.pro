#-------------------------------------------------
#
# Project created by QtCreator 2019-01-08T14:23:03
#
#-------------------------------------------------
# *************************************************
include(../../common.pri)
# *************************************************
QT       -= gui
CONFIG += c++11
CONFIG += plugin
CONFIG += qt

TARGET = _qZod_DnObjects
TEMPLATE = lib

DEFINES += QZOD_DNOBJECTS_LIBRARY
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
        qzod_dnobjects.cpp \
    cursor.cpp \
    oflag.cpp \
    ogrenades.cpp \
    ohut.cpp \
    omapobject.cpp \
    orock.cpp \
    orockets.cpp \
    zcomp_message_engine.cpp \
    zgfile.cpp \
    zgui_window.cpp \
    zhud.cpp \
    zmini_map.cpp \
    zmusic_engine.cpp \
    zobject.cpp \
    zunitrating.cpp \
    Animals/abird.cpp \
    Animals/ahutanimal.cpp \
    Buildings/bbridge.cpp \
    Buildings/bfort.cpp \
    Buildings/bradar.cpp \
    Buildings/brepair.cpp \
    Buildings/brobot.cpp \
    Buildings/bvehicle.cpp \
    Buildings/zbuilding.cpp \
    Buildings/zbuildlist.cpp \
    Cannon/cgatling.cpp \
    Cannon/cgun.cpp \
    Cannon/chowitzer.cpp \
    Cannon/cmissilecannon.cpp \
    Cannon/zcannon.cpp \
    Gw/gwcreateuser.cpp \
    Gw/gwfactory_list.cpp \
    Gw/gwlogin.cpp \
    Gw/gwproduction.cpp \
    Gw/gwproduction_fus.cpp \
    Gw/gwproduction_us.cpp \
    Robots/rgrunt.cpp \
    Robots/rlaser.cpp \
    Robots/rpsycho.cpp \
    Robots/rpyro.cpp \
    Robots/rsniper.cpp \
    Robots/rtough.cpp \
    Robots/zrobot.cpp \
    Vehicles/vapc.cpp \
    Vehicles/vcrane.cpp \
    Vehicles/vheavy.cpp \
    Vehicles/vjeep.cpp \
    Vehicles/vlight.cpp \
    Vehicles/vmedium.cpp \
    Vehicles/vmissilelauncher.cpp \
    Vehicles/zvehicle.cpp

HEADERS += \
        qzod_dnobjects.h \
        qzod_dnobjects_global.h \
    cursor.h \
    oflag.h \
    ogrenades.h \
    ohut.h \
    omapobject.h \
    orock.h \
    orockets.h \
    zcomp_message_engine.h \
    zgfile.h \
    zgui_window.h \
    zhud.h \
    zmini_map.h \
    zmusic_engine.h \
    zobject.h \
    zod_objects_old.hpp \
    zod_obj_structures.h \
    zunitrating.h \
    Animals/abird.h \
    Animals/ahutanimal.h \
    Buildings/bbridge.h \
    Buildings/bfort.h \
    Buildings/bradar.h \
    Buildings/brepair.h \
    Buildings/brobot.h \
    Buildings/bvehicle.h \
    Buildings/zbuilding.h \
    Buildings/zbuildlist.h \
    Cannon/cgatling.h \
    Cannon/cgun.h \
    Cannon/chowitzer.h \
    Cannon/cmissilecannon.h \
    Cannon/zcannon.h \
    Gw/gwcreateuser.h \
    Gw/gwfactory_list.h \
    Gw/gwlogin.h \
    Gw/gwproduction.h \
    Robots/rgrunt.h \
    Robots/rlaser.h \
    Robots/rpsycho.h \
    Robots/rpyro.h \
    Robots/rsniper.h \
    Robots/rtough.h \
    Robots/zrobot.h \
    Vehicles/vapc.h \
    Vehicles/vcrane.h \
    Vehicles/vheavy.h \
    Vehicles/vjeep.h \
    Vehicles/vlight.h \
    Vehicles/vmedium.h \
    Vehicles/vmissilelauncher.h \
    Vehicles/zvehicle.h

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
-l_qZod_DnEffect


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



