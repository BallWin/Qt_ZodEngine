#-------------------------------------------------
#
# Project created by QtCreator 2019-01-07T20:55:41
#
#-------------------------------------------------
# *************************************************
include(../../common.pri)
# *************************************************
#QT       += widgets
QT       -= gui
CONFIG += c++11
CONFIG += plugin
CONFIG += qt


TARGET = _qZod_DnMap
TEMPLATE = lib

DEFINES += QZOD_DNMAP_LIBRARY
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
        qzod_dnmap.cpp \
    qzod_map.cpp \
    zmap_crater_graphics.cpp \
    zteam.cpp \
    finding/zpath_finding_old.cpp

HEADERS += \
        qzod_dnmap.h \
        qzod_dnmap_global.h \
    qzod_map.h \
    zmap_crater_graphics.h \
    zmap_structures_old.h \
    zteam.h \
    finding/find_structures.h \
    finding/zpath_finding_old.h


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



#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}
