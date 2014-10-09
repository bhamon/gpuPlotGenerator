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
#include <tuple>
#include <future>
#include <memory>

#include "constants.h"
#include "util.h"
#include "OpenclError.h"
#include "OpenclPlatform.h"
#include "OpenclDevice.h"
#include "PlotsFile.h"
#include "GenerationConfig.h"
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
	std::cout << "Usage: ./gpuPlotGenerator generate <path> <address> <startNonce> <noncesNumber> <staggerSize>" << std::endl;
	std::cout << "    Generate plots and write them to the specified directory." << std::endl;
	std::cout << "Parameters:" << std::endl;
	std::cout << "    - path: Path to the plots directory (must exists)." << std::endl;
	std::cout << "    - address: Burst numerical address." << std::endl;
	std::cout << "    - startNonce: First nonce of the plot generation." << std::endl;
	std::cout << "    - noncesNumber: Number of nonces to generate (must be a multiple of <staggerSize>)." << std::endl;
	std::cout << "    - staggerSize: Stagger size." << std::endl;
}

int CommandGenerate::execute(const std::vector<std::string>& p_args) {
	if(p_args.size() < 5) {
		help();
		return -1;
	}

	unsigned char* bufferStagger = 0;
	unsigned char* bufferPlots = 0;
	std::vector<std::shared_ptr<std::thread>> threads;

	int returnCode = 0;

	try {
		std::string path(p_args[0]);
		unsigned long long address = std::strtoull(p_args[1].c_str(), 0, 10);
		unsigned long long startNonce = std::strtoull(p_args[2].c_str(), 0, 10);
		unsigned int noncesNumber = std::atol(p_args[3].c_str());
		unsigned int staggerSize = std::atol(p_args[4].c_str());

		std::cout << "Checking input parameters..." << std::endl;

		if(noncesNumber % staggerSize != 0) {
			std::cout << "[WARNING] <noncesNumber> must be a multiple of <staggerSize>, correcting it..." << std::endl;
			noncesNumber -= noncesNumber % staggerSize;
			noncesNumber += staggerSize;
		}

		if(sizeof(std::size_t) == 4 && staggerSize > 16000) {
			throw std::runtime_error("Stagger size value too high (32bits platform restriction)");
		}

		std::cout << "----" << std::endl;

		std::vector<unsigned int> timeUnits {60, 60, 24, 7, 52};
		std::vector<std::string> timeLabels {"s", "m", "h", "d", "w", "y"};
		std::vector<unsigned long long> sizeUnits {1024, 1024};
		std::vector<std::string> sizeLabels {"MB", "GB", "TB"};

		unsigned long long endNonce = startNonce + noncesNumber - 1;
		unsigned long long noncesMemory = ((unsigned long long)noncesNumber * PLOT_SIZE) >> 20;
		std::size_t bufferStaggerSize = (std::size_t)staggerSize * PLOT_SIZE;
		std::size_t bufferPlotsSize = PLOT_SIZE;
		unsigned long long cpuMemory = ((unsigned long long)bufferStaggerSize + bufferPlotsSize) >> 20;
		std::cout << "Path: " << path << std::endl;
		std::cout << "Nonces: " << startNonce << " to " << endNonce << " (" << cryo::util::formatValue(noncesMemory, sizeUnits, sizeLabels) << ")" << std::endl;
		std::cout << "CPU memory: " << cryo::util::formatValue(cpuMemory, sizeUnits, sizeLabels) << std::endl;
		std::cout << "----" << std::endl;

		std::cout << "Loading platforms..." << std::endl;
		std::vector<std::shared_ptr<OpenclPlatform>> platforms(OpenclPlatform::list());

		std::cout << "Loading devices..." << std::endl;
		std::vector<std::vector<std::shared_ptr<OpenclDevice>>> devices;
		for(const std::shared_ptr<OpenclPlatform>& platform : platforms) {
			devices.push_back(OpenclDevice::list(platform));
		}

		std::cout << "Loading devices configurations..." << std::endl;
		std::vector<std::shared_ptr<GenerationConfig>> configs(GenerationConfig::loadFromFile(DEVICES_FILE));

		std::cout << "Initializing generation contexts..." << std::endl;
		std::vector<std::shared_ptr<GenerationContext>> contexts;
		std::size_t i = 0;
		for(std::shared_ptr<GenerationConfig>& config : configs) {
			if(config->getPlatform() >= platforms.size()) {
				std::cout << "[ERROR][" << i << "] No platform found with the provided id, ignoring device" << std::endl;
				continue;
			} else if(config->getDevice() >= devices[config->getPlatform()].size()) {
				std::cout << "[ERROR][" << i << "] No device found with the provided id, ignoring device" << std::endl;
				continue;
			}

			config->normalize();
			contexts.push_back(std::shared_ptr<GenerationContext>(new GenerationContext(config, devices[config->getPlatform()][config->getDevice()])));
			std::cout << "[INFO][" << i << "] Used memory: " << cryo::util::formatValue(config->getBufferSize() >> 20, sizeUnits, sizeLabels) << std::endl;

			++i;
		}

		if(contexts.size() == 0) {
			throw std::runtime_error("No properly configured device found");
		}

		std::cout << "Creating CPU stagger buffer..." << std::endl;
		bufferStagger = new unsigned char[bufferStaggerSize];
		if(!bufferStagger) {
			throw std::runtime_error("Unable to create the CPU stagger buffer");
		}

		std::cout << "Creating CPU plots buffer..." << std::endl;
		bufferPlots = new unsigned char[bufferPlotsSize];
		if(!bufferPlots) {
			throw std::runtime_error("Unable to create the CPU plots buffer");
		}

		std::cout << "Opening output file..." << std::endl;
		PlotsFile out(path, address, startNonce, noncesNumber, staggerSize);

		std::cout << "Generating nonces..." << std::endl;
		unsigned int noncesDistributed = 0;
		bool generationError = false;
		std::mutex mutex;
		std::condition_variable barrier;
		typedef std::tuple<std::shared_ptr<GenerationContext>, std::shared_future<unsigned int>> PendingTask;
		std::list<PendingTask> pendingTasks;
		for(std::shared_ptr<GenerationContext>& context : contexts) {
			std::shared_ptr<std::thread> thread(new std::thread([&](std::shared_ptr<GenerationContext> p_context) {
				while(true) {
					unsigned long long nonce;
					unsigned int workSize;
					std::promise<unsigned int> promise;

					{
						std::unique_lock<std::mutex> lock(mutex);
						barrier.wait(lock, [&](){
							for(PendingTask& pendingTask : pendingTasks) {
								if(p_context == std::get<0>(pendingTask)) {
									return false;
								}
							}

							return true;
						});

						if(noncesDistributed == noncesNumber || generationError) {
							break;
						}

						nonce = startNonce + noncesDistributed;
						workSize = std::min((unsigned int)p_context->getConfig()->getGlobalWorkSize(), noncesNumber - noncesDistributed);
						noncesDistributed += workSize;

						pendingTasks.push_back(PendingTask(p_context, promise.get_future().share()));
						barrier.notify_all();
					}

					try {
						p_context->computePlots(address, nonce, workSize);
						promise.set_value(workSize);
					} catch(const std::exception& ex) {
						promise.set_exception(std::current_exception());
					}
				}
			}, context));

			threads.push_back(thread);
		}

		std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
		std::ostringstream console;
		unsigned int noncesWritten = 0;
		while(noncesWritten < noncesNumber) {
			std::cout << std::string(console.str().length(), '\b');
			console.str("");

			std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
			std::chrono::seconds interval = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
			double speed = (double)noncesWritten * 60.0 / interval.count();
			double percent = 100.0 * (double)noncesWritten / (double)noncesNumber;
			unsigned int estimatedTime = (noncesNumber - noncesWritten) * 60.0 / speed;
			console << std::fixed << std::setprecision(2) << percent << "% (" << noncesWritten << "/" << noncesNumber << " nonces)";
			console << ", " << std::fixed << std::setprecision(2) << speed << " nonces/minutes";
			console << ", ETA: " << cryo::util::formatValue(estimatedTime, timeUnits, timeLabels);
			console << "...          ";
			std::cout << console.str();

			std::list<PendingTask>::iterator it;
			{
				std::unique_lock<std::mutex> lock(mutex);
				barrier.wait(lock, [&](){ return pendingTasks.size() > 0; });

				it = pendingTasks.begin();
			}

			try {
				std::shared_ptr<GenerationContext> context(std::get<0>(*it));
				std::shared_future<unsigned int> future(std::get<1>(*it));

				unsigned int workSize = future.get();

				for(unsigned int i = 0 ; i < workSize ; ++i) {
					context->readPlots(bufferPlots, i, 1);

					unsigned int currentNonce = noncesWritten + i;
					unsigned int staggerNonce = currentNonce % staggerSize;
					for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE) {
						std::copy_n(bufferPlots + j, SCOOP_SIZE, bufferStagger + (std::size_t)staggerNonce * SCOOP_SIZE + (std::size_t)j * staggerSize);
					}

					if(staggerNonce == staggerSize - 1) {
						out.write(bufferStagger, bufferStaggerSize);
					}
				}

				noncesWritten += workSize;
			} catch(const std::exception& ex) {
				std::unique_lock<std::mutex> lock(mutex);
				generationError = true;
				pendingTasks.clear();
				barrier.notify_all();

				throw;
			}

			{
				std::unique_lock<std::mutex> lock(mutex);

				pendingTasks.erase(it);
				barrier.notify_all();
			}
		}

		std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
		std::chrono::seconds interval = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
		double speed = (double)noncesNumber * 60.0 / interval.count();
		std::cout << std::string(console.str().length(), '\b');
		std::cout << "100% (" << noncesNumber << "/" << noncesNumber << " nonces)";
		std::cout << ", " << std::fixed << std::setprecision(2) << speed << " nonces/minutes";
		std::cout << ", " << cryo::util::formatValue((unsigned int)interval.count(), timeUnits, timeLabels);
		std::cout << "                    " << std::endl;
	} catch(const OpenclError& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR][" << ex.getCode() << "][" << ex.getCodeString() << "] " << ex.what() << std::endl;
		returnCode = -1;
	} catch(const std::exception& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR] " << ex.what() << std::endl;
		returnCode = -1;
	}

	for(std::shared_ptr<std::thread>& thread : threads) {
		thread->join();
	}

	if(bufferPlots) { delete[] bufferPlots; }
	if(bufferStagger) { delete[] bufferStagger; }

	return returnCode;
}

}}
