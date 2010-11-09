PROJECT(isisdump)
add_executable(isisdump isisdump.cpp)

target_link_libraries(isisdump ${ISIS_DIR}/lib/libisis_core.a ${Boost_LIBRARIES} dl)

SET_TARGET_PROPERTIES(isisdump PROPERTIES
	    LINK_FLAGS -Wl)

INSTALL(TARGETS isisdump
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
					
