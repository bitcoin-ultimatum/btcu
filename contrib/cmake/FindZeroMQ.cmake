# Copyright (c) 2017-2020 The Bitcoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# .rst:
# FindZeroMQ
# --------------
#
# Find the ZeroMQ library. The following conponents are
# available::
#   zmq
#
# This will define the following variables::
#
#   ZeroMQ_FOUND - True if the ZeroMQ library is found.
#   ZeroMQ_INCLUDE_DIRS - List of the header include directories.
#   ZeroMQ_LIBRARIES - List of the libraries.
#   ZeroMQ_VERSION - The library version MAJOR.MINOR.PATCH
#   ZeroMQ_VERSION_MAJOR - Major version number
#   ZeroMQ_VERSION_MINOR - Minor version number
#   ZeroMQ_VERSION_PATCH - Patch version number
#
# And the following imported targets::
#
#   ZeroMQ::zmq

find_brew_prefix(_ZMQ_BREW_HINT zmq)

if(ZeroMQ_ROOT_DIR)
    set(_ZMQ_PATHS "${ZeroMQ_ROOT_DIR}")
else()
    # Paths for anything other than Windows
    # Cellar/zmq is for macOS from homebrew installation
    list(APPEND _ZMQ_PATHS
            "/usr/local/opt/zmq"
            "/usr/local/Cellar/zmq"
            "/opt"
            "/opt/local"
			"/usr"
            "/usr/local"
            )
endif()

if(NOT ZeroMQ_INCLUDE_DIR)
	find_path(ZeroMQ_INCLUDE_DIR
			zmq.h
			PATHS "${_ZMQ_PATHS}"
		    HINTS ${_ZMQ_BREW_HINT}
			PATH_SUFFIXES "include" "includes"
			)
endif()
# Find includes path
		
set(ZeroMQ_INCLUDE_DIRS "${ZeroMQ_INCLUDE_DIR}")
mark_as_advanced(ZeroMQ_INCLUDE_DIR)

if(ZeroMQ_INCLUDE_DIR)
	# Extract version information from the zmq.h header.
	if(NOT DEFINED ZeroMQ_VERSION)
		# Read the version from file zmq.h into a variable.
		file(READ "${ZeroMQ_INCLUDE_DIR}/zmq.h" _ZMQ_HEADER)

		# Parse the version into variables.
		string(REGEX REPLACE
			".*ZMQ_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1"
			ZeroMQ_VERSION_MAJOR
			"${_ZMQ_HEADER}"
		)
		string(REGEX REPLACE
			".*ZMQ_VERSION_MINOR[ \t]+([0-9]+).*" "\\1"
			ZeroMQ_VERSION_MINOR
			"${_ZMQ_HEADER}"
		)
		# Patch version example on non-crypto installs: x.x.xNC
		string(REGEX REPLACE
			".*ZMQ_VERSION_PATCH[ \t]+([0-9]+(NC)?).*" "\\1"
			ZeroMQ_VERSION_PATCH
			"${_ZMQ_HEADER}"
		)

		# Cache the result.
		set(ZeroMQ_VERSION_MAJOR ${ZeroMQ_VERSION_MAJOR}
			CACHE INTERNAL "ZeroMQ major version number"
		)
		set(ZeroMQ_VERSION_MINOR ${ZeroMQ_VERSION_MINOR}
			CACHE INTERNAL "ZeroMQ minor version number"
		)
		set(ZeroMQ_VERSION_PATCH ${ZeroMQ_VERSION_PATCH}
			CACHE INTERNAL "ZeroMQ patch version number"
		)
		# The actual returned/output version variable (the others can be used if
		# needed).
		set(ZeroMQ_VERSION
			"${ZeroMQ_VERSION_MAJOR}.${ZeroMQ_VERSION_MINOR}.${ZeroMQ_VERSION_PATCH}"
			CACHE INTERNAL "ZeroMQ full version"
		)
	endif()

	include(ExternalLibraryHelper)

	if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		# The dependency to iphlpapi starts from 4.2.0
		if(ZeroMQ_VERSION VERSION_LESS 4.2.0)
			set(_ZeroMQ_WINDOWS_LIBRARIES "$<$<PLATFORM_ID:Windows>:ws2_32;rpcrt4>")
		else()
			set(_ZeroMQ_WINDOWS_LIBRARIES "$<$<PLATFORM_ID:Windows>:ws2_32;rpcrt4;iphlpapi>")
		endif()

		find_component(ZeroMQ zmq
			NAMES zmq libzmq
			INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIRS}
			HINTS ${_ZMQ_BREW_HINT}
			PATH_SUFFIXES "lib" "lib64" "libs" "libs64"
			PATHS ${_ZMQ_PATHS}
			INTERFACE_LINK_LIBRARIES "${_ZeroMQ_WINDOWS_LIBRARIES}"
		)
	else()
		if(BUILD_STATIC)
			find_library(LIBSODIUM_LIBRARY NAMES sodium libsodium)
			list (APPEND ZeroMQ_LIBRARIES ${Qrcode_LIBRARIES})
			list (APPEND ZeroMQ_LIBRARIES ${LIBSODIUM_LIBRARY})
			if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
				find_library(LIBNORM_LIBRARY NAMES libnorm norm)
				find_library(LIBPGM_LIBRARY NAMES libpgm pgm)
				if(LIBNORM_LIBRARY)
					list (APPEND ZeroMQ_LIBRARIES ${LIBNORM_LIBRARY})
				endif()
				if(LIBPGM_LIBRARY)
					list (APPEND ZeroMQ_LIBRARIES ${LIBPGM_LIBRARY})
				endif()
			endif()
		endif()

		find_component(
			ZeroMQ zmq
			NAMES
			zmq libzmq
			INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIRS}
			HINTS ${_ZMQ_BREW_HINT}
			PATH_SUFFIXES "lib" "lib64" "libs" "libs64"
			PATHS ${_ZMQ_PATHS}
			INTERFACE_LINK_LIBRARIES "${ZeroMQ_LIBRARIES}"
		)
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZeroMQ
	REQUIRED_VARS ZeroMQ_INCLUDE_DIR
	VERSION_VAR ZeroMQ_VERSION
	HANDLE_COMPONENTS
)

if(ZeroMQ_FOUND)
    set(ZeroMQ_INCLUDE_DIR "${ZeroMQ_INCLUDE_DIRS}")
    set(ZeroMQ_LIBRARY "${ZeroMQ_LIBRARIES}")

    message(STATUS "Found ZeroMQ  (include: ${ZeroMQ_INCLUDE_DIR}, library: ${ZeroMQ_LIBRARY})")
    mark_as_advanced(ZeroMQ_INCLUDE_DIR ZeroMQ_LIBRARY ZeroMQ_LIBRARIES)
endif()
