QT -= gui

TEMPLATE = lib
#CONFIG += plugin

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += GLOG_NO_ABBREVIATED_SEVERITIES


QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO


INCLUDEPATH += $${UBT_LIB}/ubtlib/include

TARGET = handwrittenplugin


win32:CONFIG(release, debug|release) {
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/handwritten/$${HANDWRITTEN_VERSION}

    LIBS += -L$${UBT_LIB}/ubtlib/lib \
            -lhandwrittenwrapper
}

SOURCES += \
    handwrittenplugin.cpp

HEADERS += handwritteninterface.h \
    handwrittenplugin.h

DISTFILES += handwritten.json

QMAKE_POST_LINK += \
    copy /y            \"$${SRC}\plugins\handwritten\handwritten.json\"                       \"$${DESTDIR}\" && \
    xcopy /y /s /e /i  \"$${DESTDIR}\*.*\"   \"$${DESTDIR}\..\..\main\\$${MAIN_VERSION}\"
