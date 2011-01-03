PROJECT(vqview)

# need to include the local directory in the include path due to the
# inappropriate use of '#include<>' instead of '#include ""' for local
# headers.
INCLUDE_DIRECTORIES($ {QT_INCLUDE_DIR} .)

QT_WRAP_CPP(vqview VQVIEW_MOC
            CentralWidget.h Configurator.h ImageWidget.h MainWindow.h
            PreferenceDialog.h plotwidget.h plotdialog.h)

ADD_EXECUTABLE(vqview
               main.cpp VImageManager.cpp interpolator.cc CentralWidget.cpp
               Configurator.cpp ImageWidget.cpp plotwidget.cpp plotdialog.cpp
               PreferenceDialog.cpp MainWindow.cpp $ {VQVIEW_MOC})
TARGET_LINK_LIBRARIES(vqview $ {QT_LIBRARIES} lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vqview PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vqview
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
