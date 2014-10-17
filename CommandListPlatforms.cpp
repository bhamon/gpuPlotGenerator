/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>
#include <vector>
#include <memory>
#include <exception>

#include "OpenclError.h"
#include "OpenclPlatform.h"
#include "CommandListPlatforms.h"

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
	try {
		std::vector<std::shared_ptr<OpenclPlatform>> platforms(OpenclPlatform::list());
		std::cout << "Platforms number: " << platforms.size() << std::endl;

		std::size_t i = 0;
		for(const std::shared_ptr<OpenclPlatform>& platform : platforms) {
			std::cout << "----" << std::endl;
			std::cout << "Id:       " << i++ << std::endl;
			std::cout << "Name:     " << platform->getName() << std::endl;
			std::cout << "Vendor:   " << platform->getVendor() << std::endl;
			std::cout << "Version:  " << platform->getVersion() << std::endl;
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
