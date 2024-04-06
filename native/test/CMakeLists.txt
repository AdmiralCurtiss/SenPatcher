cmake_minimum_required(VERSION 3.24 FATAL_ERROR)


set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(GTEST_HAS_ABSL OFF CACHE BOOL "" FORCE)
add_subdirectory(../gtest googletest)


add_executable(tests)
target_sources(tests PRIVATE
	../sha1.cpp
	../sha1.h
	../file.cpp
	../file.h
	../util/text.cpp
	../util/text.h

	sha1_test.cpp
)
target_include_directories(tests
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.." "../gtest/googletest/include"
)
target_link_libraries(
	tests
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
