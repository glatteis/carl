include(${CMAKE_SOURCE_DIR}/cmake/carlmacros.cmake)

add_definitions(-fPIC)

file(GLOB_RECURSE carl-settings_sources "*.cpp")

add_new_libraries(carl-settings "${PROJECT_VERSION_LIB}" "${carl-settings_sources}" "Boost_program_options_SHARED")
target_include_dirs_from(carl-settings-objects SYSTEM PUBLIC Boost_program_options_SHARED)
target_link_libraries(carl-settings-shared Boost_program_options_SHARED ${LIB_FILESYSTEM})
target_link_libraries(carl-settings-static Boost_program_options_STATIC ${LIB_FILESYSTEM})

install_libraries(carl carl-settings-shared carl-settings-static)
