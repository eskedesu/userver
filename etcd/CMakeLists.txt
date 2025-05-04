project(userver-etcd CXX)

userver_module(etcd
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    UTEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*_test.cpp"
)

if (USERVER_BUILD_TESTS)
  add_subdirectory(functional_tests)
endif()
