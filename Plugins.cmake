set (DEFAULT_PLUGINS

# protocols
	# Facebook Chat protocol support
	facebook_protocol
	# GaduGadu protocol support
	gadu_protocol
	# Jabber/XMPP protocol support
	jabber_protocol

# notifiers
	# Enables notifications about buddies presence and other in chat windows
	chat_notify
	# Notifications by docking plugin
	docking_notify
	# Notification by external commands module
	exec_notify
	# Hints near tray icon
	hints
	# Speech synthesis support
	speech
	# PC Speaker notification support
	pcspeaker

# history
	# General history plugin
	history
	# Chat history storing, browsing and searching support using sqlite
	sql_history

# encryption
	# OTR encryption
	encryption_otr

# sound
	# General sound plugin
	sound
	# External sound player support
	ext_sound

# mediaplayer
	# Media players support plugin
	mediaplayer

# misc
	# Antistring
	antistring
	# Auto away module
	autoaway
	# Auto hide Kadu window
	auto_hide
	# Autoresponder plugin
	autoresponder
	# Autostatus
	autostatus
	# Cenzor
	cenzor
	# Configuration wizard
	config_wizard
	# Tray icon support
	docking
	# Displays graphical emoticons in chat window
	emoticons
	# Loads presence status messages from file
	filedesc
	# Protection against unwanted chats
	firewall
	# Idle time counter
	idle
	# Shows image links and youtube links as images and videos in chat
	imagelink
	# Last status infos
	last_seen
	# Easily take screenshots and send as images
	screenshot
	# Simple view - neww port for 0.11.0
	simpleview
	# Single window mode
	single_window
	# Sms gateway support
	sms
	# Spellchecking in chat window
	#spellchecker
	# Tabbed chat dialog
	tabs
	# Automatic mispelled word replacement
	word_fix

# integration
	# integration with Windows
	windows_integration
)

# Platform-speficic plugins

if (UNIX)
	list (APPEND DEFAULT_PLUGINS

	# mediaplayer
		# MPRIS Media Players support
		mprisplayer_mediaplayer
	)
endif (UNIX)

if (UNIX AND NOT APPLE)
	list (APPEND DEFAULT_PLUGINS

	# docking
		# Indicator docking support
		# Comment if you are not compilign under Ubuntu flavor
		#indicator_docking

	# integration
		# integration with Unity
		# Comment if you are not compilign under Ubuntu flavor
		#unity_integration

	# notifiers
		# Freedesktop notification support
		freedesktop_notify

	# mediaplayer
		# MPD mediaplayer support
		mpd_mediaplayer
	)
endif (UNIX AND NOT APPLE)

if (WIN32)
	list (APPEND DEFAULT_PLUGINS

	# mediaplayer
		# Winamp Media Player support
		winamp_mediaplayer
	)
endif (WIN32)

# Sort the list so plugins will be built in alphabetical order
list (SORT DEFAULT_PLUGINS)

# We must remember that the defaults may change and we want all Git users who didn't set
# COMPILE_PLUGINS by hand to always have current defaults.
# So if it is the very first run (and the user didn't manually specify COMPILE_PLUGINS)
# or last time default plugins were compiled and the user didn't change COMPILE_PLUGINS manually,
# pick the default plugins.
if ((NOT COMPILE_PLUGINS) OR (HAVE_DEFAULT_PLUGINS AND ("${OLD_COMPILE_PLUGINS}" STREQUAL "${COMPILE_PLUGINS}")))
	set (COMPILE_PLUGINS ${DEFAULT_PLUGINS})
	set (HAVE_DEFAULT_PLUGINS "TRUE" CACHE INTERNAL "" FORCE)
else ((NOT COMPILE_PLUGINS) OR (HAVE_DEFAULT_PLUGINS AND ("${OLD_COMPILE_PLUGINS}" STREQUAL "${COMPILE_PLUGINS}")))
	# Replace whitespace and commas with semicolons
	string (REGEX REPLACE "[ \t\n\r,]" ";" COMPILE_PLUGINS "${COMPILE_PLUGINS}")
	# Convert to list
	set (COMPILE_PLUGINS ${COMPILE_PLUGINS})

	# Remove empty entries
	list (REMOVE_ITEM COMPILE_PLUGINS "")

	# Sort the list locally so we can compare with the default
	set (_compile_plugins ${COMPILE_PLUGINS})
	list (SORT _compile_plugins)

	if ("${_compile_plugins}" STREQUAL "${DEFAULT_PLUGINS}")
		set (HAVE_DEFAULT_PLUGINS "TRUE" CACHE INTERNAL "" FORCE)
	else ("${_compile_plugins}" STREQUAL "${DEFAULT_PLUGINS}")
		set (HAVE_DEFAULT_PLUGINS "FALSE" CACHE INTERNAL "" FORCE)
	endif ("${_compile_plugins}" STREQUAL "${DEFAULT_PLUGINS}")
endif ((NOT COMPILE_PLUGINS) OR (HAVE_DEFAULT_PLUGINS AND ("${OLD_COMPILE_PLUGINS}" STREQUAL "${COMPILE_PLUGINS}")))

set (COMPILE_PLUGINS "${COMPILE_PLUGINS}" CACHE STRING "Plugins to compile" FORCE)
set (OLD_COMPILE_PLUGINS "${COMPILE_PLUGINS}" CACHE INTERNAL "" FORCE)
