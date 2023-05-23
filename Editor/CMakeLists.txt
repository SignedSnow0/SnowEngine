add_executable (Editor "Source/Editor.h" 
					   "Source/Editor.cpp" 
					   "Source/EntityView.cpp"
					   "Source/EntityView.h" 
					   "Source/ParticleTest.cpp"
					   "Source/SceneView.cpp"
					   "Source/SceneView.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Editor PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(Editor PUBLIC "../Engine/Source/"
										 "../Engine/External/glfw/include/")

target_link_libraries(Editor Engine)