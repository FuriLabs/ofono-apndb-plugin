cmake_minimum_required(VERSION 3.10)
project(ofono-apndb-plugin VERSION 0.20230719 LANGUAGES C)

set(
    OFONO_PKG "ofono"
    CACHE STRING
    "oFono package to build against"
)

option(
    UBUNTU_API
    "Whether oFono variant has Ubuntu-modified provisioning API"
    FALSE
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(OFONO REQUIRED ${OFONO_PKG})

pkg_get_variable(OFONO_PLUGINDIR ${OFONO_PKG} plugindir)

set(
    SYSTEM_APNDB_PATH "/system/etc/apns-conf.xml"
    CACHE STRING
    "Path for system-level apns-conf.xml"
)

set(
    CUSTOM_APNDB_PATH "/custom/etc/apns-conf.xml"
    CACHE STRING
    "Path for apns-conf.xml from customization tarball"
)

add_library(apndbplugin MODULE
    src/ubuntu-apndb.c
    src/ubuntu-apndb.h
    src/apndb-provision.c
)

target_include_directories(apndbplugin PRIVATE
    ${GLIB_INCLUDE_DIRS}
    ${OFONO_INCLUDE_DIRS}
)

target_link_libraries(apndbplugin
    ${GLIB_LIBRARIES}
    # Do not include oFono; ofono.pc adds -export-symbols-regex linker option
    # which doesn't work on all platforms.
)

target_compile_options(apndbplugin PRIVATE
    -fvisibility=hidden
)

target_compile_definitions(apndbplugin PRIVATE
    VERSION="${CMAKE_PROJECT_VERSION}"
    SYSTEM_APNDB_PATH="${SYSTEM_APNDB_PATH}"
    CUSTOM_APNDB_PATH="${CUSTOM_APNDB_PATH}"
)

if (UBUNTU_API)
    target_compile_definitions(apndbplugin PRIVATE
        UBUNTU_API
    )
endif ()

set_target_properties(apndbplugin PROPERTIES PREFIX "")

install(TARGETS apndbplugin DESTINATION "${OFONO_PLUGINDIR}")
