project (tabs)

cmake_minimum_required (VERSION 3.0)

find_package (Kadu REQUIRED CONFIG)

set (SOURCES
	gui/widgets/tab-bar.cpp
	gui/widgets/tab-widget.cpp
	attach-tab-action.cpp
	open-in-new-tab-action.cpp
	tabs-chat-widget-container-handler.cpp
	tabs-module.cpp
	tabs-plugin-modules-factory.cpp
	tabs-plugin-object.cpp
	tabs.cpp
)

set (CONFIGURATION_FILES
	configuration/tabs.ui
)

if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	include_directories (${X11_X11_INCLUDE_PATH})
endif ()

kadu_plugin (tabs
	PLUGIN_SOURCES ${SOURCES}
	PLUGIN_CONFIGURATION_FILES ${CONFIGURATION_FILES}
)
