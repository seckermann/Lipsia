PROJECT(vdotrans3d)

ADD_EXECUTABLE(vdotrans3d isisdotrans3d.cxx isisTransformMerger3D.cpp isisTimeStepExtractionFilter.hpp )

# TARGET_LINK_LIBRARIES(vdotrans3d ITKIO viaio ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} /usr/lib/liboil-0.3.a )
TARGET_LINK_LIBRARIES(vdotrans3d lipsia ${ITK_LIBRARIES} ${VIAIO_LIBRARY} ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${LIPSIA_LIBRARY} )

SET_TARGET_PROPERTIES(vdotrans3d PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vdotrans3d
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)

