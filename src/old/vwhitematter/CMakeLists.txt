PROJECT(vwhitematter)

ADD_EXECUTABLE(vwhitematter
               LocalContrast.c TopSmooth.c vwhitematter.c Lee3d.c VCheckPlane.c)
TARGET_LINK_LIBRARIES(vwhitematter lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vwhitematter PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vwhitematter
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
