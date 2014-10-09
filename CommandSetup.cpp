/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>
#include <memory>
#include <algorithm>
#include <exception>

#include "util.h"
#include "OpenclError.h"
#include "OpenclPlatform.h"
#include "OpenclDevice.h"
#include "GenerationConfig.h"
#include "OpenclError.h"
#include "CommandSetup.h"

namespace cryo {
namespace gpuPlotGenerator {

CommandSetup::CommandSetup()
: Command("Step by step setup guide.") {
}

CommandSetup::CommandSetup(const CommandSetup& p_command)
: Command(p_command) {
}

CommandSetup::~CommandSetup() throw () {
}

void CommandSetup::help() const {
	std::cout << "Usage: ./gpuPlotGenerator setup" << std::endl;
	std::cout << "    Launch a step by step setup guide to create the [devices.txt] file." << std::endl;
}

int CommandSetup::execute(const std::vector<std::string>&) {
	int returnCode = 0;

	try {
		std::cout << "Loading platforms..." << std::endl;
		std::vector<std::shared_ptr<OpenclPlatform>> platforms(OpenclPlatform::list());

		std::cout << "Loading devices..." << std::endl;
		std::vector<std::vector<std::shared_ptr<OpenclDevice>>> devices;
		for(const std::shared_ptr<OpenclPlatform>& platform : platforms) {
			devices.push_back(OpenclDevice::list(platform));
		}

		std::cout << "Loading devices configurations..." << std::endl;
		std::vector<std::shared_ptr<GenerationConfig>> configs(GenerationConfig::loadFromFile(DEVICES_FILE));

		std::vector<unsigned long long> sizeUnits {1024, 1024};
		std::vector<std::string> sizeLabels {"MB", "GB", "TB"};

		bool run = true;
		while(run) {
			std::cout << "----" << std::endl;
			std::cout << "1. List all devices" << std::endl;
			std::cout << "2. List configured devices" << std::endl;
			std::cout << "3. Add device config" << std::endl;
			std::cout << "4. Remove device config" << std::endl;
			std::cout << "9. Save config" << std::endl;
			std::cout << "0. Quit" << std::endl;

			unsigned int action;
			std::cout << std::endl;
			std::cout << "> Select an option: ";
			std::cin >> action;
			std::cout << std::endl;

			switch(action) {
				case 1:{
					std::cout << "----" << std::endl;
					std::size_t i = 0;
					for(const std::shared_ptr<OpenclPlatform>& platform : platforms) {
						std::cout << "[" << i << "] " << platform->getName() << " (" << platform->getVersion() << ")" << std::endl;

						std::size_t j = 0;
						for(const std::shared_ptr<OpenclDevice>& device : devices[i]) {
							std::cout << "    [" << j << "] " << device->getName() << " (" << device->getVersion() << ")" << std::endl;
						}
					}
				}
				break;
				case 2:{
					std::cout << "Number of configured devices: " << configs.size() << std::endl;

					std::size_t i = 0;
					for(const std::shared_ptr<GenerationConfig>& config : configs) {
						std::cout << "----" << std::endl;
						std::cout << "Index:             " << i++ << std::endl;
						std::cout << "Platform:          " << config->getPlatform() << std::endl;
						std::cout << "Device:            " << config->getDevice() << std::endl;
						std::cout << "Global work size:  " << config->getGlobalWorkSize() << std::endl;
						std::cout << "Local work size:   " << config->getLocalWorkSize() << std::endl;
						std::cout << "Hashes number:     " << config->getHashesNumber() << std::endl;
						std::cout << "Used memory:       " << cryo::util::formatValue(config->getBufferSize() >> 20, sizeUnits, sizeLabels) << std::endl;
					}
				}
				break;
				case 3:{
					unsigned int platformId;
					std::cout << "Platform id: ";
					std::cin >> platformId;

					if(platformId >= platforms.size()) {
						std::cout << "[ERROR] No platform found with the provided id" << std::endl;
						break;
					}

					unsigned int deviceId;
					std::cout << "Device id: ";
					std::cin >> deviceId;

					if(deviceId >= devices[platformId].size()) {
						std::cout << "[ERROR] No device found with the provided id" << std::endl;
						break;
					}

					std::shared_ptr<OpenclDevice> device(devices[platformId][deviceId]);
					std::shared_ptr<GenerationConfig> config(new GenerationConfig(
						platformId,
						deviceId,
						std::min(device->getGlobalMemorySize() / PLOT_SIZE, device->getMaxMemoryAllocationSize() / PLOT_SIZE),
						1,
						PLOT_SIZE / HASH_SIZE
					));

					config->normalize();

					std::cout << "Global work size (" << config->getGlobalWorkSize() << " recommended): ";
					std::cin >> config->globalWorkSize();

					std::vector<std::size_t> maxWorkItemSizes(device->getMaxWorkItemSizes());
					config->setLocalWorkSize(std::max((std::size_t)1, maxWorkItemSizes[0] / 4) * 3);
					config->normalize();

					std::cout << "Local work size (" << config->getLocalWorkSize() << " recommended): ";
					std::cin >> config->localWorkSize();

					std::cout << "Hashes number (" << config->getHashesNumber() << " recommended): ";
					std::cin >> config->hashesNumber();

					std::cout << "----" << std::endl;
					std::cout << "Normalizing config..." << std::endl;
					config->normalize();

					configs.push_back(config);
					std::cout << "New config added" << std::endl;
				}
				break;
				case 4:{
					std::size_t configIndex;
					std::cout << "Config index: ";
					std::cin >> configIndex;

					if(configIndex >= configs.size()) {
						std::cout << "[ERROR] No config found with the provided index" << std::endl;
						break;
					}

					configs.erase(configs.begin() + configIndex);
					std::cout << "Config removed" << std::endl;
				}
				break;
				case 9:{
					GenerationConfig::storeToFile(DEVICES_FILE, configs);
					std::cout << "Config saved" << std::endl;
				}
				break;
				case 0:
					run = false;
				break;
			}

			std::cout << std::endl;
		}
	} catch(const OpenclError& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR][" << ex.getCode() << "][" << ex.getCodeString() << "] " << ex.what() << std::endl;
		returnCode = -1;
	} catch(const std::exception& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR] " << ex.what() << std::endl;
		returnCode = -1;
	}

	return returnCode;
}

}}
