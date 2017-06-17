cmake_minimum_required(VERSION 2.6)
project(gpuPlotGenerator)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIR})

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
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	list(APPEND GPU_PLOT_GENERATOR_SRC PlotsFile-linux.cpp)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	list(APPEND GPU_PLOT_GENERATOR_SRC PlotsFile-mac.cpp)
endif()

add_executable(gpuPlotGenerator ${GPU_PLOT_GENERATOR_SRC})

target_link_libraries(gpuPlotGenerator ${OpenCL_LIBRARY})
