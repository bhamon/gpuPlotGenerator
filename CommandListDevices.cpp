/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>
#include <cstdlib>
#include <CL/cl.h>

#include "util.h"
#include "CommandListDevices.h"
#include "OpenclError.h"

namespace cryo {
namespace gpuPlotGenerator {

CommandListDevices::CommandListDevices()
: Command("List the available OpenCL devices of the specified platform.") {
}

CommandListDevices::CommandListDevices(const CommandListDevices& p_command)
: Command(p_command) {
}

CommandListDevices::~CommandListDevices() throw () {
}

void CommandListDevices::help() const {
	std::cout << "Usage: ./gpuPlotGenerator listDevices <platformId>" << std::endl;
	std::cout << "    List the available OpenCL devices of the specified platform." << std::endl;
	std::cout << "Parameters:" << std::endl;
	std::cout << "    - platformId: The id of the platform to scan." << std::endl;
}

int CommandListDevices::execute(const std::vector<std::string>& p_args) {
	if(p_args.size() < 1) {
		help();
		return -1;
	}

	cl_uint platformId = atol(p_args[0].c_str());

	cl_platform_id* platforms = 0;
	cl_uint platformsNumber = 0;
	cl_device_id* devices = 0;
	cl_uint devicesNumber = 0;

	int returnCode = 0;

	try {
		cl_int error;

		error = clGetPlatformIDs(0, 0, &platformsNumber);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to retrieve the OpenCL platforms number");
		}

		if(platformId >= platformsNumber) {
			throw std::runtime_error("No platform found with the provided id");
		}

		platforms = new cl_platform_id[platformsNumber];
		error = clGetPlatformIDs(platformsNumber, platforms, 0);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to retrieve the OpenCL platforms");
		}

		error = clGetDeviceIDs(platforms[platformId], CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, 0, 0, &devicesNumber);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to retrieve the OpenCL devices number");
		}

		devices = new cl_device_id[devicesNumber];
		error = clGetDeviceIDs(platforms[platformId], CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, devicesNumber, devices, 0);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to retrieve the OpenCL devices");
		}

		std::vector<unsigned long long> sizeUnits {1024, 1024, 1024};
		std::vector<std::string> sizeLabels {"KB", "MB", "GB", "TB"};

		std::cout << "Devices number: " << devicesNumber << std::endl;
		for(cl_uint i = 0 ; i < devicesNumber ; ++i) {
			std::cout << "----" << std::endl;
			std::cout << "Id:                          " << i << std::endl;

			std::size_t size;
			char* buffer;

			cl_device_type type;
			clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(cl_device_type), (void*)&type, 0);
			std::cout << "Type:                        ";
			if(type & CL_DEVICE_TYPE_CPU) {
				std::cout << "CPU";
			} else if(type & CL_DEVICE_TYPE_GPU) {
				std::cout << "GPU";
			}
			std::cout << std::endl;

			clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 0, 0, &size);
			buffer = new char[size];
			clGetDeviceInfo(devices[i], CL_DEVICE_NAME, size, (void*)buffer, 0);
			std::cout << "Name:                        " << buffer << std::endl;
			delete[] buffer;

			clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, 0, 0, &size);
			buffer = new char[size];
			clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, size, (void*)buffer, 0);
			std::cout << "Vendor:                      " << buffer << std::endl;
			delete[] buffer;

			clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, 0, 0, &size);
			buffer = new char[size];
			clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, size, (void*)buffer, 0);
			std::cout << "Version:                     " << buffer << std::endl;
			delete[] buffer;

			clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, 0, 0, &size);
			buffer = new char[size];
			clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, size, (void*)buffer, 0);
			std::cout << "Driver version:              " << buffer << std::endl;
			delete[] buffer;

			cl_ulong maxGlobalMemSize;
			clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), (void*)&maxGlobalMemSize, 0);
			std::cout << "Max global memory size:      " << cryo::util::formatValue((unsigned long long)maxGlobalMemSize >> 10, sizeUnits, sizeLabels) << std::endl;

			cl_ulong maxMemAllocSize;
			clGetDeviceInfo(devices[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), (void*)&maxMemAllocSize, 0);
			std::cout << "Max memory allocation size:  " << cryo::util::formatValue((unsigned long long)maxMemAllocSize >> 10, sizeUnits, sizeLabels) << std::endl;

			cl_ulong maxLocalMemSize;
			clGetDeviceInfo(devices[i], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), (void*)&maxLocalMemSize, 0);
			std::cout << "Max local memory size:       " << cryo::util::formatValue((unsigned long long)maxLocalMemSize >> 10, sizeUnits, sizeLabels) << std::endl;

			std::size_t maxWorkGroupSize;
			clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE , sizeof(std::size_t), (void*)&maxWorkGroupSize, 0);
			std::cout << "Max work group size:         " << maxWorkGroupSize << std::endl;

			cl_uint maxClockFrequency;
			clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY , sizeof(cl_uint), (void*)&maxClockFrequency, 0);
			std::cout << "Max clock frequency:         " << maxClockFrequency << "MHz" << std::endl;

			cl_uint maxComputeUnits;
			clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS , sizeof(cl_uint), (void*)&maxComputeUnits, 0);
			std::cout << "Max compute units:           " << maxComputeUnits << std::endl;
		}
	} catch(const OpenclError& ex) {
		std::cout << "[ERROR] [" << ex.getCode() << "] " << ex.what() << std::endl;
		returnCode = -1;
	} catch(const std::exception& ex) {
		std::cout << "[ERROR] " << ex.what() << std::endl;
		returnCode = -1;
	}

	if(devices) { delete[] devices; }
	if(platforms) { delete[] platforms; }

	return returnCode;
}

}}
