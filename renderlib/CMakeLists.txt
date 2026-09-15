add_library(renderlib INTERFACE)

target_include_directories(renderlib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)
add_executable(mainCode
  mainCode.cpp # if you have an executable, list its .cpp file(s) here
)

target_link_libraries(mainCode PRIVATE renderlib)