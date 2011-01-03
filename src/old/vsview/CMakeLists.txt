PROJECT(vsview)

INCLUDE_DIRECTORIES($ {QT_INCLUDE_DIR})

QT_WRAP_CPP(vsview VSVIEW_MOC
            xv.h CW.h bild1.h attrWin.h preferences.h)

ADD_EXECUTABLE(vsview
               xv.C CW.C bild1.C attrWin.C preferences.C $ {VSVIEW_MOC})
TARGET_LINK_LIBRARIES(vsview $ {QT_LIBRARIES} lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vsview PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vsview
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
