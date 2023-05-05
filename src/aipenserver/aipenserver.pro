CONFIG += c++11
#CONFIG -= app_bundle
QT += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_DEPRECATED_WARNINGS GLOG_NO_ABBREVIATED_SEVERITIES SHOW_TEST_BTN

QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

TARGET = aipenserver

INCLUDEPATH += $${SRC}/utils \
    $${SRC}/plugins/handwritten \
    $${TRD}/glog/include \
    $${TRD}/jsoncpp/include \
    $${TRD}/TQL/include \
    $${TRD}/CrashRpt/include \
    $${UBT_LIB}/charevaluation/include \
    $${UBT_LIB}/penmanship/include \
    $${UBT_LIB}/wordevaluatetranslation/include \
    $${UBT_LIB}/ubtlib/include

SOURCES += \
        aipen.cpp \
        aipenmanager.cpp \
        aipenserver.cpp \
        bookinfo.cpp \
        charevaluateserver.cpp \
        evaluaterequestworker.cpp \
        exceptionhandler.cpp \
        localxmlworker.cpp \
        main.cpp \
        ocrcharhandler.cpp \
        origindatasaver.cpp \
        pendingwordhandlethread.cpp \
        pendothandler.cpp \
        postdotthread.cpp \
        readevaluateresultthread.cpp \
        tqlsdkadapter.cpp \
        uploaddatathread.cpp \
        uploadorigindataworker.cpp

win32:CONFIG(release, debug|release) {
    DESTDIR = $${BIN}/$${ARCH_NAME}/$${MODE_NAME}/aipenserver/$${AIPEN_SERVER_VERSION}
    LIBS += -L$${BIN}/$${ARCH_NAME}/$${MODE_NAME}/main/$${MAIN_VERSION} \
            -lutils \
            -L$${TRD}/TQL/lib/$${ARCH_NAME} \
            -lTQLAPComm
    LIBS += -L$${TRD}/glog/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -lglog
    LIBS += -L$${TRD}/jsoncpp/lib/$${ARCH_NAME}/$${MODE_NAME} \
            -ljsoncpp

    LIBS += -L$${UBT_LIB}/penmanship/$${ARCH_NAME}/$${MODE_NAME} \
            -lpenmanship
    LIBS += -L$${UBT_LIB}/ubtlib/lib \
            -lportlib -lubtlib -levaluationwrapper
}

HEADERS += \
    aipen.h \
    aipenmanager.h \
    aipenserver.h \
    bookinfo.h \
    charevaluateserver.h \
    evaluaterequestworker.h \
    exceptionhandler.h \
    localxmlworker.h \
    ocrcharhandler.h \
    origindatasaver.h \
    pendingwordhandlethread.h \
    pendothandler.h \
    postdotthread.h \
    readevaluateresultthread.h \
    tqlsdkadapter.h \
    uploaddatathread.h \
    uploadorigindataworker.h

DISTFILES += aipenserver.json

QMAKE_POST_LINK += \
    copy /y            \"$${SRC}\aipenserver\aipenserver.json\"                       \"$${DESTDIR}\" && \
    xcopy /y /s /e /i  \"$${DESTDIR}\*.*\"   \"$${DESTDIR}\..\..\main\\$${MAIN_VERSION}\" && \
    $$(QTDIR)\bin\windeployqt.exe $${DESTDIR}\..\..\main\\$${MAIN_VERSION}\\$${TARGET}.exe
