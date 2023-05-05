#The following code works on Windows (at least with all the recent MSVC compilers - didn't test MinGW), Mac OS X (clang) and Linux (GCC).
#Feel free to omit the first clause and refer to QT_ARCH directly if you don't need Qt 4 support.
greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}

contains(TARGET_ARCH, x86_64) {
    message("64 bit")
    ARCH_NAME = win64
} else {
    message("32 bit")
    ARCH_NAME = win32
}

CONFIG(release, debug|release) {
    message("release")
    MODE_NAME = release
} else {
    message("debug")
    MODE_NAME = debug
}

BASE    = $$PWD/../..
BIN     = $${BASE}/bin
SRC     = $${BASE}/src
TRD     = $${SRC}/trd
UBT_LIB = $${SRC}/libs
MAIN_VERSION = 1.6.0.79
AIPEN_SERVER_VERSION=1.4.0.64
HANDWRITTEN_VERSION=1.1.0.64
CHAREVALUATION_VERSION=1.6.1.65

cache(MODE_NAME, set, MODE_NAME)
cache(ARCH_NAME, set, ARCH_NAME)
cache(BASE, set, BASE)
cache(BIN, set, BIN)
cache(SRC, set, SRC)
cache(TRD, set, TRD)
cache(UBT_LIB, set, UBT_LIB)
cache(MAIN_VERSION, set, MAIN_VERSION)
cache(HANDWRITTEN_VERSION, set, HANDWRITTEN_VERSION)
cache(CHAREVALUATION_VERSION, set, CHAREVALUATION_VERSION)
cache(AIPEN_SERVER_VERSION, set, AIPEN_SERVER_VERSION)

export (MODE_NAME)
export (ARCH_NAME)
export (BASE)
export (SRC)
export (TRD)
export (UBT_LIB)
export (MAIN_VERSION)
export (HANDWRITTEN_VERSION)
export (CHAREVALUATION_VERSION)
export (AIPEN_SERVER_VERSION)

