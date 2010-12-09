PROJECT(vreg3d)

ADD_EXECUTABLE(vreg3d
    vreg3d.c Reg3d.c Refine.c IniShift.c GetTransf.c Orientation.c)
TARGET_LINK_LIBRARIES(vreg3d lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vreg3d PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vreg3d
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
