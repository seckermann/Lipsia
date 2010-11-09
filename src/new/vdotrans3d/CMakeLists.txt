PROJECT(vdotrans3d)

ADD_EXECUTABLE(vdotrans3d isisdotrans3d.cxx isisTransformMerger3D.cpp)

TARGET_LINK_LIBRARIES(vdotrans3d ITKIO viaio ${Boost_LIBRARIES} ${ISIS_DIR}/lib/libisis_core.a  )

SET_TARGET_PROPERTIES(vdotrans3d PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vdotrans3d
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)

