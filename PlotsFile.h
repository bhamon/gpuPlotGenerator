/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_PLOTS_FILE_H
#define CRYO_GPU_PLOT_GENERATOR_PLOTS_FILE_H

#include <string>
#include <fstream>
#include <exception>

namespace cryo {
namespace gpuPlotGenerator {

class PlotsFile {
	private:
		unsigned long long m_address;
		unsigned long long m_startNonce;
		unsigned int m_noncesNumber;
		unsigned int m_staggerSize;
		std::fstream m_stream;

	public:
		PlotsFile(const std::string& p_path) throw (std::exception);
		PlotsFile(const std::string& p_path, unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_noncesNumber, unsigned int p_staggerSize) throw (std::exception);
		PlotsFile(const PlotsFile& p_other) = delete;
		virtual ~PlotsFile() throw ();

		PlotsFile& operator=(const PlotsFile& p_other) = delete;

		inline unsigned long long getAddress() const;
		inline unsigned long long getStartNonce() const;
		inline unsigned long long getEndNonce() const;
		inline unsigned int getNoncesNumber() const;
		inline unsigned int getStaggerSize() const;

		unsigned long long getNonceStaggerOffset(unsigned long long p_nonce) const throw (std::exception);
		unsigned long long getNonceStaggerDecal(unsigned long long p_nonce) const throw (std::exception);

		void seek(std::streamoff p_offset);
		void read(unsigned char* p_buffer, std::streamsize p_size) throw (std::exception);
		void write(const unsigned char* p_buffer, std::streamsize p_size) throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline unsigned long long PlotsFile::getAddress() const {
	return m_address;
}

inline unsigned long long PlotsFile::getStartNonce() const {
	return m_startNonce;
}

inline unsigned long long PlotsFile::getEndNonce() const {
	return m_startNonce + m_noncesNumber - 1;
}

inline unsigned int PlotsFile::getNoncesNumber() const {
	return m_noncesNumber;
}

inline unsigned int PlotsFile::getStaggerSize() const {
	return m_staggerSize;
}

}}

#endif
