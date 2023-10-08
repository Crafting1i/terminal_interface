# cmake_minimum_required(VERSION 2.8)

project(terminal_interface)

SET(GCC_COVERAGE_COMPILE_FLAGS "--std=c++17")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(SOURCE main.cpp engine.cpp util.cpp win.cpp mythreads.cpp)
set(HEADER engine.h util.h win.h keys.h styles.h mythreads.h)
set(LIBS -lncurses)

add_executable(interface ${SOURCE})

target_link_libraries(interface ${LIBS})