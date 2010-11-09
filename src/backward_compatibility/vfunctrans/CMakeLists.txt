PROJECT(vfunctrans)

ADD_EXECUTABLE(vfunctrans
    Functrans.c  SliceOrientation.c  vfunctrans.c FuncCompress.c
    TransTimeStep.c)
TARGET_LINK_LIBRARIES(vfunctrans lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vfunctrans PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vfunctrans
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
