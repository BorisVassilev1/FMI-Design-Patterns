cmake_minimum_required(VERSION 3.15...3.30)

project(
	FiguresHomework
	VERSION 1.0
	LANGUAGES CXX)

# Prepare doctest for other targets to use



file(GLOB_RECURSE FIGURES_SOURCES
	./src/*.cpp
)
# Make main application
add_executable(main main.cpp ${FIGURES_SOURCES})
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)
target_compile_features(main PRIVATE cxx_std_20)
target_include_directories(main PUBLIC ./lib/)


# Make test executable
add_executable(tests test.cpp ${FIGURES_SOURCES})
set_target_properties(tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)
target_compile_features(tests PRIVATE cxx_std_20)
set(CMAKE_BUILD_TYPE Debug)
target_include_directories(tests PUBLIC ./lib/)

list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")

include(CTest)
enable_testing()
add_test(NAME allTests 
	COMMAND
	"../tests"
	DEPENDS tests
)
