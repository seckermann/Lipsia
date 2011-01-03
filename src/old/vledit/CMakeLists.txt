PROJECT(vledit)

# need to include the local directory in the include path due to the
# inappropriate of '#include<>' instead of '#include ""' for local headers
INCLUDE_DIRECTORIES($ {QT_INCLUDE_DIR} . ../lib_vlh ../lib_vlh/xpm)

QT_WRAP_UI(vledit VLEDIT_UI_H VLEDIT_UI_SRC
           aboutdialog.ui mainwindow.ui segmentwindow.ui)

QT_WRAP_CPP(vledit VLEDIT_MOC
            vlglwidget.h vlcolorboxtableitem.h uiconfig.h vlmainwindow.h
            vlserverconnection.h vltable.h vlvisibleboxtableitem.h vlsavereportdialog.h
            vlsegmentwindow.h)

ADD_EXECUTABLE(vledit
               main.cpp datamanager.cpp
               point.cpp vistaimage.cpp vlmainwindow.cpp vlbrainplane.cpp vlglwidget.cpp
               vlcamera.cpp vlaim.cpp vlsavereportdialog.cpp vltable.cpp
               vlvisibleboxtableitem.cpp vlsegmentwindow.cpp vlcolorboxtableitem.cpp
               uiconfig.cpp vlserverconnection.cpp $ {VLEDIT_MOC} $ {VLEDIT_UI_SRC})

TARGET_LINK_LIBRARIES(vledit $ {QT_LIBRARIES} $ {OPENGL_gl_LIBRARY} lipsia $ {VIA_LBIRARY})

SET_TARGET_PROPERTIES(vledit PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vledit
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
