cmake_minimum_required(VERSION 3.16)
project(BTWTutorial)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

include(CheckIncludeFile)

#set(PREPARED_FILES TRUE)

set(BIN_ROOT ${PROJECT_SOURCE_DIR}/bin)
set(CXX_FLAGS_DEBUG -O0 -g)
set(CXX_FLAGS_RELEASE -O2)

if (DEFINED PREPARED_FILES)
  set(COMPILE_DEFINITIONS )
else()
  set(COMPILE_DEFINITIONS LIVE_SESSION)
endif()

if (DEFINED GENERATE_TSL)
  if (DEFINED LOCAL_TSL_PATH)
    set(TSL_PATH ${LOCAL_TSL_PATH})
  else()
    set(TSL_PATH ${PROJECT_SOURCE_DIR}/3rdparty/tslgen)
  endif()
  include(${TSL_PATH}/tsl.cmake)
  if (DEFINED ArchId)
    create_tsl(
      USE_CONCEPTS
      TARGET_ARCHID ${ArchId}
      TSLGENERATOR_DIRECTORY ${TSL_PATH}
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tsl
    )
  else()
    create_tsl(
      USE_CONCEPTS
      TSLGENERATOR_DIRECTORY ${TSL_PATH}
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tsl
    )
  endif()
  set(SYSTEM_TSL FALSE)
else()
  find_file(HAS_TSL_INSTALLED
    NAMES tsl.hpp
    PATHS /usr/include/tsl
    NO_DEFAULT_PATH  # Prevents searching in default locations
  )
  if (NOT HAS_TSL_INSTALLED)
    execute_process(
      COMMAND bash "-c" "curl -L -s \"https://github.com/db-tu-dresden/TSL/releases/latest/download/setup_tsl.sh\" | /bin/bash -s -- --local ${CMAKE_CURRENT_BINARY_DIR}"
      RESULT_VARIABLE TSL_INSTALL_RESULT
      ERROR_VARIABLE TSL_INSTALL_ERROR
    )
    if (NOT TSL_INSTALL_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to install TSL: ${TSL_INSTALL_ERROR}")
    endif()
  else ()
    set(SYSTEM_TSL TRUE)
  endif()
  set(TSL_COMPILE_OPTIONS -march=native -Wno-ignored-attributes -Wno-attributes -Wdeprecated-declarations -flax-vector-conversions)
endif()


function(create_target)
  set(oneValueArgs TARGET_NAME)
  set(multiValueArgs SRCS INCLUDE_DIRS COMPILE_DEFS COMPILE_OPTS LINK_OPTS LIBS)
  cmake_parse_arguments(CT "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if((NOT DEFINED CT_TARGET_NAME) OR (NOT CT_TARGET_NAME))
    message(FATAL_ERROR "Target name not provided")
  endif()
  add_executable(${CT_TARGET_NAME} ${CT_SRCS})
  set_target_properties(${CT_TARGET_NAME} PROPERTIES 
    CXX_STANDARD 23 
    CXX_STANDARD_REQUIRED ON
  )
  set_target_properties(${CT_TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_ROOT}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_ROOT}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_ROOT}/Release
  )
  set_target_properties(${CT_TARGET_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
  )
  
  target_include_directories(${CT_TARGET_NAME} PRIVATE ${CT_INCLUDE_DIRS})
  target_compile_definitions(${CT_TARGET_NAME} PRIVATE ${CT_COMPILE_DEFS} ${COMPILE_DEFINITIONS})
  target_compile_options(
    ${CT_TARGET_NAME} PRIVATE 
      ${CT_COMPILE_OPTS} 
      $<$<CONFIG:Release>:${CXX_FLAGS_RELEASE}> 
      $<$<CONFIG:Debug>:${CXX_FLAGS_DEBUG}>
  )

  target_link_options(${CT_TARGET_NAME} PRIVATE ${CT_LINK_OPTS})
  if(SYSTEM_TSL)
    target_compile_options(${CT_TARGET_NAME} PRIVATE ${TSL_COMPILE_OPTIONS})
  else()
    target_include_directories(${CT_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${CT_TARGET_NAME} tsl ${CT_LIBS})
  endif()
endfunction()

if(NOT DEFINED CMAKE_CONFIGURATION_TYPES AND NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Debug or Release)" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

message(STATUS "Configured Build Type: ${CMAKE_BUILD_TYPE}")



create_target(
  TARGET_NAME hello_tsl
  SRCS src/hello_tsl.cpp
  INCLUDE_DIRS src
)


create_target(
  TARGET_NAME filter_agg
  SRCS src/filter_aggregate.cpp
  INCLUDE_DIRS src
)