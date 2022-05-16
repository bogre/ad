cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(srk)

add_executable(srk main_ad.cpp)
set_target_properties(srk PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_compile_features(srk PUBLIC cxx_std_20)
target_compile_options(srk PUBLIC
    # -fsanitize=address,undefined
    -fno-builtin
    -Wall
    -Wextra
)
target_include_directories(srk PUBLIC 3pty 3pty/boost)
target_link_options(srk PUBLIC
    # -fsanitize=address,undefined
)
add_custom_target(
    clang-tidy-check clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json
    -checks=cert* ${PROJECT_SOURCE_DIR}/main_ad.cpp
    )
add_custom_target(check DEPENDS clang-tidy-check)
