/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <cstdlib>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "constants.h"
#include "util.h"
#include "GenerationConfig.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationConfig::GenerationConfig(const std::string& p_fullPath) throw (std::exception) {
	std::string fullPath(p_fullPath);
	std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

	std::vector<std::string> path(cryo::util::split(fullPath, "/"));
	std::string name(path.back());
	path.pop_back();

	std::vector<std::string> parts(cryo::util::split(name, "_"));
	if(parts.size() != 4) {
		throw std::runtime_error("Invalid file name");
	}

	m_path = cryo::util::join(path.begin(), path.end(), "/");
	if(path.size() > 0) {
		m_path += "/";
	}

	m_address = std::strtoull(parts[0].c_str(), 0, 10);
	m_startNonce = std::strtoull(parts[1].c_str(), 0, 10);
	m_noncesNumber = std::atol(parts[2].c_str());
	m_staggerSize = std::atol(parts[3].c_str());
}

GenerationConfig::GenerationConfig(const std::string& p_path, unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_noncesNumber, unsigned int p_staggerSize)
: m_path(p_path), m_address(p_address), m_startNonce(p_startNonce), m_noncesNumber(p_noncesNumber), m_staggerSize(p_staggerSize) {
	std::replace(m_path.begin(), m_path.end(), '\\', '/');
	if(m_path.length() > 0 && m_path[m_path.length() - 1] != '/') {
		m_path += "/";
	}
}

GenerationConfig::GenerationConfig(const GenerationConfig& p_other)
: m_path(p_other.m_path), m_address(p_other.m_address), m_startNonce(p_other.m_startNonce), m_noncesNumber(p_other.m_noncesNumber), m_staggerSize(p_other.m_staggerSize) {
}

GenerationConfig::~GenerationConfig() throw () {
}

GenerationConfig& GenerationConfig::operator=(const GenerationConfig& p_other) {
	m_path = p_other.m_path;
	m_address = p_other.m_address;
	m_startNonce = p_other.m_startNonce;
	m_noncesNumber = p_other.m_noncesNumber;
	m_staggerSize = p_other.m_staggerSize;

	return *this;
}

std::string GenerationConfig::getFullPath() const {
	std::ostringstream path;
	path << m_path << m_address << "_" << m_startNonce << "_" << m_noncesNumber << "_" << m_staggerSize;
	return path.str();
}

unsigned long long GenerationConfig::getNoncesSize() const {
	return (unsigned long long)m_noncesNumber * PLOT_SIZE;
}

unsigned long long GenerationConfig::getNonceStaggerOffset(unsigned long long p_nonce) const throw (std::exception) {
	if(p_nonce < m_startNonce || p_nonce >= m_startNonce + m_noncesNumber) {
		throw std::runtime_error("Nonce out of bounds");
	}

	return ((p_nonce - m_startNonce) / m_staggerSize) * m_staggerSize * PLOT_SIZE;
}

unsigned long long GenerationConfig::getNonceStaggerDecal(unsigned long long p_nonce) const throw (std::exception) {
	if(p_nonce < m_startNonce || p_nonce >= m_startNonce + m_noncesNumber) {
		throw std::runtime_error("Nonce out of bounds");
	}

	return (p_nonce - m_startNonce) % m_staggerSize * SCOOP_SIZE;
}

void GenerationConfig::normalize() throw (std::exception) {
	if(m_noncesNumber % m_staggerSize != 0) {
		m_noncesNumber -= m_noncesNumber % m_staggerSize;
		m_noncesNumber += m_staggerSize;
	}

	if(sizeof(std::size_t) == 4 && m_staggerSize > 16000) {
		throw std::runtime_error("Stagger size value too high (32bits platform restriction)");
	}
}

}}
