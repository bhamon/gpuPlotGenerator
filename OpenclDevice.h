/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_OPENCL_DEVICE_H
#define CRYO_GPU_PLOT_GENERATOR_OPENCL_DEVICE_H

#include <exception>
#include <string>
#include <CL/cl.h>

namespace cryo {
namespace gpuPlotGenerator {

class OpenclDevice {
	private:
		std::size_t m_globalWorkSize;
		std::size_t m_localWorkSize;
		unsigned int m_hashesNumber;
		cl_context m_context;
		cl_command_queue m_commandQueue;
		cl_mem m_buffer;
		cl_program m_program;
		cl_kernel m_kernels[3];

	public:
		OpenclDevice(cl_device_id& p_device, std::size_t p_globalWorkSize, std::size_t p_localWorkSize, unsigned int p_hashesNumber) throw (std::exception);
		OpenclDevice(const OpenclDevice& p_other) = delete;

		virtual ~OpenclDevice() throw ();

		OpenclDevice& operator=(const OpenclDevice& p_other) = delete;

		inline std::size_t getGlobalWorkSize() const;
		inline std::size_t getLocalWorkSize() const;
		inline unsigned int getHashesNumber() const;
		inline unsigned long long getBufferSize() const;

		void computePlots(unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_workSize) throw (std::exception);
		void readPlots(unsigned char* p_buffer, std::size_t p_offset, unsigned int p_size) throw (std::exception);

	private:
		std::string loadSource(const std::string& p_file) const throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline std::size_t OpenclDevice::getGlobalWorkSize() const {
	return m_globalWorkSize;
}

inline std::size_t OpenclDevice::getLocalWorkSize() const {
	return m_localWorkSize;
}

inline unsigned int OpenclDevice::getHashesNumber() const {
	return m_hashesNumber;
}

inline unsigned long long OpenclDevice::getBufferSize() const {
	return m_globalWorkSize * GEN_SIZE;
}

}}

#endif
