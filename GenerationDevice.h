/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_GENERATION_DEVICE_H
#define CRYO_GPU_PLOT_GENERATOR_GENERATION_DEVICE_H

#include <memory>
#include <string>
#include <exception>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "constants.h"
#include "DeviceConfig.h"
#include "OpenclDevice.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationDevice {
	private:
		std::shared_ptr<DeviceConfig> m_config;
		std::shared_ptr<OpenclDevice> m_device;
		unsigned char* m_bufferCpu;
		cl_context m_context;
		cl_command_queue m_commandQueue;
		cl_mem m_bufferDevice;
		cl_program m_program;
		cl_kernel m_kernels[3];
		bool m_available;

	public:
		GenerationDevice(const std::shared_ptr<DeviceConfig>& p_config, const std::shared_ptr<OpenclDevice>& p_device) throw (std::exception);
		GenerationDevice(const GenerationDevice& p_other) = delete;

		virtual ~GenerationDevice() throw ();

		GenerationDevice& operator=(const GenerationDevice& p_other) = delete;

		inline const std::shared_ptr<DeviceConfig>& getConfig() const;
		inline const std::shared_ptr<OpenclDevice>& getDevice() const;
		inline const unsigned char* getBufferCpu() const;
		inline std::size_t getMemorySize() const;
		inline bool isAvailable() const;
		inline void setAvailable(bool p_available);

		void computePlots(unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_workSize) throw (std::exception);
		void bufferPlots() throw (std::exception);

	private:
		std::string loadSource(const std::string& p_file) const throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline const std::shared_ptr<DeviceConfig>& GenerationDevice::getConfig() const {
	return m_config;
}

inline const std::shared_ptr<OpenclDevice>& GenerationDevice::getDevice() const {
	return m_device;
}

inline const unsigned char* GenerationDevice::getBufferCpu() const {
	return m_bufferCpu;
}

inline std::size_t GenerationDevice::getMemorySize() const {
	return (std::size_t)m_config->getGlobalWorkSize() * PLOT_SIZE;
}

inline bool GenerationDevice::isAvailable() const {
	return m_available;
}

inline void GenerationDevice::setAvailable(bool p_available) {
	m_available = p_available;
}

}}

#endif
