/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>

#include "util.h"
#include "GenerationConfig.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationConfig::GenerationConfig(std::size_t p_platform, std::size_t p_device, std::size_t p_globalWorkSize, std::size_t p_localWorkSize, unsigned int p_hashesNumber) throw (std::exception)
: m_platform(p_platform), m_device(p_device), m_globalWorkSize(p_globalWorkSize), m_localWorkSize(p_localWorkSize), m_hashesNumber(p_hashesNumber) {
}

GenerationConfig::GenerationConfig(const GenerationConfig& p_other)
: m_platform(p_other.m_platform), m_device(p_other.m_device), m_globalWorkSize(p_other.m_globalWorkSize), m_localWorkSize(p_other.m_localWorkSize), m_hashesNumber(p_other.m_hashesNumber) {
}

GenerationConfig::~GenerationConfig() throw () {
}

GenerationConfig& GenerationConfig::operator=(const GenerationConfig& p_other) {
	m_platform = p_other.m_platform;
	m_device = p_other.m_device;
	m_globalWorkSize = p_other.m_globalWorkSize;
	m_localWorkSize = p_other.m_localWorkSize;
	m_hashesNumber = p_other.m_hashesNumber;

	return *this;
}

void GenerationConfig::normalize() {
	if(m_localWorkSize > m_globalWorkSize) {
		m_localWorkSize = m_globalWorkSize;
	}

	if(m_globalWorkSize % m_localWorkSize != 0) {
		m_localWorkSize = std::max((std::size_t)1, m_localWorkSize - m_globalWorkSize % m_localWorkSize);
	}

	if(m_hashesNumber == 0) {
		m_hashesNumber = 1;
	} else if(m_hashesNumber > PLOT_SIZE / HASH_SIZE) {
		m_hashesNumber = PLOT_SIZE / HASH_SIZE;
	}
}

std::vector<std::shared_ptr<GenerationConfig>> GenerationConfig::loadFromFile(const std::string& p_path) throw (std::exception) {
	std::vector<std::shared_ptr<GenerationConfig>> configs;

	std::ifstream file(p_path, std::ios::in);
	if(!file) {
		throw std::runtime_error("Unable to open config file");
	}

	char buf[256];
	for(unsigned int i = 0 ; file.getline(buf, 256) ; ++i) {
		std::vector<std::string> parts(cryo::util::split(buf, " "));
		if(parts.size() != 5) {
			std::ostringstream message;
			message << "Invalid parameters count at line [" << i << "]";
			throw std::runtime_error(message.str());
		}

		unsigned int platform = std::atol(parts[0].c_str());
		unsigned int device = std::atol(parts[1].c_str());
		std::size_t globalWorkSize = std::atol(parts[2].c_str());
		std::size_t localWorkSize = std::atol(parts[3].c_str());
		unsigned int hashesNumber = std::atol(parts[4].c_str());

		configs.push_back(std::shared_ptr<GenerationConfig>(new GenerationConfig(platform, device, globalWorkSize, localWorkSize, hashesNumber)));
	}

	return configs;
}

void GenerationConfig::storeToFile(const std::string& p_path, const std::vector<std::shared_ptr<GenerationConfig>>& p_configs) throw (std::exception) {
	std::ofstream file(p_path, std::ios::out | std::ios::trunc);
	if(!file) {
		throw std::runtime_error("Unable to open config file");
	}

	for(const std::shared_ptr<GenerationConfig>& config : p_configs) {
		file << config->getPlatform() << " ";
		file << config->getDevice() << " ";
		file << config->getGlobalWorkSize() << " ";
		file << config->getLocalWorkSize() << " ";
		file << config->getHashesNumber() << std::endl;
	}
}

}}
