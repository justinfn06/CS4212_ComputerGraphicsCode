add_library (cs4212-util
  ArgumentParsing.cpp ArgumentParsing.h
  handleGraphicsArgs.cpp handleGraphicsArgs.h
  model_obj.cpp model_obj.h

  ../renderlib/cameras/camera.cpp
  ../renderlib/cameras/perspectiveCamera.cpp
  ../renderlib/geometry/sphere.cpp
  ../renderlib/geometry/triangle.cpp
  ../renderlib/output/Framebuffer.cpp
  ../renderlib/scene/scene.cpp

)

target_include_directories(cs4212-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(cs4212-util PUBLIC HAS_GLM)
target_link_libraries(cs4212-util PRIVATE Boost::program_options)
target_link_libraries(cs4212-util PUBLIC glm::glm)
target_link_libraries(cs4212-util PUBLIC PNG::PNG ZLIB::ZLIB)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(mainCode
    ../renderlib/mainCode.cpp
    ../renderlib/loading/SceneParser_JSON.cpp
)

target_link_libraries(mainCode PRIVATE
    cs4212-util
    nlohmann_json::nlohmann_json
)

find_package(OpenMP REQUIRED)
target_link_libraries(mainCode PRIVATE OpenMP::OpenMP_CXX)