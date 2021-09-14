# .rst:
# FindRocksDB
# --------------
# Find the RocksDB libraries
#
# The following variables are optionally searched for defaults
#  ROCKSDB_ROOT_DIR:    Base directory where all RocksDB components are found
#
# The following are set after configuration is done:
#  RocksDB_FOUND
#  RocksDB_INCLUDE_DIR
#  ROCKSDB_LIBRARY

find_brew_prefix(_ROCKSDB_BREW_HINT rocksdb)

if(ROCKSDB_ROOT_DIR)
    set(_ROCKSDB_PATHS "${ROCKSDB_ROOT_DIR}")
else()
    # Paths for anything other than Windows
    # Cellar/rocksdb is for macOS from homebrew installation
    list(APPEND _ROCKSDB_PATHS
            "/usr/local/opt/rocksdb"
            "/usr/local/Cellar/rocksdb"
            "/opt"
            "/opt/local"
			"/usr"
            "/usr/local"
            )
endif()
if(NOT RocksDB_INCLUDE_DIR)
    find_path(RocksDB_INCLUDE_DIR NAMES rocksdb/db.h
            PATHS "${_ROCKSDB_PATHS}"
		    HINTS ${_ROCKSDB_BREW_HINT}
            PATH_SUFFIXES "include" "includes")
endif()

if(RocksDB_INCLUDE_DIR)
    if(BUILD_STATIC)
        find_package(BZip2)
        find_library(ZSTD_LIBRARY NAMES zstd libzstd)
        find_library(LZ4_LIBRARY NAMES lz4 liblz4)
        # find_library(GFLAGS_LIBRARY NAMES gflags PATHS ${GFLAGS_ROOT_DIR}/lib)
        list (APPEND ROCKSDB_LIBRARIES ${Snappy_LIBRARIES})
        list (APPEND ROCKSDB_LIBRARIES ${ZLIB_LIBRARY})
        list (APPEND ROCKSDB_LIBRARIES ${LZ4_LIBRARY})
        list (APPEND ROCKSDB_LIBRARIES ${BZIP2_LIBRARY})
        list (APPEND ROCKSDB_LIBRARIES ${ZSTD_LIBRARY})
        # list (APPEND ROCKSDB_LIBRARIES ${GFLAGS_LIBRARY})
    endif()

	find_component(
		RocksDB rocksdb
		NAMES
            rocksdbd rocksdb librocksdb
		INCLUDE_DIRS ${RocksDB_INCLUDE_DIR}
		HINTS ${_ROCKSDB_BREW_HINT}
        PATH_SUFFIXES "lib" "lib64" "libs" "libs64"
        PATHS ${_ROCKSDB_PATHS}
        INTERFACE_LINK_LIBRARIES "${ROCKSDB_LIBRARIES}"
	)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RocksDB
	REQUIRED_VARS RocksDB_INCLUDE_DIR
	HANDLE_COMPONENTS
)

if(RocksDB_FOUND)
    set(RocksDB_INCLUDE_DIR "${RocksDB_INCLUDE_DIRS}")
    set(RocksDB_LIBRARY "${RocksDB_LIBRARIES}")
    set(ROCKSDB_LIBRARY "${RocksDB_LIBRARIES}")

    message(STATUS "Found RocksDB  (include: ${RocksDB_INCLUDE_DIR}, library: ${ROCKSDB_LIBRARY})")
    mark_as_advanced(RocksDB_INCLUDE_DIR ROCKSDB_LIBRARY RocksDB_LIBRARY RocksDB_LIBRARIES)
endif()
