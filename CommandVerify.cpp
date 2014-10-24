/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <memory>

#include "constants.h"
#include "util.h"
#include "GenerationConfig.h"
#include "PlotsFile.h"
#include "CommandVerify.h"

namespace cryo {
namespace gpuPlotGenerator {

CommandVerify::CommandVerify()
: Command("Verify a generated plots file against a reference.") {
}

CommandVerify::CommandVerify(const CommandVerify& p_command)
: Command(p_command) {
}

CommandVerify::~CommandVerify() throw () {
}

void CommandVerify::help() const {
	std::cout << "Usage: ./gpuPlotGenerator verify <generated> <reference>" << std::endl;
	std::cout << "    Verify a generated plots file against a reference file." << std::endl;
	std::cout << "    The two files must share the same address and have, at least, one nonce in common." << std::endl;
	std::cout << "    The stagger size values can be different." << std::endl;
	std::cout << "Parameters:" << std::endl;
	std::cout << "    - generated: The generated plots file to verify." << std::endl;
	std::cout << "    - reference: The reference plots file." << std::endl;
}

int CommandVerify::execute(const std::vector<std::string>& p_args) {
	if(p_args.size() < 2) {
		help();
		return -1;
	}

	try {
		GenerationConfig generatedConfig(p_args[0]);
		GenerationConfig referenceConfig(p_args[1]);

		std::cout << "Checking input parameters..." << std::endl;

		if(generatedConfig.getAddress() != referenceConfig.getAddress()) {
			throw std::runtime_error("Files don't share the same address");
		}

		if(sizeof(std::size_t) == 4 && (generatedConfig.getStaggerSize() > 16000 || referenceConfig.getStaggerSize() > 16000)) {
			throw std::runtime_error("Stagger size value too high (32bits platform restriction)");
		}

		PlotsFile generated(generatedConfig.getFullPath(), false);
		PlotsFile reference(referenceConfig.getFullPath(), false);

		unsigned long long commonStart = std::max(generatedConfig.getStartNonce(), referenceConfig.getStartNonce());
		unsigned long long commonEnd = std::min(generatedConfig.getEndNonce(), referenceConfig.getEndNonce());
		unsigned int commonNb = commonEnd - commonStart + 1;
		if(commonStart > commonEnd) {
			throw std::runtime_error("No common nonces between the two files");
		}

		std::cout << "----" << std::endl;

		std::vector<unsigned long long> sizeUnitsKB {1024, 1024, 1024};
		std::vector<std::string> sizeLabelsKB {"KB", "MB", "GB", "TB"};

		std::vector<unsigned long long> sizeUnitsB {1024, 1024, 1024, 1024};
		std::vector<std::string> sizeLabelsB {"B", "KB", "MB", "GB", "TB"};

		unsigned long long noncesMemory = ((unsigned long long)(commonEnd - commonStart + 1) * PLOT_SIZE) >> 10;
		std::size_t generatedBufferSize = SCOOP_SIZE;
		std::size_t referenceBufferSize = SCOOP_SIZE;
		unsigned long long cpuMemory = (unsigned long long)generatedBufferSize + referenceBufferSize;
		std::cout << "Common nonces: " << commonStart << " to " << commonEnd << " (" << cryo::util::formatValue(noncesMemory, sizeUnitsKB, sizeLabelsKB) << ")" << std::endl;
		std::cout << "CPU memory: " << cryo::util::formatValue(cpuMemory, sizeUnitsB, sizeLabelsB) << std::endl;
		std::cout << "----" << std::endl;

		std::unique_ptr<unsigned char[]> generatedBuffer(new unsigned char[generatedBufferSize]);
		std::unique_ptr<unsigned char[]> referenceBuffer(new unsigned char[referenceBufferSize]);

		std::cout << "Checking generated plots file..." << std::endl;

		std::ostringstream console;
		console << std::fixed << std::setprecision(2);

		for(unsigned int i = 0 ; i < commonNb ; ++i) {
			std::cout << std::string(console.str().length(), '\b');
			std::cout << std::string(console.str().length(), ' ');
			std::cout << std::string(console.str().length(), '\b');
			console.str("");

			double percent = 100.0 * (double)i / (double)commonNb;
			console << percent << "% (" << i << "/" << commonNb << " nonces)";
			console << "...";
			std::cout << console.str();

			generated.seek((std::streamoff)generatedConfig.getNonceStaggerOffset(commonStart + i) + generatedConfig.getNonceStaggerDecal(commonStart + i), std::ios::beg);
			reference.seek((std::streamoff)referenceConfig.getNonceStaggerOffset(commonStart + i) + referenceConfig.getNonceStaggerDecal(commonStart + i), std::ios::beg);

			for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE) {
				generated.read(generatedBuffer.get(), SCOOP_SIZE);
				reference.read(referenceBuffer.get(), SCOOP_SIZE);

				if(!std::equal(generatedBuffer.get(), generatedBuffer.get() + SCOOP_SIZE, referenceBuffer.get())) {
					throw std::runtime_error("Common nonces doesn't match");
				}

				generated.seek(((std::streamoff)generatedConfig.getStaggerSize() - 1) * SCOOP_SIZE, std::ios::cur);
				reference.seek(((std::streamoff)referenceConfig.getStaggerSize() - 1) * SCOOP_SIZE, std::ios::cur);
			}
		}

		std::cout << std::string(console.str().length(), '\b');
		std::cout << std::string(console.str().length(), ' ');
		std::cout << std::string(console.str().length(), '\b');

		std::cout << "100% (" << commonNb << "/" << commonNb << " nonces)" << std::endl << std::endl;
		std::cout << "[OK] The generated plots file has been successfully verified against the provided reference" << std::endl;
	} catch(const std::exception& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR] " << ex.what() << std::endl;
		return -1;
	}

	return 0;
}

}}
