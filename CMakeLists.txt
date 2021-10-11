cmake_minimum_required(VERSION 3.10)

PROJECT(VKEngine VERSION 0.1.1)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)


add_definitions(-DGLFW_INCLUDE_VULKAN -DDEBUG)
ADD_COMPILE_OPTIONS(-Wall)
LINK_LIBRARIES(vulkan glfw )
LINK_DIRECTORIES(/usr/lib /usr/local/lib)
INCLUDE_DIRECTORIES(/usr/local/include)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)

ADD_LIBRARY(VKEngine SHARED 
			core/src/vk_engine.cpp
			core/src/vk_context.cpp
			core/src/vk_swapchain.cpp
			core/src/vk_buffer.cpp
			core/src/vk_image.cpp
			core/src/vk_queue.cpp
			core/src/vk_queue_family.cpp
			core/src/vk_utils.cpp
			core/src/vk_compute.cpp
			core/src/vk_program.cpp
			core/src/vk_shader.cpp
			core/src/vk_infos.cpp
			core/src/vk_application.cpp)
MESSAGE([ STATUS ]"PROJECT VERSION :  ${CMAKE_PROJECT_VERSION}")
set_target_properties(VKEngine PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(VKEngine PROPERTIES SOVERSION 0)
target_include_directories(VKEngine PRIVATE include)
target_include_directories(VKEngine PRIVATE core/src)
