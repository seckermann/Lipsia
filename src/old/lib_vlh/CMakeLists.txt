PROJECT(LIBVLH)

# configure source files
SET(LIBVLH_SRC
    trilinear.C VLTools.C VLCorr.C showVImage.C pictureView.C prefs.C lView.C
    lLoad.C timecourse.C flow.C dialog.C bildWin.C preferences.C power.C
    rawCW.C rawpaint.C VCheck.C fenster1.C rawplot.C openGL.C bilderCW.C
    interpolator.cc)

QT_WRAP_CPP(vlh LIBVLH_MOC
            pictureView.h dialog.h bildWin.h preferences.h fenster1.h rawplot.h
            rawpaint.h rawCW.h openGL.h bilderCW.h lView.h VLCorr.h)

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR} xpm)

# the shared library
ADD_LIBRARY(vlh SHARED ${LIBVLH_SRC} ${LIBVLH_MOC})
TARGET_LINK_LIBRARIES(vlh
                      ${QT_LIBRARIES}
                      ${OPENGL_gl_LIBRARY}
                      ${OPENGL_glu_LIBRARY}
                      ${FFTW3_FFTW3_LIBRARY}
                      lipsia)

# this libraries contains undefined symbols by design (e.g. 'pr')
# therefore it cannot be linked with '--no-undefined' !!
SET_TARGET_PROPERTIES(vlh
                      PROPERTIES COMPILE_FLAGS -ansi
                      VERSION ${LIPSIA_SHARED_LIB_VERSION}
                      SOVERSION ${LIPSIA_SHARED_LIB_VERSION}
                      CLEAN_DIRECT_OUTPUT 1
                     )

# the static library
ADD_LIBRARY(vlh_static STATIC ${LIBVLH_SRC} ${LIBVLH_MOC})
SET_TARGET_PROPERTIES(vlh_static
                      PROPERTIES COMPILE_FLAGS -ansi
                      VERSION ${LIPSIA_SHARED_LIB_VERSION}
                      SOVERSION ${LIPSIA_SHARED_LIB_VERSION}
                      OUTPUT_NAME "vlh"
                      CLEAN_DIRECT_OUTPUT 1
                     )

# install the shared and static libraries
INSTALL(TARGETS vlh vlh_static
        LIBRARY DESTINATION ${LIB_INSTALL_PATH} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${LIB_INSTALL_PATH} COMPONENT Development
       )

# install header files
FILE(GLOB VLH_HDR "${CMAKE_SOURCE_DIR}/src/lib_vlh/*.h")
INSTALL(FILES ${VLH_HDR}
        DESTINATION ${LIPSIA_INSTALL_INCLUDE_DIR} COMPONENT Development)
