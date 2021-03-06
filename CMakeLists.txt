cmake_minimum_required(VERSION 3.0.0)

project(fanControl  VERSION 1.0.0
        DESCRIPTION "Fan speed controller for hwmon fans and sensors driver. \
 It controls fan speeds based on preconfigured temperatures ranges.")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compile settings
set(CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_COMPILER_IS_GNUCXX True)
find_package(Threads REQUIRED)

# Include modules
include(GNUInstallDirs)
include(InstallRequiredSystemLibraries)

# Options
option(WHITHOUT_LIBS 
       "Install without shared libs. All in one binary file." 
       ON)
option(SET_PERMS_ON_BUILD 
       "Set root owner and suid on build, needed sudo privileges on build." 
       OFF)

# Some usefull variables ##################################################
set(SRC_DIR ${PROJECT_SOURCE_DIR})
set(BUILD_DIR ${PROJECT_BINARY_DIR})
set(SCRIPTS_DIR ${SRC_DIR}/scripts)
set(INCLUDE_DIR ${BUILD_DIR}/include)
set(LOG_DIR ${BUILD_DIR}/logs)
set(SRC_FILES src/main.cpp src/config_menu.cpp src/Sensors.cpp)
set(LIB_FILES lib/utils.cpp lib/menu.cpp)
set(cmake ${CMAKE_COMMAND})
set(found_hddtemp "whereis hddtemp 2> /dev/null\
                   | sed 's, ,\\n,g' | grep bin | tail -n+1")

# hdd binary path variable
execute_process(COMMAND bash -c ${found_hddtemp}
                OUTPUT_VARIABLE HDDTEMP_BIN
                COMMAND_ECHO NONE
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# Default build release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Log directory for additional cmake scripts output
make_directory(${LOG_DIR})

# Generic header locations
include_directories(${INCLUDE_DIR})
        
# Config file
configure_file(fanControlConfig.h.in 
               ${INCLUDE_DIR}/fanControlConfig.h)

# Scripts ################################################################
# Script build
configure_file(${SCRIPTS_DIR}/build.cmake.in 
               ${BUILD_DIR}/build.cmake)
# Script install
configure_file(${SCRIPTS_DIR}/install.cmake.in 
               ${BUILD_DIR}/install.cmake)
# Script uninstall
configure_file(${SCRIPTS_DIR}/cmake_uninstall.cmake.in 
               ${BUILD_DIR}/cmake_uninstall.cmake
               IMMEDIATE @ONLY)

# Executable ##############################################################
# Executable build libraries linked on one executable file ################
if(WHITHOUT_LIBS)
  add_executable(fanControl ${SRC_FILES} ${LIB_FILES})

  include_directories(lib)
  
# Executable build with shared libs linked and include on install
else()
  add_executable(fanControl ${SRC_FILES})

  add_subdirectory(lib)

  list(APPEND EXTRA_LIBS menu utils)
  target_link_libraries(fanControl PUBLIC menu utils)
endif()

# Thread compile option
target_link_options(fanControl PUBLIC -pthread)

## set root owner and suid guid perms for debugging on build
if(SET_PERMS_ON_BUILD)
add_custom_command(TARGET fanControl POST_BUILD 
                   COMMAND ${cmake} -P ${BUILD_DIR}/build.cmake)
endif()

# uninstall target (to use with make "make uninstall") ####################
add_custom_target(uninstall
                  COMMAND ${cmake} -P ${BUILD_DIR}/cmake_uninstall.cmake)

# install process #########################################################
install(FILES ${BUILD_DIR}/fanControl DESTINATION bin)
install(FILES ${INCLUDE_DIR}/fanControlConfig.h DESTINATION include)
install(SCRIPT ${BUILD_DIR}/install.cmake)

# CPack configuration #####################################################
set(CPACK_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_PACKAGE_NAME "fanControl-b4fThrive")
set(CPACK_PACKAGE_VENDOR "b4fThrive")
set(CPACK_RESOURCE_FILE_LICENSE ${SRC_DIR}/LICENSE)
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/b4fThrive/fanControl")

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "b4f.thrive@gmail.com")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "hddtemp")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${SCRIPTS_DIR}/cPack/postinst)

set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_LICENSE ${SRC_DIR}/LICENSE)
set(CPACK_RPM_PACKAGE_SUGGESTS "hddtemp")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${SCRIPTS_DIR}/cPack/postinst)

include(CPack)
