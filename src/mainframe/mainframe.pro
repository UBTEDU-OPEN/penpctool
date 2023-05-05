QT += core gui widgets

QT += websockets sql

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS SHOW_TEST_BTN

QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

INCLUDEPATH += $${SRC}/utils \
    $${TRD}/glog/include \
    $${TRD}/TQL/include \
    $${TRD}/jsoncpp/include \
    $${UBT_LIB}/ubtlib/include

DEFINES += GLOG_NO_ABBREVIATED_SEVERITIES

SOURCES += \
    aboutdialog.cpp \
    aipenclient.cpp \
    ap.cpp \
    apmanager.cpp \
    customstyledmenu.cpp \
    evaluationcheckthread.cpp \
    exceptionhandler.cpp \
    main.cpp \
    mainframe.cpp \
    messagehandler.cpp \
    moduleinfo.cpp \
    moduleupdateworker.cpp \
    popupdialog.cpp \
    promptdialog.cpp \
    testwidget.cpp \
    upgradeprocessor.cpp \
    upgradeworker.cpp \
    warningdialog.cpp \
    websocketserver.cpp

HEADERS += \
    aboutdialog.h \
    aipenclient.h \
    ap.h \
    apmanager.h \
    customstyledmenu.h \
    evaluationcheckthread.h \
    exceptionhandler.h \
    mainframe.h \
    messagehandler.h \
    moduleinfo.h \
    moduleupdateworker.h \
    popupdialog.h \
    promptdialog.h \
    testwidget.h \
    upgradeprocessor.h \
    upgradeworker.h \
    warningdialog.h \
    websocketserver.h


FORMS += \
    aboutdialog.ui \
    mainframe.ui \
    promptdialog.ui \
    testdialog.ui \
    upgradedialog.ui \
    warningdialog.ui

TRANSLATIONS += \
    mainframe_zh_CN.ts

win32:CONFIG(release, debug|release) {
    TARGET = penpctool
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/main/$${MAIN_VERSION}
    LIBS += -L$${DESTDIR} \
            -lutils
    LIBS += -L$${TRD}/glog/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -lglog
    LIBS += -L$${TRD}/jsoncpp/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -ljsoncpp
    LIBS += -L$${TRD}/TQL/lib/$${ARCH_NAME} \
            -lTQLAPComm

    LIBS += -L$${UBT_LIB}/ubtlib/lib \
            -lportlib -lubtlib
}

LIBS += -luser32 -lgdi32

DISTFILES += $${SRC}/configs/config.ini

include(../install/install.pro)
include(../qtsingleapplication/src/qtsingleapplication.pri)
RC_ICONS = aipen.ico

RESOURCES += \
    res.qrc


