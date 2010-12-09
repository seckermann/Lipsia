PROJECT(LIBLIPSIA)

# configure source files
SET(LIBLIPSIA_SRC
    GeoInfo.c StatsConversions.c Talairach.c gsl_futils.c gsl_dutils.c via2gsl.c
    SmoothnessEstim.c wilcoxtables.c wilcox_pvals.c Convolve.c GaussConv.c
    GetVersion.c)

# add the shared library target 
ADD_LIBRARY(lipsia SHARED ${LIBLIPSIA_SRC})
TARGET_LINK_LIBRARIES(lipsia m ${VIAIO_LIBRARY} ${GSL_GSL_LIBRARY} ${GSL_CBLAS_LIBRARY})
# set properties for shared library
SET_TARGET_PROPERTIES(lipsia PROPERTIES 
	VERSION ${LIPSIA_SHARED_LIB_VERSION}
  SOVERSION ${LIPSIA_SHARED_LIB_VERSION}
	CLEAN_DIRECT_OUTPUT 1
	COMPILE_FLAGS -ansi
	LINK_FLAGS -Wl
	)

# add the static library target
ADD_LIBRARY(lipsia_static STATIC ${LIBLIPSIA_SRC})
TARGET_LINK_LIBRARIES(lipsia_static m ${VIAIO_LIBRARY} ${GSL_GSL_LIBRARY} ${GSL_CBLAS_LIBRARY})
SET_TARGET_PROPERTIES(lipsia_static PROPERTIES
	VERSION ${LIPSIA_SHARED_LIB_VERSION}
  SOVERSION ${LIPSIA_SHARED_LIB_VERSION}
	OUTPUT_NAME "lipsia"
	CLEAN_DIRECT_OUTPUT 1
	COMPILE_FLAGS -ansi
	LINK_FLAGS -Wl
	)

# install shared and static libraries
INSTALL(TARGETS lipsia lipsia_static
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
	ARCHIVE DESTINATION lib COMPONENT Development)

# install header files
FILE(GLOB LIPSIA_HDR "${CMAKE_SOURCE_DIR}/src/old/lib_lipsia/*.h")
INSTALL(FILES ${LIPSIA_HDR}
	DESTINATION include COMPONENT Development)
