QT += core gui
QT += network websockets sql
TEMPLATE = lib
DEFINES += DEVICECOMMON_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS GLOG_NO_ABBREVIATED_SEVERITIES SHOW_TEST_BTN

QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $${SRC}/utils \
    $${TRD}/glog/include \
    $${TRD}/TQL/include \
    $${TRD}/jsoncpp/include \
    $${UBT_LIB}/charevaluation/include \
    $${UBT_LIB}/penmanship/include

SOURCES += \
    aipenclient.cpp \
    ap.cpp \
    apmanager.cpp \
    pingipthread.cpp

HEADERS += \
    DeviceCommon_global.h \
    aipenclient.h \
    ap.h \
    apmanager.h \
    pingipthread.h

win32:CONFIG(debug, debug|release) {
    TARGET = devicecommond
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/main/$${MAIN_VERSION}
    LIBS += -L$${DESTDIR} \
            -lutilsd \
            -L$${TRD}/TQL/lib/$${ARCH_NAME} \
            -lTQLAPComm

    LIBS += -L$${TRD}/glog/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -lglogd
    LIBS += -L$${TRD}/jsoncpp/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -ljsoncpp
}

win32:CONFIG(release, debug|release) {
    TARGET = devicecommon
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/main/$${MAIN_VERSION}
    LIBS += -L$${DESTDIR} \
            -lutils \
            -L$${TRD}/TQL/lib/$${ARCH_NAME} \
            -lTQLAPComm

    LIBS += -L$${TRD}/glog/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -lglog
    LIBS += -L$${TRD}/jsoncpp/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -ljsoncpp
}
