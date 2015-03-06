TEMPLATE = app
TARGET = MKVSegmentJoiner
QT += core \
    gui
HEADERS += MyListWidget.h \
    MKVSegmentJoiner.h
SOURCES += MyListWidget.cpp \
    main.cpp \
    MKVSegmentJoiner.cpp
FORMS += MKVSegmentJoiner.ui
RESOURCES += 
greaterThan(QT_MAJOR_VERSION, 4) { # QT5+
    QT += widgets # for all widgets
    win32-msvc*:DEFINES += NOMINMAX
}