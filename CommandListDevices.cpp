/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>
#include <cstdlib>
#include <memory>
#include <exception>

#include "util.h"
#include "OpenclError.h"
#include "OpenclPlatform.h"
#include "OpenclDevice.h"
#include "CommandListDevices.h"

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

	try {
		std::size_t platformId = std::atol(p_args[0].c_str());

		std::vector<std::shared_ptr<OpenclPlatform>> platforms(OpenclPlatform::list());
		if(platformId >= platforms.size()) {
			throw std::runtime_error("No platform found with the provided id");
		}

		std::vector<unsigned long long> sizeUnits {1024, 1024, 1024};
		std::vector<std::string> sizeLabels {"KB", "MB", "GB", "TB"};

		std::vector<std::shared_ptr<OpenclDevice>> devices(OpenclDevice::list(platforms[platformId]));
		std::cout << "Devices number: " << devices.size() << std::endl;

		std::size_t i = 0;
		for(const std::shared_ptr<OpenclDevice>& device : devices) {
			std::cout << "----" << std::endl;
			std::cout << "Id:                          " << i++ << std::endl;
			std::cout << "Type:                        " << device->getType() << std::endl;
			std::cout << "Name:                        " << device->getName() << std::endl;
			std::cout << "Vendor:                      " << device->getVendor() << std::endl;
			std::cout << "Version:                     " << device->getVersion() << std::endl;
			std::cout << "Driver version:              " << device->getDriverVersion() << std::endl;
			std::cout << "Max clock frequency:         " << device->getMaxClockFrequency() << "MHz" << std::endl;
			std::cout << "Max compute units:           " << device->getMaxComputeUnits() << std::endl;
			std::cout << "Global memory size:          " << cryo::util::formatValue(device->getGlobalMemorySize() >> 10, sizeUnits, sizeLabels) << std::endl;
			std::cout << "Max memory allocation size:  " << cryo::util::formatValue(device->getMaxMemoryAllocationSize() >> 10, sizeUnits, sizeLabels) << std::endl;
			std::cout << "Max work group size:         " << device->getMaxWorkGroupSize() << std::endl;
			std::cout << "Local memory size:           " << cryo::util::formatValue(device->getLocalMemorySize() >> 10, sizeUnits, sizeLabels) << std::endl;

			std::vector<std::size_t> maxWorkItemSizes(device->getMaxWorkItemSizes());
			std::cout << "Max work-item sizes:         (" << cryo::util::join(maxWorkItemSizes.begin(), maxWorkItemSizes.end(), ", ") << ")" << std::endl;
		}
	} catch(const OpenclError& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR][" << ex.getCode() << "][" << ex.getCodeString() << "] " << ex.what() << std::endl;
		return -1;
	} catch(const std::exception& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR] " << ex.what() << std::endl;
		return -1;
	}

	return 0;
}

}}
