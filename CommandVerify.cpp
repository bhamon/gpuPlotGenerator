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

#include "constants.h"
#include "util.h"
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

	unsigned char* generatedBuffer = 0;
	unsigned char* referenceBuffer = 0;

	int returnCode = 0;

	try {
		PlotsFile generated(p_args[0]);
		PlotsFile reference(p_args[1]);

		std::cout << "Checking input parameters..." << std::endl;

		if(generated.getAddress() != reference.getAddress()) {
			throw std::runtime_error("Files don't share the same address");
		}

		if(sizeof(std::size_t) == 4 && (generated.getStaggerSize() > 16000 || reference.getStaggerSize() > 16000)) {
			throw std::runtime_error("Stagger size value too high (32bits platform restriction)");
		}

		unsigned long long commonStart = std::max(generated.getStartNonce(), reference.getStartNonce());
		unsigned long long commonEnd = std::min(generated.getEndNonce(), reference.getEndNonce());
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

		generatedBuffer = new unsigned char[generatedBufferSize];
		if(!generatedBuffer) {
			throw std::runtime_error("Unable to create the generated file buffer");
		}

		referenceBuffer = new unsigned char[referenceBufferSize];
		if(!referenceBuffer) {
			throw std::runtime_error("Unable to create the reference file buffer");
		}

		std::cout << "Checking generated plots file..." << std::endl;

		std::ostringstream console;
		console << std::fixed << std::setprecision(2);

		for(unsigned int i = 0 ; i < commonNb ; ++i) {
			std::cout << std::string(console.str().length(), '\b');
			console.str("");

			double percent = 100.0 * (double)i / (double)commonNb;
			console << percent << "% (" << i << "/" << commonNb << " nonces)";
			console << "...          ";
			std::cout << console.str();

			std::streamoff generatedNonceStaggerOffset = generated.getNonceStaggerOffset(commonStart + i);
			std::streamoff generatedNonceStaggerDecal = generated.getNonceStaggerDecal(commonStart + i);

			std::streamoff referenceNonceStaggerOffset = reference.getNonceStaggerOffset(commonStart + i);
			std::streamoff referenceNonceStaggerDecal = reference.getNonceStaggerDecal(commonStart + i);

			for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE) {
				generated.seek(generatedNonceStaggerOffset + generatedNonceStaggerDecal + (std::streamoff)j * generated.getStaggerSize());
				reference.seek(referenceNonceStaggerOffset + referenceNonceStaggerDecal + (std::streamoff)j * reference.getStaggerSize());

				generated.read(generatedBuffer, SCOOP_SIZE);
				reference.read(referenceBuffer, SCOOP_SIZE);

				if(!std::equal(generatedBuffer, generatedBuffer + SCOOP_SIZE, referenceBuffer)) {
					throw std::runtime_error("Common nonces doesn't match");
				}
			}
		}

		std::cout << std::string(console.str().length(), '\b');
		std::cout << "100% (" << commonNb << "/" << commonNb << " nonces)";
		std::cout << "                    " << std::endl << std::endl;
		std::cout << "[OK] The generated plots file has been successfully verified against the provided reference" << std::endl;
	} catch(const std::exception& ex) {
		std::cout << std::endl;
		std::cout << "[ERROR] " << ex.what() << std::endl;
		returnCode = -1;
	}

	if(referenceBuffer) { delete[] referenceBuffer; }
	if(generatedBuffer) { delete[] generatedBuffer; }

	return returnCode;
}

}}
