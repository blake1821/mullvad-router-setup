cmake_minimum_required(VERSION 3.0.0)
project(traffic-monitor VERSION 0.1.0 LANGUAGES C CXX)

# Set common directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")
set(KERNEL_DIR "${SRC_DIR}/kernel")
set(USER_DIR "${SRC_DIR}/user")
set(COMMON_DIR "${SRC_DIR}/common")

# Set module name
set(mod_name "trafficmon")

# Set kernel directory
execute_process(COMMAND uname -r OUTPUT_VARIABLE uname_r OUTPUT_STRIP_TRAILING_WHITESPACE)
set(kerneldir "/lib/modules/${uname_r}/build")
find_file(kernel_makefile NAMES Makefile PATHS ${kerneldir} NO_DEFAULT_PATH)
if(NOT kernel_makefile)
  message(FATAL_ERROR "There is no Makefile in kerneldir!")
endif()
message(STATUS "Kernel makefile: ${kernel_makefile}")

# User executables
set(USER_EXECUTABLES "${USER_DIR}/filter.cpp" "${USER_DIR}/test/net-simulator.cpp" "${USER_DIR}/test/test.cpp")

# Find source files
file(GLOB_RECURSE USER_SOURCE_FILES "${USER_DIR}/*.cpp" "${USER_DIR}/*.h" "${COMMON_DIR}/*.c" "${COMMON_DIR}/*.h")
file(GLOB_RECURSE MODULE_SOURCE_FILES "${KERNEL_DIR}/*.c" "${KERNEL_DIR}/*.h" "${COMMON_DIR}/*.c" "${COMMON_DIR}/*.h")

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

set(BUILD_TARGETS "module")

# Copy module source files for building
# Prepare module object files list
set(module_objects "")
foreach(src ${MODULE_SOURCE_FILES})
  file(RELATIVE_PATH rel_path ${SRC_DIR} ${src})
  configure_file(${src} kbuild-files/${rel_path} COPYONLY)
  if(src MATCHES "\\.c$")
    string(REPLACE ".c" ".o" object ${rel_path})
    string(APPEND module_objects ${object} " ")
  endif()
endforeach()

# Configure the kernel module Makefile
configure_file(${KERNEL_DIR}/Makefile.in "kbuild-files/Makefile" @ONLY)

# Add the user executable
foreach(executable ${USER_EXECUTABLES})
  # Extract the filename without extension to use as the executable name
  get_filename_component(exec_name ${executable} NAME_WE)

  # Initialize a list to hold sources for this executable
  set(current_exec_sources "")

  # Add the current executable's source
  list(APPEND current_exec_sources ${executable})

  # Add other sources except for the USER_EXECUTABLES
  foreach(src ${USER_SOURCE_FILES})
    # Check if src is not in USER_EXECUTABLES, except for the current executable
    list(FIND USER_EXECUTABLES ${src} src_index)
    if(src_index EQUAL -1 OR src STREQUAL executable)
      list(APPEND current_exec_sources ${src})
    endif()
  endforeach()

  # Add the executable with its specific source files
  set(CMAKE_BUILD_TYPE Debug)
  add_executable(${exec_name} ${current_exec_sources})

  # Set C++ standard
  set_property(TARGET ${exec_name} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${exec_name} PROPERTY CXX_STANDARD_REQUIRED ON)
  list(APPEND BUILD_TARGETS "${exec_name}")
endforeach()

# Kernel module build commands
set(module_cmd ${CMAKE_MAKE_PROGRAM} -C ${kerneldir} M=${CMAKE_BINARY_DIR}/kbuild-files)
add_custom_command(OUTPUT kbuild-files/${mod_name}.ko
  COMMAND ${module_cmd} modules
  DEPENDS ${MODULE_SOURCE_FILES} kbuild-files/Makefile
  VERBATIM)

add_custom_target(module DEPENDS kbuild-files/${mod_name}.ko)
add_custom_target(module-clean COMMAND ${module_cmd} clean)

# Post-build: Copy the kernel module to the build directory
add_custom_command(TARGET module POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy kbuild-files/${mod_name}.ko ${BUILD_DIR}/kernel/${mod_name}.ko
  COMMENT Copying ${mod_name}.ko to ${BUILD_DIR}/kernel)

add_custom_target(traffic-monitor DEPENDS ${BUILD_TARGETS})