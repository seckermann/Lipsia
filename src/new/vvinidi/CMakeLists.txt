PROJECT(vvinidi)
add_executable(vvinidi isisconv.cpp)

target_link_libraries(vvinidi ${ISIS_DIR}/lib/libisis_core.a ${Boost_LIBRARIES} dl)

SET_TARGET_PROPERTIES(vvinidi PROPERTIES
	    LINK_FLAGS -Wl)

INSTALL(TARGETS vvinidi
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
					
