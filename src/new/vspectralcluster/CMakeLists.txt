PROJECT(vspectralcluster)

ADD_EXECUTABLE(vspectralcluster Eigen.c KMeans.c LaplacianMatrix.c vspectralcluster.c)
TARGET_LINK_LIBRARIES(vspectralcluster /usr/lib/libf2c.a /usr/lib/libgsl.a /usr/lib/liblapack-3.a /usr/lib/libatlas.a ${VIA_LIBRARY} ${BLAS_LIBRARY} ${LIPSIA_LIBRARY} )

SET_TARGET_PROPERTIES(vspectralcluster PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vspectralcluster
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
