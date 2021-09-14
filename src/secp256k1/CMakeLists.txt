# Copyright (c) 2017 The Bitcoin developers

cmake_minimum_required(VERSION 3.14)
project(secp256k1 LANGUAGES C VERSION 0.1.0)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 90)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(TestBigEndian)
include(CheckCSourceCompiles)
include(CheckIncludeFiles)

if (MSVC OR BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_STATIC_LIBS ON)
else()
    set(BUILD_SHARED_LIBS ON)
    set(BUILD_STATIC_LIBS OFF)
endif()

# Add path for custom modules when building as a standalone project
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/script)

# Default to RelWithDebInfo configuration
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Select the configuration for the build" FORCE)
	set(__NO_USER_CMAKE_BUILD_TYPE ON CACHE BOOL "True if the user didn't set a build type on the command line")
endif()

include(AddCompilerFlags)

# libsecp256k1 use a different set of flags.
if(MSVC)
	add_compile_options(/wd4244)
	add_compile_options(/wd4267)
	add_compile_options(/wd4244)
else()
	add_c_compiler_flags(
			-pedantic
			-Wall
			-Wextra
			-Wcast-align
			-Wshadow
			-Wundef
			-Wno-unused-function
			-Wno-overlength-strings
			-Wno-nonnull-compare
			-Wno-implicit-fallthrough
			-std=c89
			-Wnested-externs
			-Wstrict-prototypes
			-Wno-long-long
	)
endif()

#
# Feature Testing
#
check_c_source_compiles([=[
    #include <stdint.h>
    int main(void) {
      uint64_t a = 11, tmp;
      __asm__ __volatile__("movq $0x100000000,%1; mulq %%rsi"
                           : "+a" (a) : "S" (tmp) : "cc", "%rdx");
      return 0;
    }
]=] SECP256K1_HAS_ASM_X64)

test_big_endian(SECP256K1_BIGENDIAN)

check_type_size("__int128" SECP256K1_HAS_INT128)

if(SECP256K1_HAS_INT128)
	set(USE_FORCE_WIDEMUL_INT128 1)
endif()
# we do not need to calculate uint64_t availability here since it will be resolved within src/util.h

if(SECP256K1_HAS_ASM_X64)
  set(USE_ASM_X86_64 1)
endif()

# Check is OpenSSL::Crypto exists
if(OpenSSL::Crypto)
	set(HAVE_LIBCRYPTO 1)
endif()

# TODO: Add support for valgrind module
# set(HAVE_VALGRIND 1)

# Guess the target architecture, within the ones with supported ASM.
# First check if the CMAKE_C_COMPILER_TARGET is set (should be when
# cross compiling), then CMAKE_SYSTEM_PROCESSOR as a fallback if meaningful
# (this is not the case for ARM as the content is highly non standard).
if(CMAKE_C_COMPILER_TARGET MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR SECP256K1_HAS_ASM_X64)
	set(SECP256K1_ASM_BUILD_TARGET "x86_64")
  	set(ASM_BUILD_TARGET "x86_64")
	set(SECP256K1_DEFAULT_USE_ASM ON)
  	set(USE_ASM 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(SECP256K1_DEFAULT_USE_ASM OFF)
  	set(USE_ASM OFF)
elseif(CMAKE_C_COMPILER_TARGET MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64" OR CMAKE_C_COMPILER_TARGET MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
	set(USE_ASM_ARM 1)
	set(SECP256K1_ASM_BUILD_TARGET "arm")
  	set(ASM_BUILD_TARGET "arm")
	set(SECP256K1_DEFAULT_USE_ASM ON)
  	set(USE_ASM 1)
elseif(CMAKE_C_COMPILER_TARGET MATCHES "arm-linux-gnueabihf")
	set(SECP256K1_ASM_BUILD_TARGET "arm-linux-gnueabihf")
  	set(ASM_BUILD_TARGET "arm-linux-gnueabihf")
	set(SECP256K1_DEFAULT_USE_ASM ON)
  	set(USE_ASM 1)
endif()

set(SECP256K1_TEST_OVERRIDE_WIDE_MULTIPLY "" CACHE STRING "Test-only override of the (autodetected by the C code) \"widemul\" setting (can be int64 or int128)")
if(SECP256K1_TEST_OVERRIDE_WIDE_MULTIPLY STREQUAL "int128")
	message(STATUS "Force the use of the (unsigned) __int128 based wide multiplication implementation")
  	set(USE_FORCE_WIDEMUL_INT128 1)
	unset(USE_FORCE_WIDEMUL_INT64)
elseif(SECP256K1_TEST_OVERRIDE_WIDE_MULTIPLY STREQUAL "int64")
	message(STATUS "Force the use of the (u)int64_t based wide multiplication implementation")
	unset(USE_FORCE_WIDEMUL_INT128)
  	set(USE_FORCE_WIDEMUL_INT64 1)
endif()
# we do not need here an "auto" option for SECP256K1_TEST_OVERRIDE_WIDE_MULTIPLY since it have been calculated already

option(SECP256K1_BUILD_TEST "Build secp256k1's unit tests" ${ENABLE_TESTS})

include(CMakeDependentOption)
cmake_dependent_option(
	SECP256K1_BUILD_OPENSSL_TESTS "Build the OpenSSL tests" ${ENABLE_TESTS}
	SECP256K1_BUILD_TEST ${ENABLE_TESTS}
)
if(SECP256K1_BUILD_OPENSSL_TESTS)
	if(NOT OpenSSL_FOUND)
		message(FATAL_ERROR
			"OpenSSL is not found, but is required for some tests. You can disable them by passing -DSECP256K1_BUILD_OPENSSL_TESTS=OFF."
		)
	endif()
	set(ENABLE_OPENSSL_TESTS 1)
endif()


option(SECP256K1_ENABLE_COVERAGE "Enable secp256k1's coverage" ${ENABLE_COVERAGE})
if(SECP256K1_ENABLE_COVERAGE)
	set(COVERAGE 1)
	add_c_compiler_flags(
		--coverage
	)
endif()

# External default callbacks
option(SECP256K1_ENABLE_EXTERNAL_DEFAULT_CALLBACKS "Enable external default callbacks" OFF)
if(SECP256K1_ENABLE_EXTERNAL_DEFAULT_CALLBACKS)
  	set(USE_EXTERNAL_DEFAULT_CALLBACKS 1)
endif()

# Make the emult window size customizable.
# window size for ecmult precomputation for verification, specified as integer in range [2..24].
# Larger values result in possibly better performance at the cost of an exponentially larger precomputed table.
# The table will store 2^(SIZE-1) * 64 bytes of data but can be larger in memory due to platform-specific padding and alignment.
# 15 is a reasonable setting for desktop machines
set(SECP256K1_ECMULT_WINDOW_SIZE 15 CACHE STRING "Window size for ecmult precomputation for verification, specified as integer in range [2..24].")
              
if(${SECP256K1_ECMULT_WINDOW_SIZE} LESS 2 OR ${SECP256K1_ECMULT_WINDOW_SIZE} GREATER 24)
	message(FATAL_ERROR "SECP256K1_ECMULT_WINDOW_SIZE must be an integer in range [2..24]")
else()
  	set(ECMULT_WINDOW_SIZE 1)
endif()

# Precision bits to tune the precomputed table size for signing.
# The size of the table is 32kB for 2 bits, 64kB for 4 bits, 512kB for 8 bits of precision.
# A larger table size usually results in possible faster signing.
# 4 is a reasonable setting for desktop machines
set(SECP256K1_ECMULT_GEN_PRECISION 4 CACHE STRING "Precision bits to tune the precomputed table size for signing.")
set(VALID_PRECISIONS 2 4 8)
if(NOT ${SECP256K1_ECMULT_GEN_PRECISION} IN_LIST VALID_PRECISIONS)
	message(FATAL_ERROR "SECP256K1_ECMULT_GEN_PRECISION not 2, 4, 8")
endif()

set(secp256k1_sources
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_4x64.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_8x32.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_low.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_4x64_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_8x32_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scalar_low_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/group.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/group_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecdsa.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecdsa_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/eckey.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/eckey_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecmult.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecmult_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecmult_const.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecmult_const_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecmult_gen.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ecmult_gen_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field_10x26.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field_10x26_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field_5x52.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field_5x52_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field_5x52_int128_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field_5x52_asm_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/modinv32.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/modinv32_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/modinv64.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/modinv64_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/assumptions.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/util.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scratch.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/scratch_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/selftest.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/testrand.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/testrand_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/hash.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/hash_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/field_impl.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/bench.h
	${CMAKE_CURRENT_SOURCE_DIR}/contrib/lax_der_parsing.h
	${CMAKE_CURRENT_SOURCE_DIR}/contrib/lax_der_parsing.c
	${CMAKE_CURRENT_SOURCE_DIR}/contrib/lax_der_privatekey_parsing.h
	${CMAKE_CURRENT_SOURCE_DIR}/contrib/lax_der_privatekey_parsing.c
	)

include_directories(
	.
	src
	contrib
	# For the config
	${CMAKE_CURRENT_BINARY_DIR}/src
)

# ECDH module
option(SECP256K1_ENABLE_MODULE_ECDH "Build libsecp256k1's ECDH module" ON)
if(SECP256K1_ENABLE_MODULE_ECDH)
	set(ENABLE_MODULE_ECDH 1)
	if(SECP256K1_ENABLE_BENCHMARKS)
		add_secp256k1_bench(ecdh src/bench_ecdh.c)
	endif()
	list(APPEND SECP256K1_PUBLIC_HEADERS include/secp256k1_ecdh.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/ecdh/main_impl.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/ecdh/tests_impl.h)
endif()

# Recovery module
option(SECP256K1_ENABLE_MODULE_RECOVERY "Build libsecp256k1's recovery module" ON)
if(SECP256K1_ENABLE_MODULE_RECOVERY)
	set(ENABLE_MODULE_RECOVERY 1)
	if(SECP256K1_ENABLE_BENCHMARKS)
		add_secp256k1_bench(recover src/bench_recover.c)
	endif()
	list(APPEND SECP256K1_PUBLIC_HEADERS include/secp256k1_recovery.h)

	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/recovery/main_impl.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/recovery/tests_impl.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/recovery/tests_exhaustive_impl.h)
endif()

# Extrakeys module
option(SECP256K1_ENABLE_MODULE_EXTRAKEYS "Build libsecp256k1's extrakeys module (experimental)" ON)
if(SECP256K1_ENABLE_MODULE_EXTRAKEYS)
	set(ENABLE_MODULE_EXTRAKEYS 1)
	list(APPEND SECP256K1_PUBLIC_HEADERS include/secp256k1_extrakeys.h)

	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/extrakeys/tests_impl.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/extrakeys/tests_exhaustive_impl.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/extrakeys/main_impl.h)
endif()

# Schnorrsig module
option(SECP256K1_ENABLE_MODULE_SCHNORRSIG "Build libsecp256k1's schnorrsig module (experimental)" ON)
if(SECP256K1_ENABLE_MODULE_SCHNORRSIG)
	set(ENABLE_MODULE_SCHNORRSIG 1)
	if(SECP256K1_ENABLE_BENCHMARKS)
		add_secp256k1_bench(schnorrsig src/bench_schnorrsig.c)
	endif()
	list(APPEND SECP256K1_PUBLIC_HEADERS include/secp256k1_schnorrsig.h)

	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/schnorrsig/main_impl.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/schnorrsig/tests_impl.h)
	list(APPEND secp256k1_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/schnorrsig/tests_exhaustive_impl.h)
endif()

# The library
add_library(secp256k1 ${secp256k1_sources} src/secp256k1.c)
target_include_directories(secp256k1 PUBLIC include)

set(SECP256K1_PUBLIC_HEADERS
	include/secp256k1.h
	include/secp256k1_preallocated.h
)

if(GMP_FOUND)
	message(STATUS "GMP included into secp256k1")
	target_link_libraries(secp256k1 ${GMP_LIBRARIES})
    target_include_directories(secp256k1 PUBLIC ${GMP_INCLUDE_DIR})
endif()


# Enable ASM by default only if we are building for a compatible target.
# The user can still enable/disable it manually if needed.
option(SECP256K1_USE_ASM "Use assembly" ${SECP256K1_DEFAULT_USE_ASM})
if(SECP256K1_USE_ASM)
	macro(unsupported_asm_error)
		message(FATAL_ERROR
			"Assembly is enabled, but not supported for your target architecture."
			"Re-run cmake with -DSECP256K1_USE_ASM=OFF to disable ASM support."
		)
	endmacro()

	if(SECP256K1_ASM_BUILD_TARGET MATCHES "x86_64")
		# We check if amd64 asm is supported.
		check_c_source_compiles("
			#include <stdint.h>
			int main() {
				uint64_t a = 11, tmp;
				__asm__ __volatile__(\"movq \$0x100000000,%1; mulq %%rsi\" : \"+a\"(a) : \"S\"(tmp) : \"cc\", \"%rdx\");
				return 0;
			}
		" USE_ASM_X86_64)

		if(NOT USE_ASM_X86_64)
			unsupported_asm_error()
		endif()
	elseif(SECP256K1_ASM_BUILD_TARGET MATCHES "arm-linux-gnueabihf")
		set(CMAKE_ASM_CREATE_SHARED_LIBRARY ${CMAKE_C_CREATE_SHARED_LIBRARY})
		enable_language(ASM)
		set(USE_EXTERNAL_ASM 1)
		add_library(secp256k1_common src/asm/field_10x26_arm.s)
		target_link_libraries(secp256k1 secp256k1_common)
	else()
		unsupported_asm_error()
	endif()
endif()

# Executable internal to secp256k1 need to have the HAVE_CONFIG_H define set.
# For convenience, we wrap this into a function.
function(link_secp256k1_internal NAME)
	target_link_libraries(${NAME} secp256k1)
	target_compile_definitions(${NAME} PRIVATE HAVE_CONFIG_H SECP256K1_BUILD)
	if(SECP256K1_BUILD_OPENSSL_TESTS)
		target_link_libraries(${NAME} OpenSSL::Crypto)
	endif()
endfunction(link_secp256k1_internal)

include(InstallationHelper)

option(SECP256K1_ENABLE_BENCHMARKS "Enable compile benchmark" OFF)
if(SECP256K1_ENABLE_BENCHMARKS)
	# Phony target to build benchmarks
	add_custom_target(bench-secp256k1)
endif()

function(add_secp256k1_bench NAME)
	set(EXECUTABLE_NAME "${NAME}-bench")
	add_executable(${EXECUTABLE_NAME} ${ARGN})
	link_secp256k1_internal(${EXECUTABLE_NAME})

	set(BENCH_NAME "bench-secp256k1-${NAME}")
	add_custom_target(${BENCH_NAME}
		COMMENT "Benchmarking libsecp256k1 ${NAME}"
		COMMAND ${EXECUTABLE_NAME}
		USES_TERMINAL
	)
	add_dependencies(bench-secp256k1 ${BENCH_NAME})

	install_target("${EXECUTABLE_NAME}"
		COMPONENT secp256k1-bench
		EXCLUDE_FROM_ALL
	)
endfunction(add_secp256k1_bench)

# temporary unavailable
option(SECP256K1_ECMULT_STATIC_PRECOMPUTATION "Precompute libsecp256k1's elliptic curve multiplication tables" ON)
if(SECP256K1_ECMULT_STATIC_PRECOMPUTATION)
		set(USE_ECMULT_STATIC_PRECOMPUTATION 1)
		add_executable(gen_context src/gen_context.c)
		target_include_directories(gen_context PRIVATE ${PROJECT_SOURCE_DIR})

		add_custom_target(ecmult_static_context gen_context WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
		add_dependencies(secp256k1 ecmult_static_context)

endif()
	
# If this project is not the top level project, then don't install by default
get_directory_property(SECP256K1_PARENT_DIRECTORY PARENT_DIRECTORY)
if(SECP256K1_PARENT_DIRECTORY)
  set(SECP256K1_INSTALL_EXCLUDE_FROM_ALL EXCLUDE_FROM_ALL)
endif()

if(BUILD_SHARED_LIBS)
	install_shared_library(secp256k1
		PUBLIC_HEADER ${SECP256K1_PUBLIC_HEADERS}
		${SECP256K1_INSTALL_EXCLUDE_FROM_ALL}
	)
else()
	set_property(TARGET secp256k1 PROPERTY PUBLIC_HEADER ${SECP256K1_PUBLIC_HEADERS})
	install_target(secp256k1 ${SECP256K1_INSTALL_EXCLUDE_FROM_ALL})
endif()

# Generate the config
configure_file(
	${PROJECT_SOURCE_DIR}/src/libsecp256k1-config.h.cmake.in ${PROJECT_SOURCE_DIR}/src/libsecp256k1-config.h ESCAPE_QUOTES)
target_compile_definitions(secp256k1 PRIVATE HAVE_CONFIG_H SECP256K1_BUILD)

# Tests
if(SECP256K1_BUILD_TEST)
	
	# RPATH is not supported in windows
	if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		return()
	endif()

	include(TestSuite)
	create_test_suite(secp256k1)

	function(create_secp256k1_test NAME FILES)
		add_test_to_suite(secp256k1 ${NAME} EXCLUDE_FROM_ALL ${FILES})
		link_secp256k1_internal(${NAME})
	endfunction()

	create_secp256k1_test(secp256k1-tests src/tests.c)

	option(SECP256K1_ENABLE_EXHAUSTIVE_TESTS "Enable exhaustive tests" OFF)
	if(SECP256K1_ENABLE_EXHAUSTIVE_TESTS)
		create_secp256k1_test(secp256k1-exhaustive_tests src/tests_exhaustive.c)
	endif()

	if(NOT SECP256K1_ENABLE_COVERAGE)
		target_compile_definitions(secp256k1-tests PRIVATE VERIFY)
		if(SECP256K1_ENABLE_EXHAUSTIVE_TESTS)
			target_compile_definitions(secp256k1-exhaustive_tests PRIVATE VERIFY)
		endif()
	endif()
endif(SECP256K1_BUILD_TEST)

# Benchmarks
if(SECP256K1_ENABLE_BENCHMARKS)
	add_secp256k1_bench(verify src/bench_verify.c)
	add_secp256k1_bench(sign src/bench_sign.c)
	add_secp256k1_bench(ecmult src/bench_ecmult.c)
	add_secp256k1_bench(internal src/bench_internal.c)
endif()
