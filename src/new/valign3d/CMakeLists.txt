PROJECT(valign3d)

ADD_EXECUTABLE(valign3d isisreg3d.cxx isisRegistrationFactory3D.hpp)
TARGET_LINK_LIBRARIES(valign3d ITKIO ITKStatistics ITKNumerics viaio ${ISIS_DIR}/lib/libisis_core.a  ${Boost_LIBRARIES})

SET_TARGET_PROPERTIES(valign3d PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS valign3d
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
