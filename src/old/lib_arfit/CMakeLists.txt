PROJECT(LIBARFIT)

# configure source files
SET(LIBARFIT_SRC
    arfit.c arfit_schloegl.c arfit_via.c arfit_error.c  arfit_schneider.c
    gsl_matrix_wrapper.c)

# the shared library
ADD_LIBRARY(arfit SHARED ${LIBARFIT_SRC})
TARGET_LINK_LIBRARIES(arfit m ${GSL_GSL_LIBRARY} ${VIAIO_LIBRARY})

SET_TARGET_PROPERTIES(arfit PROPERTIES
                      VERSION ${LIPSIA_SHARED_LIB_VERSION}
                      SOVERSION ${LIPSIA_SHARED_LIB_VERSION}
                      CLEAN_DIRECT_OUTPUT 1
                      COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl
                     )

# the static library
ADD_LIBRARY(arfit_static STATIC ${LIBARFIT_SRC})
SET_TARGET_PROPERTIES(arfit_static PROPERTIES
                      VERSION ${LIPSIA_SHARED_LIB_VERSION}
                      SOVERSION ${LIPSIA_SHARED_LIB_VERSION}
                      CLEAN_DIRECT_OUTPUT 1
                      COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl
                      OUTPUT_NAME "arfit"
                     )

INSTALL(TARGETS arfit arfit_static
        LIBRARY DESTINATION ${LIB_INSTALL_PATH} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${LIB_INSTALL_PATH} COMPONENT Development
       )

# install header files
FILE(GLOB ARFIT_HDR "${CMAKE_SOURCE_DIR}/src/lib_arfit/*.h")
INSTALL(FILES ${ARFIT_HDR}
        DESTINATION ${LIPSIA_INSTALL_INCLUDE_DIR} COMPONENT Development)
