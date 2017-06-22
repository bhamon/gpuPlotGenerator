cmake_minimum_required(VERSION 2.6)
project(gpuPlotGenerator)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIR})

configure_file("${PROJECT_SOURCE_DIR}/GenerationDevice.h.in" "${PROJECT_BINARY_DIR}/GenerationDevice.h")
configure_file("${PROJECT_SOURCE_DIR}/OpenclDevice.h.in" "${PROJECT_BINARY_DIR}/OpenclDevice.h")
configure_file("${PROJECT_SOURCE_DIR}/OpenclError.h.in" "${PROJECT_BINARY_DIR}/OpenclError.h")
configure_file("${PROJECT_SOURCE_DIR}/OpenclPlatform.h.in" "${PROJECT_BINARY_DIR}/OpenclPlatform.h")

include_directories("${PROJECT_BINARY_DIR}")

set(GPU_PLOT_GENERATOR_SRC
	Command.cpp
	CommandGenerate.cpp
	CommandHelp.cpp
	CommandListDevices.cpp
	CommandListPlatforms.cpp
	CommandSetup.cpp
	CommandVerify.cpp
	DeviceConfig.cpp
	GenerationConfig.cpp
	GenerationContext.cpp
	GenerationContextBuffer.cpp
	GenerationContextDirect.cpp
	GenerationDevice.cpp
	GenerationWork.cpp
	gpuPlotGenerator.cpp
	OpenclDevice.cpp
	OpenclError.cpp
	OpenclPlatform.cpp
	PlotsFile.cpp
	util.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	list(APPEND GPU_PLOT_GENERATOR_SRC PlotsFile-win.cpp)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	list(APPEND GPU_PLOT_GENERATOR_SRC PlotsFile-linux.cpp)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	list(APPEND GPU_PLOT_GENERATOR_SRC PlotsFile-mac.cpp)
endif()

add_executable(gpuPlotGenerator ${GPU_PLOT_GENERATOR_SRC})

target_link_libraries(gpuPlotGenerator ${CMAKE_THREAD_LIBS_INIT} ${OpenCL_LIBRARY})
