QT -= core
CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO


INCLUDEPATH += $${SRC}/utils \
    $${TRD}/glog/include \
    $${UBT_LIB}/penmanship/include \
    $${TRD}/jsoncpp/include \
    $${UBT_LIB}/ubtlib/include \
    $${SRC}/plugins/charevaluation

SOURCES += \
        charevaluateclient.cpp \
        main.cpp

TARGET = wordevaluation


win32:CONFIG(release, debug|release) {
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/main/$${MAIN_VERSION}
    LIBS += -L$${DESTDIR} \
            -lutils

    LIBS += -L$${TRD}/glog/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -lglog
    LIBS += -L$${TRD}/jsoncpp/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -ljsoncpp
    LIBS += -L$${UBT_LIB}/ubtlib/lib \
            -levaluationwrapper
}

HEADERS += \
    charevaluateclient.h
