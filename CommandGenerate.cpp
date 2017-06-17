/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <list>
#include <thread>

#include "constants.h"
#include "util.h"
#include "OpenclError.h"
#include "OpenclPlatform.h"
#include "OpenclDevice.h"
#include "GenerationDevice.h"
#include "GenerationConfig.h"
#include "PlotsFile.h"
#include "GenerationContext.h"
#include "GenerationContextBuffer.h"
#include "GenerationContextDirect.h"
#include "GenerationWork.h"
#include "CommandGenerate.h"

namespace cryo {
namespace gpuPlotGenerator {

CommandGenerate::CommandGenerate()
: Command("Plot generation.") {
}

CommandGenerate::CommandGenerate(const CommandGenerate& p_command)
: Command(p_command) {
}

CommandGenerate::~CommandGenerate() throw () {
}

void CommandGenerate::help() const {
	std::cout << "Usage: ./gpuPlotGenerator generate <writerType> <plotsFiles...>" << std::endl;
	std::cout << "    Generate plots using the configured devices and write them to the specified files." << std::endl;
	std::cout << "Parameters:" << std::endl;
	std::cout << "    - writerType: Writer type to use to write the output files." << std::endl;
	std::cout << "                 Specify [buffer] to stack nonces in a buffer before writing them." << std::endl;
	std::cout << "                 Specify [direct] to write nonces directly to files." << std::endl;
	std::cout << "                     The <staggerSize> value will be used to configure the temporary RAM buffer size." << std::endl;
	std::cout << "                     The output file will always have <staggerSize> = <noncesNumber>." << std::endl;
	std::cout << "    - plotsFiles: A space-separated list of output files to generate." << std::endl;
	std::cout << "                  The file name has to be [<address>_<startNonce>_<noncesNumber>_<staggerSize>] with:" << std::endl;
	std::cout << "                      - address: Burst numerical address." << std::endl;
	std::cout << "                      - startNonce: First nonce of the plot generation." << std::endl;
	std::cout << "                      - noncesNumber: Number of nonces to generate (must be a multiple of <staggerSize>)." << std::endl;
	std::cout << "                      - staggerSize: Stagger size." << std::endl;
}

int CommandGenerate::execute(const std::vector<std::string>& p_args) {
	if(p_args.size() < 2) {
		help();
		return -1;
	}

	try {
		std::string writerType(p_args[0]);
		if(writerType != "buffer" && writerType != "direct") {
			throw std::runtime_error("Unknown writer type");
		}

		std::vector<unsigned long long> timeUnits {60, 60, 24, 7, 52};
		std::vector<std::string> timeLabels {"s", "m", "h", "d", "w", "y"};
		std::vector<unsigned long long> sizeUnits {1024, 1024};
		std::vector<std::string> sizeLabels {"MB", "GB", "TB"};

		std::cout << "Loading platforms..." << std::endl;
		std::vector<std::shared_ptr<OpenclPlatform>> platforms(OpenclPlatform::list());

		std::cout << "Loading devices..." << std::endl;
		std::vector<std::vector<std::shared_ptr<OpenclDevice>>> devices;
		for(const std::shared_ptr<OpenclPlatform>& platform : platforms) {
			devices.push_back(OpenclDevice::list(platform));
		}

		std::cout << "Loading devices configurations..." << std::endl;
		std::vector<std::shared_ptr<DeviceConfig>> deviceConfigs(DeviceConfig::loadFromFile(DEVICES_FILE));

		std::cout << "Initializing generation devices..." << std::endl;
		std::list<std::shared_ptr<GenerationDevice>> generationDevices;
		unsigned long long cpuMemory = 0;
		for(std::size_t i = 0, end = deviceConfigs.size() ; i < end ; ++i) {
			std::shared_ptr<DeviceConfig> config(deviceConfigs[i]);

			if(config->getPlatform() >= platforms.size()) {
				std::cout << "    [" << i << "][ERROR] No platform found with the provided id, ignoring device" << std::endl;
				continue;
			} else if(config->getDevice() >= devices[config->getPlatform()].size()) {
				std::cout << "    [" << i << "][ERROR] No device found with the provided id, ignoring device" << std::endl;
				continue;
			}

			config->normalize();

			std::shared_ptr<OpenclDevice> device(devices[config->getPlatform()][config->getDevice()]);
			std::shared_ptr<GenerationDevice> generationDevice(new GenerationDevice(config, device));
			cpuMemory +=generationDevice->getMemorySize();

			std::cout << "    [" << i << "] Device: " << device->getName() << " (" << device->getVersion() << ")" << std::endl;
			std::cout << "    [" << i << "] Device memory: " << cryo::util::formatValue(config->getBufferSize() >> 20, sizeUnits, sizeLabels) << std::endl;
			std::cout << "    [" << i << "] CPU memory: " << cryo::util::formatValue((unsigned long long)generationDevice->getMemorySize() >> 20, sizeUnits, sizeLabels) << std::endl;

			generationDevices.push_back(generationDevice);
		}

		if(generationDevices.size() == 0) {
			throw std::runtime_error("No properly configured device found");
		}

		std::cout << "Initializing generation contexts..." << std::endl;
		std::list<std::shared_ptr<GenerationContext>> generationContexts;
		unsigned long long noncesNumber = 0;
		for(std::size_t i = 0, end = p_args.size() - 1 ; i < end ; ++i) {
			std::shared_ptr<GenerationConfig> config(new GenerationConfig(p_args[i + 1]));
			config->normalize();

			std::shared_ptr<PlotsFile> plotsFile;
			std::shared_ptr<GenerationContext> generationContext;
			if(writerType == "buffer") {
				plotsFile = std::shared_ptr<PlotsFile>(new PlotsFile(config->getFullPath(), true));
				generationContext = std::shared_ptr<GenerationContext>(new GenerationContextBuffer(config, plotsFile));
			} else if(writerType == "direct") {
				unsigned int staggerSize = config->getStaggerSize();
				config = std::shared_ptr<GenerationConfig>(new GenerationConfig(config->getPath(), config->getAddress(), config->getStartNonce(), config->getNoncesNumber(), config->getNoncesNumber()));

				unsigned long long size = static_cast<unsigned long long>(config->getNoncesNumber()) * PLOT_SIZE;
				PlotsFile::preallocate(config->getFullPath(), size);

				plotsFile = std::shared_ptr<PlotsFile>(new PlotsFile(config->getFullPath(), false));
				generationContext = std::shared_ptr<GenerationContext>(new GenerationContextDirect(config, plotsFile, staggerSize));
			}

			cpuMemory += generationContext->getMemorySize();
			noncesNumber += generationContext->getConfig()->getNoncesNumber();

			std::cout << "    [" << i << "] Path: " << config->getFullPath() << std::endl;
			std::cout << "    [" << i << "] Nonces: " << config->getStartNonce() << " to " << config->getEndNonce() << " (" << cryo::util::formatValue(config->getNoncesSize() >> 20, sizeUnits, sizeLabels) << ")" << std::endl;
			std::cout << "    [" << i << "] CPU memory: " << cryo::util::formatValue((unsigned long long)generationContext->getMemorySize() >> 20, sizeUnits, sizeLabels) << std::endl;

			generationContexts.push_back(generationContext);
		}

		std::cout << "----" << std::endl;

		std::cout << "Devices number: " << generationDevices.size() << std::endl;
		std::cout << "Plots files number: " << generationContexts.size() << std::endl;
		std::cout << "Total nonces number: " << noncesNumber << std::endl;
		std::cout << "CPU memory: " << cryo::util::formatValue(cpuMemory >> 20, sizeUnits, sizeLabels) << std::endl;
		std::cout << "----" << std::endl;

		std::cout << "Generating nonces..." << std::endl;
		std::exception_ptr error;
		std::mutex mutex;
		std::condition_variable barrier;
		std::vector<std::shared_ptr<std::thread>> threads;

		{
			std::unique_lock<std::mutex> lock(mutex);

			for(std::shared_ptr<GenerationDevice>& generationDevice : generationDevices) {
				std::shared_ptr<std::thread> thread(
					new std::thread([&](std::shared_ptr<GenerationDevice> p_generationDevice) {
						try {
							computePlots(error, mutex, barrier, generationContexts, p_generationDevice);
						} catch(const std::exception& ex) {
							std::unique_lock<std::mutex> errorLock(mutex);
							error = std::current_exception();
							barrier.notify_all();
						}
					}, generationDevice),
					[](std::thread* p_thread){
						p_thread->join();
						delete p_thread;
					}
				);

				threads.push_back(thread);
			}

			for(std::shared_ptr<GenerationContext>& generationContext : generationContexts) {
				std::shared_ptr<std::thread> thread(
					new std::thread([&](std::shared_ptr<GenerationContext> p_generationContext) {
						try {
							writeNonces(error, mutex, barrier, generationContexts, p_generationContext);
						} catch(const std::exception& ex) {
							std::unique_lock<std::mutex> errorLock(mutex);
							error = std::current_exception();
							barrier.notify_all();
						}
					}, generationContext),
					[](std::thread* p_thread){
						p_thread->join();
						delete p_thread;
					}
				);

				threads.push_back(thread);
			}
		}

		std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
		std::ostringstream console;
		while(true) {
			std::unique_lock<std::mutex> lock(mutex);

			if(error) {
				std::rethrow_exception(error);
			}

			std::list<std::shared_ptr<GenerationContext>>::iterator it(std::find_if(
				generationContexts.begin(),
				generationContexts.end(),
				[](std::shared_ptr<GenerationContext>& p_generationContext) {
					return p_generationContext->getNoncesWritten() < p_generationContext->getConfig()->getNoncesNumber();
				}
			));

			if(it == generationContexts.end()) {
				break;
			}

			unsigned long long noncesWritten = 0;
			for(const std::shared_ptr<GenerationContext>& generationContext : generationContexts) {
				noncesWritten += generationContext->getNoncesWritten();
			}

			std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
			std::chrono::seconds interval = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
			double speed = (double)noncesWritten * 60.0 / std::max((double)interval.count(), 1.0);
			double percent = 100.0 * (double)noncesWritten / (double)noncesNumber;
			unsigned long long estimatedTime = (noncesNumber - noncesWritten) * 60.0 / std::max(speed, 1.0);

			std::cout << std::string(console.str().length(), '\b');
			std::cout << std::string(console.str().length(), ' ');
			std::cout << std::string(console.str().length(), '\b');
			console.str("");

			console << std::fixed << std::setprecision(2) << percent << "% (" << noncesWritten << "/" << noncesNumber << " remaining nonces)";
			console << ", " << std::fixed << std::setprecision(2) << speed << " nonces/minutes";
			console << ", ETA: " << cryo::util::formatValue(estimatedTime, timeUnits, timeLabels);
			console << "...";
			std::cout << console.str();

			barrier.wait(lock);
		}

		std::cout << std::string(console.str().length(), '\b');
		std::cout << std::string(console.str().length(), ' ');
		std::cout << std::string(console.str().length(), '\b');

		std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
		std::chrono::seconds interval = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
		double speed = (double)noncesNumber * 60.0 / std::max((double)interval.count(), 1.0);
		std::cout << "100% (" << noncesNumber << " nonces)";
		std::cout << ", " << std::fixed << std::setprecision(2) << speed << " nonces/minutes";
		std::cout << ", " << cryo::util::formatValue((unsigned long long)interval.count(), timeUnits, timeLabels) << std::endl;
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

void computePlots(
	std::exception_ptr& p_error,
	std::mutex& p_mutex,
	std::condition_variable& p_barrier,
	std::list<std::shared_ptr<GenerationContext>>& p_generationContexts,
	std::shared_ptr<GenerationDevice>& p_generationDevice
) throw (std::exception) {
	while(true) {
		std::shared_ptr<GenerationContext> generationContext;
		std::shared_ptr<GenerationWork> generationWork;

		{
			std::list<std::shared_ptr<GenerationContext>>::iterator it;
			std::unique_lock<std::mutex> lock(p_mutex);
			p_barrier.wait(lock, [&](){
				if(p_error) {
					return true;
				}

				it = std::find_if(
					p_generationContexts.begin(),
					p_generationContexts.end(),
					[](std::shared_ptr<GenerationContext>& p_generationContext) {
						return p_generationContext->getNoncesDistributed() < p_generationContext->getConfig()->getNoncesNumber();
					}
				);

				if(it == p_generationContexts.end()) {
					return true;
				}

				return p_generationDevice->isAvailable();
			});

			if(p_error || it == p_generationContexts.end()) {
				break;
			}

			it = std::min_element(
				p_generationContexts.begin(),
				p_generationContexts.end(),
				[](std::shared_ptr<GenerationContext>& p_c1, std::shared_ptr<GenerationContext>& p_c2) {
					if(p_c1->getNoncesDistributed() == p_c1->getConfig()->getNoncesNumber()) {
						return false;
					} else if(p_c2->getNoncesDistributed() == p_c2->getConfig()->getNoncesNumber()) {
						return true;
					}

					if(p_c1->getPendingNonces() == p_c2->getPendingNonces()) {
						return p_c1->getNoncesDistributed() < p_c2->getNoncesDistributed();
					}

					return p_c1->getPendingNonces() < p_c2->getPendingNonces();
				}
			);

			generationContext = *it;
			generationWork = generationContext->requestWork(p_generationDevice);
			p_generationDevice->setAvailable(false);
		}

		p_generationDevice->computePlots(
			generationContext->getConfig()->getAddress(),
			generationWork->getStartNonce(),
			generationWork->getWorkSize()
		);

		{
			std::unique_lock<std::mutex> lock(p_mutex);
			generationWork->setStatus(GenerationStatus::Generated);
			p_barrier.notify_all();
		}
	}
}

void writeNonces(
	std::exception_ptr& p_error,
	std::mutex& p_mutex,
	std::condition_variable& p_barrier,
	std::list<std::shared_ptr<GenerationContext>>& p_generationContexts,
	std::shared_ptr<GenerationContext>& p_context
) throw (std::exception) {
	while(true) {
		std::shared_ptr<GenerationWork> generationWork;

		{
			std::list<std::shared_ptr<GenerationContext>>::iterator it;
			std::unique_lock<std::mutex> lock(p_mutex);
			p_barrier.wait(lock, [&](){
				if(p_error) {
					return true;
				}

				it = std::find_if(
					p_generationContexts.begin(),
					p_generationContexts.end(),
					[](std::shared_ptr<GenerationContext>& p_generationContext) {
						return p_generationContext->getNoncesWritten() < p_generationContext->getConfig()->getNoncesNumber();
					}
				);

				if(it == p_generationContexts.end()) {
					return true;
				}

				return
					p_context->hasPendingWork() &&
					p_context->getLastPendingWork()->getStatus() == GenerationStatus::Generated;
			});

			if(p_error || it == p_generationContexts.end()) {
				break;
			}

			generationWork = p_context->getLastPendingWork();
			generationWork->setStatus(GenerationStatus::Writing);
		}

		generationWork->getDevice()->bufferPlots();

		{
			std::unique_lock<std::mutex> lock(p_mutex);
			generationWork->getDevice()->setAvailable(true);
			p_barrier.notify_all();
		}

		p_context->writeNonces(generationWork);

		{
			std::unique_lock<std::mutex> lock(p_mutex);

			generationWork->setStatus(GenerationStatus::Written);
			p_context->popLastPendingWork();
			p_barrier.notify_all();

			if(p_context->getNoncesWritten() == p_context->getConfig()->getNoncesNumber()) {
// TODO: std::cout?
				break;
			}
		}
	}
}

}}
