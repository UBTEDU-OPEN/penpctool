# add common infomation
QT += core network sql
TEMPLATE = lib

# add dll define
DEFINES += UTILS_LIB

DEFINES += GLOG_NO_ABBREVIATED_SEVERITIES SHOW_TEST_BTN

DEFINES += STR_MAIN_VERSION=$${MAIN_VERSION}
DEFINES += STR_CHAREVALUATION_VERSION=$${CHAREVALUATION_VERSION}
DEFINES += STR_HANDWRITTEN_VERSION=$${HANDWRITTEN_VERSION}

QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

# add dll include path
INCLUDEPATH += \
    $${TRD}/glog/include \
    $${TRD}/aly/include \
    $${UBT_LIB}/ubtlib/include


# add source/header/form/resource file
SOURCES += \
    Classwork.cpp \
    alyservice.cpp \
    cmd5.cpp \
    config.cpp \
    fileDirHandler.cpp \
    getportuitls.cpp \
    httpworker.cpp \
    logHelper.cpp \
    md5.cpp \
    ubtserver.cpp

HEADERS += utilsGlobal.h \
    Classwork.h \
    IPCProtocol.h \
    alyservice.h \
    cmd5.h \
    commonMacro.h \
    commondefines.h \
    config.h \
    fileDirHandler.h \
    getportuitls.h \
    httpworker.h \
    logHelper.h \
    md5.h \
    projectconst.h \
    ubtserver.h

win32:CONFIG(release, debug|release) {
    TARGET = utils
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/main/$${MAIN_VERSION}
    LIBS += -L$${TRD}/glog/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -lglog
    LIBS += -L$${TRD}/aly/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -lalibabacloud-oss-cpp-sdk
    LIBS += -L$${TRD}/aly/third_party/lib/$${ARCH_NAME} \
            -llibcurl -llibeay32 -lssleay32
    LIBS += -lWS2_32 \
            -lIPHlpApi
    LIBS += -L$${UBT_LIB}/ubtlib/lib \
            -lportlib -lubtlib
}


