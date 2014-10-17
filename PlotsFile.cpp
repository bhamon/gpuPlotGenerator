/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <stdexcept>
#include <algorithm>

#include "constants.h"
#include "PlotsFile.h"

namespace cryo {
namespace gpuPlotGenerator {

PlotsFile::PlotsFile(const std::string& p_path, bool p_truncate) throw (std::exception) {
	std::ofstream::openmode mode(std::ios::in | std::ios::out | std::ios::binary);
	if(p_truncate) {
		mode |= std::ios::trunc;
	}

	m_stream.open(p_path, mode);
	if(!m_stream) {
		throw std::runtime_error("Unable to open plots file");
	}
}

PlotsFile::~PlotsFile() throw () {
	m_stream.close();
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
