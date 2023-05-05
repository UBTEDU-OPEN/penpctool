QT -= gui

TEMPLATE = lib
#CONFIG += plugin

CONFIG += c++14

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += GLOG_NO_ABBREVIATED_SEVERITIES

QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

INCLUDEPATH += $${SRC}/evaluation \
    $${UBT_LIB}/ubtlib/include

SOURCES += \
    charevaluationplugin.cpp

HEADERS += charevaluationinterface.h \
    charevaluationplugin.h

TARGET = charevaluationplugin


win32:CONFIG(release, debug|release) {
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/charevaluation/$${CHAREVALUATION_VERSION}

    LIBS += -L$${UBT_LIB}/ubtlib/lib \
            -levaluationwrapper
}

DISTFILES += charevaluation.json

QMAKE_POST_LINK += \
    copy /y            \"$${SRC}\plugins\charevaluation\charevaluation.json\"                       \"$${DESTDIR}\" && \
    xcopy /y /s /e /i  \"$${DESTDIR}\*.*\"   \"$${DESTDIR}\..\..\main\\$${MAIN_VERSION}\"

