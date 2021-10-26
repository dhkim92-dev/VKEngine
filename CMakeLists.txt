cmake_minimum_required(VERSION 3.10)
PROJECT(VKEngine VERSION 0.1.3)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)


add_definitions(-DGLFW_INCLUDE_VULKAN -DDEBUG)
ADD_COMPILE_OPTIONS(-Wall)
LINK_LIBRARIES(vulkan glfw )

LINK_DIRECTORIES(/usr/lib /usr/local/lib)
INCLUDE_DIRECTORIES(/usr/local/include)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)

ADD_LIBRARY(VKEngine SHARED 
			src/vk_engine.cpp
			src/vk_context.cpp
			src/vk_swapchain.cpp
			src/vk_buffer.cpp
			src/vk_image.cpp
			src/vk_queue.cpp
			src/vk_queue_family.cpp
			src/vk_utils.cpp
			src/vk_compute.cpp
			src/vk_program.cpp
			src/vk_shader.cpp
			src/vk_infos.cpp
			src/vk_application.cpp)
MESSAGE([ STATUS ]"PROJECT VERSION :  ${CMAKE_PROJECT_VERSION}")
set_target_properties(VKEngine PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(VKEngine PROPERTIES SOVERSION 0)
target_include_directories(VKEngine PRIVATE include)
target_include_directories(VKEngine PRIVATE core/src)
