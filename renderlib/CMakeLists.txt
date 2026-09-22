add_library(renderlib INTERFACE)

target_include_directories(renderlib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)