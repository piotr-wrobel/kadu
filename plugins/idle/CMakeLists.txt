project (idle)

cmake_minimum_required (VERSION 3.0)

find_package (Kadu REQUIRED CONFIG)

kadu_api_directories (plugins/idle
	.
)

set (SOURCES
	idle-module.cpp
	idle-plugin-modules-factory.cpp
	idle-plugin-object.cpp
	idle.cpp
)

if (UNIX AND NOT APPLE)
	find_package (X11 REQUIRED COMPONENTS Xutil Xscreensaver)
	if (X11_Xscreensaver_FOUND)
		message (STATUS "Found required X11 libraries: ${X11_X11_LIB};${X11_Xscreensaver_LIB}")
	else ()
		message (SEND_ERROR "Could NOT find X11 XScrnSaver")
	endif ()

	include_directories (${X11_INCLUDE_DIR})
	set (LIBRARIES ${X11_X11_LIB} ${X11_Xscreensaver_LIB})

	list (APPEND SOURCES
		idle-x11.cpp
	)
endif ()

if (WIN32)
	list (APPEND SOURCES
		idle-win.cpp
	)
endif ()

add_definitions (-Didle_EXPORTS)

kadu_plugin (idle
	PLUGIN_SOURCES ${SOURCES}
	PLUGIN_LIBRARIES ${LIBRARIES}
)
