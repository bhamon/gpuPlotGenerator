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
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <memory>

#include "constants.h"
#include "util.h"
#include "OpenclError.h"
#include "OpenclPlatform.h"
#include "OpenclDevice.h"
#include "GenerationDevice.h"
#include "GenerationConfig.h"
#include "PlotsFile.h"
#include "GenerationContext.h"
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
	std::cout << "Usage: ./gpuPlotGenerator generate <buffersNb> <plotsFiles...>" << std::endl;
	std::cout << "    Generate plots using the configured devices and write them to the specified files." << std::endl;
	std::cout << "Parameters:" << std::endl;
	std::cout << "    - buffersNb: Number of rotating buffers to use to write the output files." << std::endl;
	std::cout << "                 Specify [auto] to create as many buffers as output files." << std::endl;
//	std::cout << "                 Specify [none] to write nonces directly to files." << std::endl;
// TODO
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
		if(p_args[0] == "auto") {
			buffersNb = p_args.size() - 1;
//		} else if(p_args[0] == "none") {
// TODO
		} else {
			buffersNb = std::atol(p_args[0].c_str());
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

		std::cout << "----" << std::endl;

		unsigned long long cpuMemory = (maxBufferDeviceSize + maxBufferStaggerSize + PLOT_SIZE) * buffersNb;

		std::cout << "Devices number: " << generationDevices.size() << std::endl;
		std::cout << "Plots files number: " << generationContexts.size() << std::endl;
		std::cout << "CPU memory: " << cryo::util::formatValue(cpuMemory >> 20, sizeUnits, sizeLabels) << std::endl;
		std::cout << "----" << std::endl;

		std::cout << "Generating nonces..." << std::endl;
		std::exception_ptr error;
		std::mutex mutex;
		std::condition_variable barrier;
		std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
		std::ostringstream console;

// TODO: use an unique_lock instead
		mutex.lock();

		std::vector<std::shared_ptr<std::thread>> generationThreads;
		for(std::shared_ptr<GenerationDevice>& generationDevice : generationDevices) {
			std::shared_ptr<std::thread> thread(
				new std::thread([&](std::shared_ptr<GenerationDevice> p_generationDevice) {
					while(true) {
						std::shared_ptr<GenerationContext> generationContext;
						std::shared_ptr<GenerationWork> generationWork;

						{
							std::list<std::shared_ptr<GenerationContext>>::iterator it;
							std::unique_lock<std::mutex> lock(mutex);
							barrier.wait(lock, [&](){
								if(error) {
									return true;
								}

								it = std::find_if(
									generationContexts.begin(),
									generationContexts.end(),
									[](std::shared_ptr<GenerationContext>& p_generationContext) {
										return p_generationContext->getNoncesDistributed() < p_generationContext->getConfig()->getNoncesNumber();
									}
								);

								if(it == generationContexts.end()) {
									return true;
								}

								return p_generationDevice->isAvailable();
							});

							if(error || it == generationContexts.end()) {
								break;
							}

							it = std::min_element(
								generationContexts.begin(),
								generationContexts.end(),
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

						try {
							p_generationDevice->computePlots(
								generationContext->getConfig()->getAddress(),
								generationWork->getStartNonce(),
								generationWork->getWorkSize()
							);

							std::unique_lock<std::mutex> lock(mutex);
							generationWork->setStatus(GenerationStatus::Generated);
							barrier.notify_all();
						} catch(const std::exception& ex) {
							std::unique_lock<std::mutex> lock(mutex);
							error = std::current_exception();
							barrier.notify_all();

							break;
						}
					}
				}, generationDevice),
				[](std::thread* p_thread){
					p_thread->join();
					delete p_thread;
				}
			);

			generationThreads.push_back(thread);
		}

		std::vector<std::shared_ptr<std::thread>> writingThreads;
		for(std::size_t i = 0 ; i < buffersNb ; ++i) {
			std::shared_ptr<std::thread> thread(
				new std::thread([&]() {
					std::unique_ptr<unsigned char[]> bufferPlots(new unsigned char[PLOT_SIZE]);
					std::unique_ptr<unsigned char[]> bufferDevice(new unsigned char[maxBufferDeviceSize]);
					std::unique_ptr<unsigned char[]> bufferStagger(new unsigned char[maxBufferStaggerSize]);

					while(true) {
						std::shared_ptr<GenerationContext> generationContext;
						std::shared_ptr<GenerationWork> generationWork;

						{
							std::list<std::shared_ptr<GenerationContext>>::iterator it;
							std::unique_lock<std::mutex> lock(mutex);
							barrier.wait(lock, [&](){
								if(error) {
									return true;
								}

								it = std::find_if(
									generationContexts.begin(),
									generationContexts.end(),
									[](std::shared_ptr<GenerationContext>& p_generationContext) {
										return p_generationContext->getNoncesWritten() < p_generationContext->getConfig()->getNoncesNumber();
									}
								);

								if(it == generationContexts.end()) {
									return true;
								}

								it = std::find_if(
									generationContexts.begin(),
									generationContexts.end(),
									[](std::shared_ptr<GenerationContext>& p_generationContext) {
										return
											p_generationContext->hasPendingWork() &&
											p_generationContext->getLastPendingWork()->getStatus() == GenerationStatus::Generated;
									}
								);

								return it != generationContexts.end();
							});

							if(error || it == generationContexts.end()) {
								break;
							}

							generationContext = *it;
							generationWork = generationContext->getLastPendingWork();
							generationWork->setStatus(GenerationStatus::Writing);
						}

						try {
							std::size_t bufferDeviceOffset = 0;
							for(unsigned int i = 0, end = generationWork->getWorkSize() ; i < end ; ++i, bufferDeviceOffset += PLOT_SIZE) {
								generationWork->getDevice()->readPlots(bufferPlots.get(), i, 1);
								std::copy(bufferPlots.get(), bufferPlots.get() + PLOT_SIZE, bufferDevice.get() + bufferDeviceOffset);
							}

							{
								std::unique_lock<std::mutex> lock(mutex);
								generationWork->getDevice()->setAvailable(true);
								barrier.notify_all();
							}

							unsigned int staggerSize = generationContext->getConfig()->getStaggerSize();
							bufferDeviceOffset = 0;
							for(unsigned int i = 0, end = generationWork->getWorkSize() ; i < end ; ++i, bufferDeviceOffset += PLOT_SIZE) {
								unsigned int staggerNonce = (generationContext->getNoncesWritten() + i) % staggerSize;
								for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE) {
									std::copy_n(bufferDevice.get() + bufferDeviceOffset + j, SCOOP_SIZE, bufferStagger.get() + (std::size_t)staggerNonce * SCOOP_SIZE + (std::size_t)j * staggerSize);
								}

								if(staggerNonce == staggerSize - 1) {
									generationContext->getPlotsFile()->write(bufferStagger.get(), (std::streamsize)PLOT_SIZE * staggerSize);
									generationContext->getPlotsFile()->flush();
								}
							}

							{
								std::unique_lock<std::mutex> lock(mutex);

								if(generationContext->getNoncesWritten() == generationContext->getConfig()->getNoncesNumber()) {
// TODO: std::cout?
								}

								generationWork->setStatus(GenerationStatus::Written);
								generationContext->popLastPendingWork();
								barrier.notify_all();
							}
						} catch(const std::exception& ex) {
							std::unique_lock<std::mutex> lock(mutex);
							error = std::current_exception();
							barrier.notify_all();

							break;
						}
					}
				}),
				[](std::thread* p_thread){
					p_thread->join();
					delete p_thread;
				}
			);

			writingThreads.push_back(thread);
		}

// TODO: use unique_lock instead
		mutex.unlock();

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
			unsigned long long noncesNumber = 0;
			for(const std::shared_ptr<GenerationContext>& generationContext : generationContexts) {
				noncesWritten += generationContext->getNoncesWritten();
				noncesNumber += generationContext->getConfig()->getNoncesNumber();
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
		std::cout << "100% " << cryo::util::formatValue((unsigned long long)interval.count(), timeUnits, timeLabels) << std::endl;
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
