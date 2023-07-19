cmake_minimum_required(VERSION 3.10)
project(ofono-apndb-plugin VERSION 0.20230719 LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(OFONO REQUIRED ofono)

pkg_get_variable(OFONO_PLUGINDIR ofono plugindir)

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
)

set_target_properties(apndbplugin PROPERTIES PREFIX "")

install(TARGETS apndbplugin DESTINATION "${OFONO_PLUGINDIR}")
