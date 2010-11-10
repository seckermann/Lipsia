PROJECT(vdump)
add_executable(vdump isisdump.cpp)

target_link_libraries(vdump ${ISIS_DIR}/lib/libisis_core.a ${Boost_LIBRARIES} dl)

SET_TARGET_PROPERTIES(vdump PROPERTIES
	    LINK_FLAGS -Wl)

INSTALL(TARGETS vdump
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
					
