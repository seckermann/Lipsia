PROJECT(valign3d)

ADD_EXECUTABLE(valign3d isisreg3d.cxx isisRegistrationFactory3D.hpp isisTimeStepExtractionFilter.hpp )
# TARGET_LINK_LIBRARIES(valign3d ITKStatistics ITKNumerics viaio ${ISIS_LIB} ${ISIS_LIB_DEPENDS} /usr/lib/liboil-0.3.a)
TARGET_LINK_LIBRARIES(valign3d ${ITK_LIBRARIES} lipsia ${VIAIO_LIBRARY} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})

SET_TARGET_PROPERTIES(valign3d PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS valign3d
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
