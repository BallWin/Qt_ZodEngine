TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


QMAKE_LFLAGS += -pthread
QMAKE_CXXFLAGS += -std=c++11

OBJECTS_DIR = objs

DESTDIR = bin

SOURCES += \
        main.cpp \
    bitstream.cpp \
    jv_format.cpp \
    video_proc.cpp \
    audio_proc.cpp

HEADERS += \
    general_head.h \
    bitstream.h \
    jv_format.h \
    to_bmp.h \
    video_proc.h \
    audio_proc.h


## ----- SDL ------------------------------------------------
LIBS += \
-lSDL           \
-lSDL_image     \
-lSDL_ttf       \
-lSDL_mixer     \
-lmysqlclient   \
-lGL            \
