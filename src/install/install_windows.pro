# 拷贝第三方动态库至程序运行目录下
QMAKE_POST_LINK += \
    xcopy /y /s /e /i  \"$${TRD}\msvcp\\$${ARCH_NAME}\\$${MODE_NAME}\*.dll\"             \"$${DESTDIR}\"                 && \
    xcopy /y /s /e /i  \"$${TRD}\glog\lib\\$${ARCH_NAME}\\$${MODE_NAME}\*.dll\"          \"$${DESTDIR}\"                 && \
    xcopy /y /s /e /i  \"$${TRD}\TQL\lib\\$${ARCH_NAME}\*.dll\"                          \"$${DESTDIR}\"                 && \
    xcopy /y /s /e /i  \"$${TRD}\aly\third_party\lib\\$${ARCH_NAME}\*.dll\"   \"$${DESTDIR}\"       && \
    xcopy /y /s /e /i  \"$${TRD}\jsoncpp\lib\\$${ARCH_NAME}\\$${MODE_NAME}\*.dll\"   \"$${DESTDIR}\"       && \
    xcopy /y /s /e /i  \"$${UBT_LIB}\penmanship\\$${ARCH_NAME}\\$${MODE_NAME}\*.*\"          \"$${DESTDIR}\"            && \
    xcopy /y /s /e /i  \"$${UBT_LIB}\ubtlib\bin\*.*\"          \"$${DESTDIR}\"            && \
    xcopy /y /s /e /i  \"$${TRD}\7z\\$${ARCH_NAME}\*.*\"                                \"$${DESTDIR}\"


# 拷贝资源文件
QMAKE_POST_LINK += && \
    xcopy /y /s /e /i  \"$${SRC}\configs\*.*\"                               \"$${DESTDIR}\configs\"                     && \
    xcopy /y /s /e /i  \"$${BASE}\LICENSES\*.*\"                               \"$${DESTDIR}\LICENSES\"                     && \
    xcopy /y /s /e /i  \"$${BASE}\others\*.*\"                       \"$${DESTDIR}\"

CONFIG(release, debug|release) {
QMAKE_POST_LINK += && \
    copy  /y            \"$$(QTDIR)\bin\Qt5Sql.dll\"                        \"$${DESTDIR}\*.dll\"
}

QMAKE_POST_LINK += && \
    $$(QTDIR)\bin\windeployqt.exe $${DESTDIR}\\$${TARGET}.exe

