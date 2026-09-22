add_library (cs4212-util
  ArgumentParsing.cpp ArgumentParsing.h
  handleGraphicsArgs.cpp handleGraphicsArgs.h
  model_obj.cpp model_obj.h
  ../renderlib/vec3.h
  ../renderlib/color.h
  ../renderlib/Framebuffer.cpp
  ../renderlib/Framebuffer.h
)

target_include_directories(cs4212-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(cs4212-util PUBLIC HAS_GLM)
target_link_libraries(cs4212-util PRIVATE Boost::program_options)
target_link_libraries(cs4212-util PUBLIC glm::glm)
target_link_libraries(cs4212-util PUBLIC PNG::PNG ZLIB::ZLIB)

add_executable(mainCode ../renderlib/mainCode.cpp)
target_link_libraries(mainCode PRIVATE cs4212-util)