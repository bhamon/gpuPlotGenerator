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

#include "CommandListPlatforms.h"
#include "OpenclError.h"

namespace cryo {
namespace gpuPlotGenerator {

CommandListPlatforms::CommandListPlatforms()
: Command("List the available OpenCL platforms.") {
}

CommandListPlatforms::CommandListPlatforms(const CommandListPlatforms& p_command)
: Command(p_command) {
}

CommandListPlatforms::~CommandListPlatforms() throw () {
}

void CommandListPlatforms::help() const {
	std::cout << "Usage: ./gpuPlotGenerator listPlatforms" << std::endl;
	std::cout << "    List the available OpenCL platforms." << std::endl;
}

int CommandListPlatforms::execute(const std::vector<std::string>&) {
	cl_platform_id* platforms = 0;
	cl_uint platformsNumber = 0;

	int returnCode = 0;

	try {
		cl_int error;

		error = clGetPlatformIDs(0, 0, &platformsNumber);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to retrieve the OpenCL platforms number");
		}

		platforms = new cl_platform_id[platformsNumber];
		error = clGetPlatformIDs(platformsNumber, platforms, 0);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to retrieve the OpenCL platforms");
		}

		std::cout << "Platforms number: " << platformsNumber << std::endl;
		for(cl_uint i = 0 ; i < platformsNumber ; ++i) {
			std::cout << "----" << std::endl;
			std::cout << "Id:       " << i << std::endl;

			std::size_t size;
			char* buffer;

			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, 0, &size);
			buffer = new char[size];
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, size, (void*)buffer, 0);
			std::cout << "Name:     " << buffer << std::endl;
			delete[] buffer;

			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 0, 0, &size);
			buffer = new char[size];
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, size, (void*)buffer, 0);
			std::cout << "Vendor:   " << buffer << std::endl;
			delete[] buffer;

			clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, 0, 0, &size);
			buffer = new char[size];
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, size, (void*)buffer, 0);
			std::cout << "Version:  " << buffer << std::endl;
			delete[] buffer;
		}
	} catch(const OpenclError& ex) {
		std::cout << "[ERROR] An OpenCL error occured in the generation process, aborting..." << std::endl;
		std::cout << "[ERROR] [" << ex.getCode() << "][" << ex.getCodeString() << "] " << ex.what() << std::endl;
		returnCode = -1;
	} catch(const std::exception& ex) {
		std::cout << "[ERROR] An error occured in the generation process, aborting..." << std::endl;
		std::cout << "[ERROR] " << ex.what() << std::endl;
		returnCode = -1;
	}

	if(platforms) { delete[] platforms; }

	return returnCode;
}

}}
