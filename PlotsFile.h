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
		std::fstream m_stream;

	public:
		PlotsFile(const std::string& p_path, bool p_truncate = false) throw (std::exception);
		PlotsFile(const PlotsFile& p_other) = delete;
		virtual ~PlotsFile() throw ();

		PlotsFile& operator=(const PlotsFile& p_other) = delete;

		void seek(std::streamoff p_offset);
		void read(unsigned char* p_buffer, std::streamsize p_size) throw (std::exception);
		void write(const unsigned char* p_buffer, std::streamsize p_size) throw (std::exception);
		void flush() throw (std::exception);
};

}}

#endif
