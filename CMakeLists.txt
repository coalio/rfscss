cmake_minimum_required(VERSION 3.12)

# Set the standard to C++17
set(CMAKE_CXX_STANDARD 17)

project(main)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
add_subdirectory(src)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "rfscss")
target_link_libraries(${PROJECT_NAME} PUBLIC rfscss-src)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Install target
if (NOT BIN_INSTALL_DIR)
    set(BIN_INSTALL_DIR bin)
endif ()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
)
