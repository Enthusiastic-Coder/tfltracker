INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
QT += xml network

qtsoap-uselib:!qtsoap-buildlib {
    LIBS += -L$$QTSOAP_LIBDIR -l$$QTSOAP_LIBNAME
} else {
    SOURCES += $$PWD/qtsoap.cpp
    HEADERS += $$PWD/qtsoap.h
}

win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QtSOAP_EXPORTS
    else:qtsoap-uselib:DEFINES += QT_QTSOAP_IMPORT
}
