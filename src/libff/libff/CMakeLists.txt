project (ff)

set(FF_EXTRASRCS)
set(FF_EXTRALIBS)
set(CMAKE_FIND_LIBRARY_SUFFIXES_SAVE "${CMAKE_FIND_LIBRARY_SUFFIXES}")
if(${CURVE} STREQUAL "BN128")
  include_directories(${CMAKE_SOURCE_DIR}src/libff/depends/ate-pairing/include)
  set(
    FF_EXTRASRCS

    ${FF_EXTRASRCS}
    algebra/curves/bn128/bn128_g1.cpp
    algebra/curves/bn128/bn128_g2.cpp
    algebra/curves/bn128/bn128_gt.cpp
    algebra/curves/bn128/bn128_init.cpp
    algebra/curves/bn128/bn128_pairing.cpp
    algebra/curves/bn128/bn128_pp.cpp
  )
  set(
    FF_EXTRALIBS

    ${FF_EXTRALIBS}
    zm
  )
endif()

add_library(
  ff

  algebra/curves/alt_bn128/alt_bn128_g1.cpp
  algebra/curves/alt_bn128/alt_bn128_g2.cpp
  algebra/curves/alt_bn128/alt_bn128_init.cpp
  algebra/curves/alt_bn128/alt_bn128_pairing.cpp
  algebra/curves/alt_bn128/alt_bn128_pp.cpp
  algebra/curves/edwards/edwards_g1.cpp
  algebra/curves/edwards/edwards_g2.cpp
  algebra/curves/edwards/edwards_init.cpp
  algebra/curves/edwards/edwards_pairing.cpp
  algebra/curves/edwards/edwards_pp.cpp
  algebra/curves/mnt/mnt4/mnt4_g1.cpp
  algebra/curves/mnt/mnt4/mnt4_g2.cpp
  algebra/curves/mnt/mnt4/mnt4_init.cpp
  algebra/curves/mnt/mnt4/mnt4_pairing.cpp
  algebra/curves/mnt/mnt4/mnt4_pp.cpp
  algebra/curves/mnt/mnt46_common.cpp
  algebra/curves/mnt/mnt6/mnt6_g1.cpp
  algebra/curves/mnt/mnt6/mnt6_g2.cpp
  algebra/curves/mnt/mnt6/mnt6_init.cpp
  algebra/curves/mnt/mnt6/mnt6_pairing.cpp
  algebra/curves/mnt/mnt6/mnt6_pp.cpp
  common/double.cpp
  common/profiling.cpp
  common/utils.cpp

  ${FF_EXTRASRCS}
)
target_link_libraries(
  ff
  ${GMP_LIBRARIES}
  ${PROCPS_LIBRARIES}
  ${FF_EXTRALIBS}
)
target_include_directories(
  ff
  PUBLIC ..
  ${GMP_INCLUDE_DIR}
)

# Tests
if ("${IS_LIBFF_PARENT}")
  add_executable(
    algebra_bilinearity_test
    EXCLUDE_FROM_ALL

    algebra/curves/tests/test_bilinearity.cpp
  )
  target_link_libraries(
    algebra_bilinearity_test

    ff
  )

  add_executable(
    algebra_groups_test
    EXCLUDE_FROM_ALL

    algebra/curves/tests/test_groups.cpp
  )
  target_link_libraries(
    algebra_groups_test

    ff
  )

  add_executable(
    algebra_fields_test
    EXCLUDE_FROM_ALL

    algebra/fields/tests/test_fields.cpp
  )
  target_link_libraries(
    algebra_fields_test

    ff
  )

  include(CTest)
  add_test(
    NAME algebra_bilinearity_test
    COMMAND algebra_bilinearity_test
  )
  add_test(
    NAME algebra_groups_test
    COMMAND algebra_groups_test
  )
  add_test(
    NAME algebra_fields_test
    COMMAND algebra_fields_test
  )

  add_dependencies(check algebra_bilinearity_test)
  add_dependencies(check algebra_groups_test)
  add_dependencies(check algebra_fields_test)

  add_executable(
    multiexp_profile
    EXCLUDE_FROM_ALL

    algebra/scalar_multiplication/multiexp_profile.cpp
  )
  target_link_libraries(
    multiexp_profile

    OpenSSL::SSL
    ff
  )

  add_dependencies(profile multiexp_profile)
endif()
