#defines
PROJECT_NAME           = Q_ZodEngine

ROOT_DIR               = $$PWD
INSTALL_PATH_BIN       = $${ROOT_DIR}/bin
TRANSLATE_PATH         = $${INSTALL_PATH_BIN}/translate
INSTALL_PATH_LIB       = $${INSTALL_PATH_BIN}/lib
INSTALL_PATH_INCLUDE   = $${ROOT_DIR}/include



# --- data folders --------------------
INSTALL_PATH_DATA	  = $${ROOT_DIR}/Data
INSTALL_PATH_LOG	  = $${INSTALL_PATH_DATA}/LOG
INSTALL_PATH_SAVE	  = $${INSTALL_PATH_DATA}/Save

INSTALL_PATH_TMP           = $${INSTALL_PATH_DATA}/tmp
INSTALL_PATH_DEBUG         = $${INSTALL_PATH_DATA}/debug
INSTALL_PATH_SNAPSHOT      = $${INSTALL_PATH_DATA}/splash
INSTALL_PATH_VIDEO	   = $${INSTALL_PATH_DATA}/video


BUILD_PATH = ./
ARCH = ./

INCLUDEPATH += $${ROOT_DIR}/include
INCLUDEPATH += $${ROOT_DIR}


OBJECTS_DIR = $${BUILD_PATH}objects
MOC_DIR     = $${BUILD_PATH}mocs
UI_DIR      = $${BUILD_PATH}uics
RCC_DIR     = $${BUILD_PATH}rcc

LIBS +=  -L$${INSTALL_PATH_LIB}


QMAKE_CXXFLAGS += -ffast-math

CONFIG += debug

#build translation files
isEmpty(QMAKE_LRELEASE)
{
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

isEmpty(TRANSLATIONS){
updateqm.input = TRANSLATIONS
updateqm.output = $${TRANSLATE_PATH}/$${TARGET}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm $${TRANSLATE_PATH}/$${TARGET}.qm
updateqm.CONFIG += no_link

QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all
}

