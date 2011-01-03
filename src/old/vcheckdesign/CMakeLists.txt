PROJECT(vcheckdesign)

INCLUDE_DIRECTORIES($ {QT_INCLUDE_DIR} .)

QT_WRAP_CPP(vcheckdesign VCHECKDESIGN_MOC
            design.h designCW.h bild1.h bild2.h bild3.h)

ADD_EXECUTABLE(vcheckdesign
               design.C designCW.C bild1.C bild2.C bild3.C $ {VCHECKDESIGN_MOC})

TARGET_LINK_LIBRARIES(vcheckdesign $ {QT_LIBRARIES} $ {FFTW3_FFTW3_LIBRARY} lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vcheckdesign PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vcheckdesign
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
