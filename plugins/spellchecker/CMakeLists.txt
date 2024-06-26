project (spellchecker)

cmake_minimum_required (VERSION 3.0)

find_package (Kadu REQUIRED CONFIG)

set (SOURCES
	configuration/spellchecker-configuration.cpp
	highlighter.cpp
	spellchecker-configuration-ui-handler.cpp
	spellchecker-module.cpp
	spellchecker-plugin-modules-factory.cpp
	spellchecker-plugin-object.cpp
	spellchecker.cpp
	suggest-action.cpp
	suggester.cpp
)

set (CONFIGURATION_FILES
	configuration/spellchecker.ui
)

if (APPLE)
	list (APPEND SOURCES macspellchecker.mm)
	list (APPEND MOC_SOURCES macspellchecker.h)
else ()
	if (WITH_ASPELL AND NOT WITH_ENCHANT)
		set (USE_ASPELL TRUE)
		set (FORCE_ENGINE TRUE)
	else ()
		set (USE_ENCHANT TRUE)
		if (WITH_ENCHANT)
			set (FORCE_ENGINE TRUE)
		endif ()

		if (WITH_ASPELL)
			message (STATUS "Cannot use both enchant and aspell. Using enchant")
		endif ()
	endif ()

	if (USE_ENCHANT)
		find_package (PkgConfig)
		pkg_search_module (ENCHANT enchant-2)

		if (NOT ENCHANT_FOUND)
			if (FORCE_ENGINE)
				message (SEND_ERROR "enchant not found!")
			else ()
				message (STATUS "enchant not found. Falling back to aspell")
				set (USE_ENCHANT FALSE)
				set (USE_ASPELL TRUE)
			endif ()
		endif ()
	endif ()

	if (USE_ASPELL)
		include (FindASPELL)

		if (NOT ASPELL_FOUND)
			message (SEND_ERROR "aspell not found!")
		endif ()
	endif ()

	if (USE_ENCHANT)
		message (STATUS "Using enchant as backend. To force aspell use -DWITH_ASPELL=ON")
		add_definitions (-DHAVE_ENCHANT)

		set (LIBRARIES ${ENCHANT_LIBRARIES})
		link_directories (${ENCHANT_LIBRARY_DIRS})
		include_directories (${ENCHANT_INCLUDE_DIRS})
		add_definitions (${ENCHANT_DEFINITIONS})
	else ()
		message (STATUS "Using aspell as backend")
		add_definitions (-DHAVE_ASPELL)

		set (LIBRARIES ${ASPELL_LIBRARIES})
		link_directories (${ASPELL_LIBRARY_DIR})
		include_directories (${ASPELL_INCLUDE_DIR})
	endif ()
endif ()

kadu_plugin (spellchecker
	PLUGIN_SOURCES ${SOURCES}
	PLUGIN_CONFIGURATION_FILES ${CONFIGURATION_FILES}
	PLUGIN_LIBRARIES ${LIBRARIES}
)
