#-------------------------------------------------
#
# Project created by QtCreator 2019-01-08T10:55:47
#
#-------------------------------------------------
# *************************************************
include(../../common.pri)
# *************************************************
QT       -= gui
CONFIG += c++11
CONFIG += plugin
CONFIG += qt

TARGET = _qZod_DnGui
TEMPLATE = lib

DEFINES += QZOD_DNGUI_LIBRARY
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
        qzod_dngui.cpp \
    gmm_change_teams.cpp \
    gmm_main_menu.cpp \
    gmm_manage_bots.cpp \
    gmm_options.cpp \
    gmm_player_list.cpp \
    gmm_select_map.cpp \
    gmm_warning.cpp \
    zgui_main_menu_base.cpp \
    zgui_main_menu_widgets.cpp \
    gmmw_button.cpp \
    gmmw_label.cpp \
    gmmw_list.cpp \
    gmmw_radio.cpp \
    gmmw_team_color.cpp

HEADERS += \
        qzod_dngui.h \
        qzod_dngui_global.h \
    gmm_change_teams.h \
    gmm_main_menu.h \
    gmm_manage_bots.h \
    gmm_options.h \
    gmm_player_list.h \
    gmm_select_map.h \
    gmm_warning.h \
    gui_structures.h \
    zgui_main_menu_base.h \
    zgui_main_menu_widgets.h \
    z_gui.hpp


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
-l_qZod_DnMap


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
