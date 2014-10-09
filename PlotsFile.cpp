/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#include "constants.h"
#include "util.h"
#include "PlotsFile.h"

namespace cryo {
namespace gpuPlotGenerator {

PlotsFile::PlotsFile(const std::string& p_path) throw (std::exception) {
	std::string name(p_path);
	std::size_t slash;

	if((slash = p_path.rfind('/')) != std::string::npos) {
		name = name.substr(slash + 1);
	}

	if((slash = p_path.rfind('\\')) != std::string::npos) {
		name = name.substr(slash + 1);
	}

	std::vector<std::string> parts(cryo::util::split(name, "_"));
	if(parts.size() != 4) {
		throw std::runtime_error("Invalid file name");
	}

	m_address = std::strtoull(parts[0].c_str(), 0, 10);
	m_startNonce = std::strtoull(parts[1].c_str(), 0, 10);
	m_noncesNumber = std::atol(parts[2].c_str());
	m_staggerSize = std::atol(parts[3].c_str());

	m_stream.open(p_path, std::ios::in | std::ios::out | std::ios::binary);
	if(!m_stream) {
		throw std::runtime_error("Unable to open plots file");
	}
}

PlotsFile::PlotsFile(const std::string& p_path, unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_noncesNumber, unsigned int p_staggerSize) throw (std::exception)
: m_address(p_address), m_startNonce(p_startNonce), m_noncesNumber(p_noncesNumber), m_staggerSize(p_staggerSize) {
	std::ostringstream file;
	file << p_path << "/" << m_address << "_" << m_startNonce << "_" << m_noncesNumber << "_" << m_staggerSize;

	m_stream.open(file.str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
	if(!m_stream) {
		throw std::runtime_error("Unable to open plots file");
	}
}

PlotsFile::~PlotsFile() throw () {
	m_stream.close();
}

unsigned long long PlotsFile::getNonceStaggerOffset(unsigned long long p_nonce) const throw (std::exception) {
	if(p_nonce < m_startNonce || p_nonce >= m_startNonce + m_noncesNumber) {
		throw std::runtime_error("Nonce out of bounds");
	}

	return ((p_nonce - m_startNonce) / m_staggerSize) * m_staggerSize * PLOT_SIZE;
}

unsigned long long PlotsFile::getNonceStaggerDecal(unsigned long long p_nonce) const throw (std::exception) {
	if(p_nonce < m_startNonce || p_nonce >= m_startNonce + m_noncesNumber) {
		throw std::runtime_error("Nonce out of bounds");
	}

	return (p_nonce - m_startNonce) % m_staggerSize * SCOOP_SIZE;
}

void PlotsFile::seek(std::streamoff p_offset) {
	std::streamoff current = m_stream.tellg();

	for(std::streamoff i = current ; i < p_offset ; i += IO_CAP) {
		m_stream.seekg(std::min(p_offset - i, (std::streamoff)IO_CAP), std::ios::cur);
	}

	for(std::streamoff i = p_offset ; i < current ; i += IO_CAP) {
		m_stream.seekg(-std::min(current - i, (std::streamoff)IO_CAP), std::ios::cur);
	}
}

void PlotsFile::read(unsigned char* p_buffer, std::streamsize p_size) throw (std::exception) {
	for(std::streamsize offset = 0 ; offset < p_size ; offset += (std::streamsize)IO_CAP) {
		std::streamsize size = std::min(p_size - offset, (std::streamsize)IO_CAP);
		m_stream.read((char*)p_buffer + offset, size);
		if(!m_stream) {
			throw std::runtime_error("Error while reading from plots file");
		}
	}
}

void PlotsFile::write(const unsigned char* p_buffer, std::streamsize p_size) throw (std::exception) {
	for(std::streamsize offset = 0 ; offset < p_size ; offset += (std::streamsize)IO_CAP) {
		std::streamsize size = std::min(p_size - offset, (std::streamsize)IO_CAP);
		m_stream.write((const char*)p_buffer + offset, size);
		if(!m_stream) {
			throw std::runtime_error("Error while writting to plots file");
		}
	}
}

}}
