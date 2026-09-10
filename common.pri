#defines
PROJECT_NAME           = Q_ZodEngine

ROOT_DIR               = $$PWD
INSTALL_PATH_BIN       = $${ROOT_DIR}/bin
TRANSLATE_PATH         = $${INSTALL_PATH_BIN}/translate
INSTALL_PATH_LIB       = $${INSTALL_PATH_BIN}/lib
INSTALL_PATH_INCLUDE   = $${ROOT_DIR}/include



# --- data folders --------------------
INSTALL_PATH_DATA      = $${ROOT_DIR}/Data
INSTALL_PATH_LOG       = $${INSTALL_PATH_DATA}/LOG
INSTALL_PATH_SAVE      = $${INSTALL_PATH_DATA}/Save

INSTALL_PATH_TMP           = $${INSTALL_PATH_DATA}/tmp
INSTALL_PATH_DEBUG         = $${INSTALL_PATH_DATA}/debug
INSTALL_PATH_SNAPSHOT      = $${INSTALL_PATH_DATA}/splash
INSTALL_PATH_VIDEO         = $${INSTALL_PATH_DATA}/video


BUILD_PATH = ./
ARCH = ./

INCLUDEPATH += $${ROOT_DIR}/include
INCLUDEPATH += $${ROOT_DIR}

# Windows builds use the dependency root prepared by build_game.bat. Keeping
# this in common.pri makes the old SDL/MySQL linker declarations in every
# subproject resolve without duplicating machine-specific paths.
win32 {
    # The legacy engine passes narrow std::string/char* paths to Win32 APIs.
    # Qt's MinGW mkspec enables UNICODE by default, which remaps calls such as
    # FindFirstFile to their wide-character variants and breaks those call sites.
    # Keep the legacy Win32 API aliases ANSI until path handling is migrated.
    DEFINES -= UNICODE _UNICODE

    QZOD_DEPS_ROOT = $$(QZOD_DEPS_ROOT)
    !isEmpty(QZOD_DEPS_ROOT) {
        INCLUDEPATH += "$${QZOD_DEPS_ROOT}/include"
        INCLUDEPATH += "$${QZOD_DEPS_ROOT}/include/mariadb"
        LIBS += -L"$${QZOD_DEPS_ROOT}/lib"
    }
}


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

