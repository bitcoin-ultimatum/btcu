#
# generate packages
#
# Environment
if(NOT CPACK_SYSTEM_NAME)
	set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_PROCESSOR}")
endif()


# Basic information
if(NOT CPACK_PACKAGE_NAME)
	set(CPACK_PACKAGE_NAME "btcu")
    if(ENABLE_GUI AND ENABLE_DAEMON AND BUILD_UTILS)
	    set(CPACK_PACKAGE_NAME "btcu-complete")
    elseif(ENABLE_GUI AND ENABLE_DAEMON)
	    set(CPACK_PACKAGE_NAME "btcud-with-qt")
    elseif(ENABLE_DAEMON AND BUILD_UTILS)
	    set(CPACK_PACKAGE_NAME "btcud-with-tools")
    elseif(ENABLE_GUI)
	    set(CPACK_PACKAGE_NAME "btcu-qt")
    elseif(ENABLE_DAEMON)
	    set(CPACK_PACKAGE_NAME "btcud")
    elseif(BUILD_UTILS)
	    set(CPACK_PACKAGE_NAME "btcu-tools")
    endif()
endif()
set(CPACK_DEBIAN_PACKAGE_CONFLICTS ${CPACK_PACKAGE_NAME})
set(CPACK_DEBIAN_PACKAGE_PROVIDES  ${CPACK_PACKAGE_NAME})
set(CPACK_DEBIAN_PACKAGE_REPLACES  ${CPACK_PACKAGE_NAME})

if (NOT DEFINED PACKAGE_VERSION)
    message(STATUS "PACKAGE_VERSION not set.")
    if (DEFINED btcu_VERSION)
        message(STATUS "CPACK_PACKAGE_VERSION: Falling back to btcu_VERSION (${btcu_VERSION})")
        set(PACKAGE_VERSION ${btcu_VERSION})
    elseif (DEFINED CLIENT_VERSION)
        message(WARNING "CPACK_PACKAGE_VERSION: Falling back to CLIENT_VERSION (${CLIENT_VERSION})")
        set(PROJECT_VERSION ${CLIENT_VERSION})
    endif ()
endif ()
if (NOT DEFINED VERSION_MAJOR)
    message(STATUS "VERSION_MAJOR not set.")
    if (DEFINED btcu_VERSION_MAJOR)
        message(STATUS "VERSION_MAJOR: Falling back to btcu_VERSION_MAJOR (${btcu_VERSION_MAJOR})")
        set(VERSION_MAJOR ${btcu_VERSION_MAJOR})
    elseif (DEFINED CLIENT_VERSION_MAJOR)
        message(WARNING "CPACK_PACKAGE_VERSION: Falling back to CLIENT_VERSION_MAJOR (${CLIENT_VERSION_MAJOR})")
        set(VERSION_MAJOR ${CLIENT_VERSION_MAJOR})
    endif ()
endif ()
if (NOT DEFINED VERSION_MINOR)
    message(STATUS "VERSION_MINOR not set.")
    if (DEFINED btcu_VERSION_MINOR)
        message(STATUS "CPACK_PACKAGE_VERSION: Falling back to btcu_VERSION_MINOR (${btcu_VERSION_MINOR})")
        set(VERSION_MINOR ${btcu_VERSION_MINOR})
    elseif (DEFINED CLIENT_VERSION)
        message(WARNING "CPACK_PACKAGE_VERSION: Falling back to CLIENT_VERSION_MINOR (${CLIENT_VERSION_MINOR})")
        set(VERSION_MINOR ${CLIENT_VERSION_MINOR})
    endif ()
endif ()
if (NOT DEFINED VERSION_PATCH)
    message(STATUS "VERSION_PATCH not set.")
    if (DEFINED btcu_VERSION_PATCH)
        message(STATUS "CPACK_PACKAGE_VERSION: Falling back to btcu_VERSION_PATCH (${btcu_VERSION_PATCH})")
        set(VERSION_PATCH ${btcu_VERSION_PATCH})
    elseif (DEFINED CLIENT_VERSION_REVISION)
        message(WARNING "CPACK_PACKAGE_VERSION: Falling back to CLIENT_VERSION_REVISION (${CLIENT_VERSION_REVISION})")
        set(VERSION_PATCH ${CLIENT_VERSION_REVISION})
    endif ()
endif ()
if (NOT DEFINED VERSION_BUILD)
    message(STATUS "VERSION_BUILD not set.")
    if (DEFINED btcu_VERSION_TWEAK)
        message(STATUS "VERSION_BUILD: Falling back to btcu_VERSION_TWEAK (${btcu_VERSION_TWEAK})")
        set(VERSION_BUILD ${btcu_VERSION_TWEAK})
    elseif (DEFINED CLIENT_VERSION_BUILD)
        message(WARNING "VERSION_BUILD: Falling back to CLIENT_VERSION_BUILD (${CLIENT_VERSION_BUILD})")
        set(VERSION_BUILD ${CLIENT_VERSION_BUILD})
    endif ()
endif ()
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CPACK_PACKAGE_CONTACT "BTCU Developers <info@@btcu.io>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "info@@btcu.io")
set(CPACK_PACKAGE_HOMEPAGE "https://btcu.io/")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BTCU is a free open source peer-to-peer electronic cash system that is completely decentralized, without the need for a central server or trusted parties.  Users hold the crypto keys to their own money and transact directly with each other, with the help of a P2P network to check for double-spending.")
set(CPACK_PACKAGE_VENDOR "BTCU Developers")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_SOURCE_DIR}/build/;${CMAKE_SOURCE_DIR}/.git/;")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/src/qt/res/icons/bitcoin.png")
set(CPACK_STRIP_FILES  TRUE)

# DEB package
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "BTCU is a free open source peer-to-peer electronic cash system that
 is completely decentralized, without the need for a central server or
 trusted parties.  Users hold the crypto keys to their own money and
 transact directly with each other, with the help of a P2P network to
 check for double-spending.
 .
 Full transaction history is stored locally at each client.  This
 requires 10+ GB of space, slowly growing.")

if(ENABLE_GUI AND ENABLE_DAEMON AND BUILD_UTILS)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}
    .
    This package provides the daemon, btcud, CLI tool, and the GUI Wallet.
    btcu-cli to interact with the daemon.")
elseif(ENABLE_GUI AND ENABLE_DAEMON)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}
    .
    This package provides the daemon, btcud, and the GUI Wallet.")
elseif(ENABLE_DAEMON AND BUILD_UTILS)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}
    .
    This package provides the daemon, btcud, and the CLI tool.
    btcu-cli to interact with the daemon.")
elseif(ENABLE_GUI)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}
    .
    This package provides the GUI Wallet.")
elseif(ENABLE_DAEMON)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}
    .
    This package provides the daemon")
elseif(BUILD_UTILS)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}
    .
    This package provides the CLI tool.
    btcu-cli to interact with the daemon so it is required to install also btcud.")
endif()
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6-dev, libglvnd-dev, libx11-dev, libssl-dev, libxcb-glx0-dev, libdrm-dev, libx11-xcb-dev, libxcb-icccm4-dev, libxcb-image0-dev, libxcb-shm0-dev, libxcb-util-dev, libxcb-keysyms1-dev, libxcb-randr0-dev, libxcb-render-util0-dev, libxcb-render0-dev, libxcb-shape0-dev, libxcb-sync-dev, libxcb-xfixes0-dev, libxcb-xinerama0-dev, libxcb-xkb-dev, libxcb1-dev, libxkbcommon-x11-dev, libegl1-mesa-dev, libwayland-dev, libxkbcommon-dev, libpng-dev, libharfbuzz-dev, zlib1g-dev, libicu-dev, libzstd-dev, libglib2.0-dev, libsm-dev, libice-dev, libxext-dev, libxau-dev, libxdmcp-dev, libffi-dev, libfreetype6-dev, libgraphite2-dev, libpcre3-dev, uuid-dev, libbsd-dev, libqt5gui5, libqt5core5a, libqt5dbus5, qttools5-dev, qttools5-dev-tools, libqt5svg5-dev, libqt5charts5")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA, "${CMAKE_SOURCE_DIR}/share/deb/postinst;")
if (NOT DEFINED CPACK_DEBIAN_PACKAGE_SHLIBDEPS)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif ()
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

function(ReadRelease valuename FROM filename INTO varname)
  file (STRINGS ${filename} _distrib
	REGEX "^${valuename}="
	)
  string (REGEX REPLACE
	"^${valuename}=\"?\(.*\)" "\\1" ${varname} "${_distrib}"
	)
  # remove trailing quote that got globbed by the wildcard (greedy match)
  string (REGEX REPLACE
	"\"$" "" ${varname} "${${varname}}"
	)
  set (${varname} "${${varname}}" PARENT_SCOPE)
ENDfunction()

# RPM package
if(EXISTS /etc/os-release)
    ReadRelease("NAME" FROM /etc/os-release INTO OS_NAME)
    if(OS_NAME MATCHES ".*openSUSE.*")
        set(OS_OPENSUSE TRUE)
    endif()
endif()

if(EXISTS /etc/redhat-release OR EXISTS /etc/fedora-release OR OS_OPENSUSE)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}.${CPACK_SYSTEM_NAME}")
    set(CPACK_GENERATOR "RPM")
    find_program(RPM_PROGRAM rpm DOC "rpm RPM-based systems")
    find_program(RPMBUILD_PROGRAM rpm DOC "rpm RPM-based systems")
    if(RPMBUILD_PROGRAM)
        SET(CPACK_GENERATOR "RPM")
        execute_process(
                COMMAND ${RPM_PROGRAM} --eval %{_arch}
                OUTPUT_VARIABLE CPACK_RPM_PACKAGE_ARCHITECTURE
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}.${CPACK_RPM_PACKAGE_ARCHITECTURE}")
        # Exclude /usr/lib64/pkgconfig directory given that it is already owned by the pkg-config rpm package.
        SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    endif()
elseif(EXISTS /etc/debian_version)
    find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
    if(CPACK_SYSTEM_NAME STREQUAL "x86_64")
				set(CPACK_SYSTEM_NAME "amd64")
	endif()
    if (NOT DPKG_PROGRAM)
        message(STATUS "Can not find dpkg in your path, default to i386.")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
    else ()
        execute_process(COMMAND "${DPKG_PROGRAM}" --print-architecture
                OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
                OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif ()
    # ZIP WIX NSIS etc is currently unsupported
    # PackageMake is unsused since we use macdeployqt already
    #set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
    set(CPACK_GENERATOR "DEB")
    execute_process(
        COMMAND ${DPKG_PROGRAM} --print-architecture
        OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
else()
    message(FATAL "Can't recognize your system.")
endif()

set(CPACK_SOURCE_GENERATOR "${CPACK_GENERATOR}")

include(CPack)
