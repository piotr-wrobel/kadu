project (last_seen)

cmake_minimum_required (VERSION 3.0)

find_package (Kadu REQUIRED CONFIG)

set (SOURCES
	infos.cpp
	infos_dialog.cpp
	last-seen-module.cpp
	last-seen-plugin-modules-factory.cpp
	last-seen-plugin-object.cpp
	show-infos-window-action.cpp
)

kadu_plugin (last_seen
	PLUGIN_SOURCES ${SOURCES}
	PLUGIN_CONFIGURATION_FILES ${CONFIGURATION_FILES}
)
