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
#include "GenerationWork.h"
#include "CommandGenerate.h"
#include "GenerationWriterBuffer.h"
#include "GenerationWriterDirect.h"

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
	std::cout << "Usage: ./gpuPlotGenerator generate <buffersNb> <plotsFiles...>" << std::endl;
	std::cout << "    Generate plots using the configured devices and write them to the specified files." << std::endl;
	std::cout << "Parameters:" << std::endl;
	std::cout << "    - buffersNb: Number of rotating buffers to use to write the output files." << std::endl;
	std::cout << "                 Specify [auto] to create as many buffers as output files." << std::endl;
	std::cout << "                 Specify [direct] to write nonces directly to files." << std::endl;
	std::cout << "    - plotsFiles: A space-sparated list of output files to generate." << std::endl;
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
		std::size_t buffersNb;
		bool direct = false;
		if(p_args[0] == "auto") {
			buffersNb = p_args.size() - 1;
		} else if(p_args[0] == "direct") {
			buffersNb = p_args.size() - 1;
			direct = true;
		} else {
			buffersNb = std::atol(p_args[0].c_str());
		}

		if(buffersNb == 0 || buffersNb > p_args.size() - 1) {
			throw std::runtime_error("Parameter <buffersNb> must be between 1 and the number of output plots files");
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
		unsigned long long maxBufferDeviceSize = 0;
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

			unsigned long long bufferSize = config->getBufferSize();
			maxBufferDeviceSize = std::max(maxBufferDeviceSize, bufferSize);

			std::shared_ptr<OpenclDevice> device(devices[config->getPlatform()][config->getDevice()]);
			std::cout << "    [" << i << "] Device: " << device->getName() << " (" << device->getVersion() << ")" << std::endl;
			std::cout << "    [" << i << "] Used memory: " << cryo::util::formatValue(bufferSize >> 20, sizeUnits, sizeLabels) << std::endl;

			generationDevices.push_back(std::shared_ptr<GenerationDevice>(new GenerationDevice(config, device)));
		}

		if(generationDevices.size() == 0) {
			throw std::runtime_error("No properly configured device found");
		}

		std::cout << "Initializing generation contexts..." << std::endl;
		std::list<std::shared_ptr<GenerationContext>> generationContexts;
		unsigned long long maxBufferStaggerSize = 0;
		for(std::size_t i = 1, end = p_args.size() ; i < end ; ++i) {
			std::shared_ptr<GenerationConfig> config(new GenerationConfig(p_args[i]));
			config->normalize();

			std::shared_ptr<PlotsFile> plotsFile(new PlotsFile(config->getFullPath(), true));

			maxBufferStaggerSize = std::max(maxBufferStaggerSize, (unsigned long long)config->getStaggerSize() * PLOT_SIZE);

			std::cout << "    [" << (i - 1) << "] Path: " << config->getFullPath() << std::endl;
			std::cout << "    [" << (i - 1) << "] Nonces: " << config->getStartNonce() << " to " << config->getEndNonce() << " (" << cryo::util::formatValue(config->getNoncesSize() >> 20, sizeUnits, sizeLabels) << ")" << std::endl;

			generationContexts.push_back(std::shared_ptr<GenerationContext>(new GenerationContext(config, plotsFile)));
		}

		std::cout << "Initializing generation writers..." << std::endl;
		std::list<std::shared_ptr<GenerationWriter>> generationWriters;
		for(std::size_t i = 0 ; i < buffersNb ; ++i) {
			if(direct) {
				generationWriters.push_back(std::shared_ptr<GenerationWriter>(new GenerationWriterDirect(maxBufferDeviceSize)));
			} else {
				generationWriters.push_back(std::shared_ptr<GenerationWriter>(new GenerationWriterBuffer(maxBufferDeviceSize, maxBufferStaggerSize)));
			}
		}

		std::cout << "----" << std::endl;

		unsigned long long cpuMemory = 0;
		for(const std::shared_ptr<GenerationWriter>& generationWriter : generationWriters) {
			cpuMemory += generationWriter->getMemorySize();
		}

		unsigned long long noncesNumber = 0;
		for(const std::shared_ptr<GenerationContext>& generationContext : generationContexts) {
			noncesNumber += generationContext->getConfig()->getNoncesNumber();
		}

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

			for(std::shared_ptr<GenerationWriter>& generationWriter : generationWriters) {
				std::shared_ptr<std::thread> thread(
					new std::thread([&](std::shared_ptr<GenerationWriter> p_generationWriter) {
						try {
							writeNonces(error, mutex, barrier, generationContexts, p_generationWriter);
						} catch(const std::exception& ex) {
							std::unique_lock<std::mutex> errorLock(mutex);
							error = std::current_exception();
							barrier.notify_all();
						}
					}, generationWriter),
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
					if(p_c1->getConfig()->getNoncesNumber() == p_c1->getNoncesWritten()) {
						return false;
					} else if(p_c1->getConfig()->getNoncesNumber() == p_c1->getNoncesWritten()) {
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
	std::shared_ptr<GenerationWriter>& p_writer
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
						return p_generationContext->getNoncesWritten() < p_generationContext->getConfig()->getNoncesNumber();
					}
				);

				if(it == p_generationContexts.end()) {
					return true;
				}

				it = std::find_if(
					p_generationContexts.begin(),
					p_generationContexts.end(),
					[](std::shared_ptr<GenerationContext>& p_generationContext) {
						return
							p_generationContext->hasPendingWork() &&
							p_generationContext->getLastPendingWork()->getStatus() == GenerationStatus::Generated;
					}
				);

				return it != p_generationContexts.end();
			});

			if(p_error || it == p_generationContexts.end()) {
				break;
			}

			generationContext = *it;
			generationWork = generationContext->getLastPendingWork();
			generationWork->setStatus(GenerationStatus::Writing);
		}

		p_writer->readPlots(generationContext, generationWork);

		{
			std::unique_lock<std::mutex> lock(p_mutex);
			generationWork->getDevice()->setAvailable(true);
			p_barrier.notify_all();
		}

		p_writer->writeNonces(generationContext, generationWork);

		{
			std::unique_lock<std::mutex> lock(p_mutex);

			generationWork->setStatus(GenerationStatus::Written);
			generationContext->popLastPendingWork();
			p_barrier.notify_all();

			if(generationContext->getNoncesWritten() == generationContext->getConfig()->getNoncesNumber()) {
// TODO: std::cout?
			}
		}
	}
}

}}
