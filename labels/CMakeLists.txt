cmake_minimum_required(VERSION 3.15...3.30)

project(
	FiguresHomework
	VERSION 1.0
	LANGUAGES CXX)

# Prepare doctest for other targets to use

SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_C_COMPILER clang)
#set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE LABELS_SOURCES
	./src/*.cpp
)
# Make main application
add_executable(main main.cpp ${LABELS_SOURCES})
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)
target_compile_features(main PRIVATE cxx_std_20)
target_compile_options(main PRIVATE -fsanitize=address -g)
target_link_options(main PRIVATE -fsanitize=address)
target_include_directories(main PUBLIC ../lib/)


SET(COVERAGE_FLAGS 
	--coverage
	-fprofile-arcs
	-ftest-coverage
)

# Make test executable
add_executable(tests test.cpp ${LABELS_SOURCES})
target_compile_definitions(tests PRIVATE PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
set_target_properties(tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ./)
target_compile_options(tests PRIVATE ${COVERAGE_FLAGS} -g -std=c++23 -fsanitize=address)
target_link_options(tests PRIVATE ${COVERAGE_FLAGS} -fsanitize=address -g)
target_include_directories(tests PUBLIC ../lib/)

set(CMAKE_BUILD_TYPE Debug)



list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")

include(CTest)
enable_testing()
add_test(NAME allTests 
	COMMAND
	"./tests"
	DEPENDS tests
)
