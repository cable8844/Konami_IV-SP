cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Konami_iv_sp)

# Specify the source file
set(SOURCE_FILES src/server.c src/server_run.c)
set(SOURCE_INCLUDE_DIRS src)

# Add an executable
add_executable(server ${SOURCE_FILES})

# Set the C standard GNU99
set(CMAKE_C_STANDARD 99)

# Find the required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
# pkg_check_modules(NETCAT REQUIRED netcat) # Netcat separate runtime, not needed currently

# Include the directories for the packages
include_directories(${LIBXML2_INCLUDE_DIRS} ${NETCAT_INCLUDE_DIRS} ${SOURCE_INCLUDE_DIRS}) # Netcat separate runtime, not needed currently

# Link the libraries
target_link_libraries(server ${LIBXML2_LIBRARIES} ${NETCAT_LIBRARIES}) # Netcat separate runtime, not needed currently
# Disable optimizations and enable debug symbols if the build type is Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
endif()


# Installation / Packaging section

install(TARGETS server DESTINATION /usr/local/bin)

# Set the package name and version
set(CPACK_PACKAGE_NAME "Konami_iv_sp")
set(CPACK_PACKAGE_VERSION "0.0.1")

# Set the package file names
set(CPACK_DEBIAN_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_amd64.deb")
set(CPACK_RPM_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-1.x86_64.rpm")

# Specify the package type
set(CPACK_GENERATOR "DEB;RPM")
# Specify the package dependencies
set(CPACK_Konami_iv_sp_CONTACT "calebfarrand@gmail.com")
set(CPACK_DEBIAN_Konami_iv_sp_MAINTAINER "Caleb Farrand")
set(CPACK_DEBIAN_Konami_iv_sp_DEPENDS "libxml2-dev, netcat")
set(CPACK_RPM_Konami_iv_sp_REQUIRES "libxml2-devel, netcat")


# Repetitive code for the sake of example and redundancy. Will be used if CPACK_PACKAGE_NAME is not set
set(CPACK_PACKAGE_CONTACT "calebfarrand@gmail.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Caleb Farrand")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libxml2-dev, netcat")
set(CPACK_RPM_PACKAGE_REQUIRES "libxml2-devel, netcat")

# Include CPack for packaging
include(CPack)
