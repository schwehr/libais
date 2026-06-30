find_package(Threads REQUIRED)

include_directories(
    SYSTEM
    ${CMAKE_SOURCE_DIR}/third_party/gmock/include
    ${CMAKE_SOURCE_DIR}/third_party/gtest/include
)

file(GLOB TEST_SRCS "*_test.cpp")

foreach(test_src ${TEST_SRCS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE ais gmock_main Threads::Threads)
    target_compile_options(${test_name} PRIVATE -Wno-deprecated-copy)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
