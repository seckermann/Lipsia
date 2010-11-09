#qmake project file

LIBS += /usr/lib/libvia.a \
        -lvista \
        /usr/lib/liblipsia.a
INCLUDEPATH += /usr/include/via
OBJECTS_DIR = ../bin
QMAKE_CXXFLAGS_DEBUG += -ansi
DEFINES = DEBUG_DLG
TARGET = vledit
DESTDIR = ../bin
CONFIG += debug \
          warn_on \
          qt \
          thread \
          opengl \
          x11
TEMPLATE = app
FORMS += mainwindow.ui \
			segmentwindow.ui \
			aboutdialog.ui
HEADERS += datamanager.h \
           vistaimage.h \
           point.h \
           vlmainwindow.h \
           vlbrainplane.h \
           vlglwidget.h \
           vlcamera.h \
           vlaim.h \
           uiconfig.h \
           vlserverconnection.h \
           vlsegmentwindow.h \
           vlcolorboxtableitem.h \
           vlvisibleboxtableitem.h \
           vltable.h \
           vlsavereportdialog.h
SOURCES += datamanager.cpp \
           vistaimage.cpp \
           point.cpp \
           main.cpp \
           vlmainwindow.cpp \
           vlbrainplane.cpp \
           vlglwidget.cpp \
           vlcamera.cpp \
           vlaim.cpp \
           uiconfig.cpp \
           vlserverconnection.cpp \
           vlsegmentwindow.cpp \
           vlcolorboxtableitem.cpp \
           vlvisibleboxtableitem.cpp \
           vltable.cpp \
           vlsavereportdialog.cpp
