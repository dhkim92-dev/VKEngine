cmake_minimum_required(VERSION 3.10)
PROJECT(VKEngine VERSION 0.1.3)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-DGLFW_INCLUDE_VULKAN -DVK_ENGINE_DEBUG)
ADD_COMPILE_OPTIONS(-Wall)
add_subdirectory(externals/glm)
SET(VULKAN_SDK $ENV{VULKAN_SDK})

IF(WIN32)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY lib)
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
	SET(GLFW_PATH $ENV{GLFW_PATH})
	INCLUDE_DIRECTORIES(${VULKAN_SDK}/include)
	LINK_DIRECTORIES(${VULKAN_SDK}/Lib)
	INCLUDE_DIRECTORIES(${GLFW_PATH}/include)
	LINK_DIRECTORIES(${GLFW_PATH}/lib)
	INCLUDE_DIRECTORIES(externals/glm/)
	LINK_LIBRARIES(vulkan-1)
	LINK_LIBRARIES(glfw3)
	SET(GLFW_LINK glfw3)
ELSE()
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
	INCLUDE_DIRECTORIES(${VULKAN_SDK}/include)
	LINK_DIRECTORIES(${VULKAN_SDK}/lib)
	INCLUDE_DIRECTORIES(externals/glm/ /usr/local/include /usr/include)
    LINK_DIRECTORIES(/usr/local/lib /usr/lib)
	LINK_LIBRARIES(vulkan)
	LINK_LIBRARIES(glfw)
	SET(GLFW_LINK glfw)
ENDIF()

MESSAGE("VULKAN SDK PATH : ${VULKAN_SDK}")
MESSAGE("GLFW PATH : ${GLFW_PATH}")

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
target_include_directories(VKEngine PRIVATE src)
#target_link_libraries(VKEngine PRIVATE ${GLFW_LINK})
