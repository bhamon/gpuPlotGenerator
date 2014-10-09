/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_GENERATION_CONTEXT_H
#define CRYO_GPU_PLOT_GENERATOR_GENERATION_CONTEXT_H

#include <memory>
#include <exception>
#include <CL/cl.h>

#include "constants.h"
#include "GenerationConfig.h"
#include "OpenclDevice.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationContext {
	private:
		std::shared_ptr<GenerationConfig> m_config;
		std::shared_ptr<OpenclDevice> m_device;
		cl_context m_context;
		cl_command_queue m_commandQueue;
		cl_mem m_buffer;
		cl_program m_program;
		cl_kernel m_kernels[3];

	public:
		GenerationContext(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<OpenclDevice>& p_device) throw (std::exception);
		GenerationContext(const GenerationContext& p_other) = delete;

		virtual ~GenerationContext() throw ();

		GenerationContext& operator=(const GenerationContext& p_other) = delete;

		inline std::shared_ptr<GenerationConfig> getConfig() const;
		inline std::shared_ptr<OpenclDevice> getDevice() const;

		void computePlots(unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_workSize) throw (std::exception);
		void readPlots(unsigned char* p_buffer, std::size_t p_offset, unsigned int p_size) throw (std::exception);

	private:
		std::string loadSource(const std::string& p_file) const throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline std::shared_ptr<GenerationConfig> GenerationContext::getConfig() const {
	return m_config;
}

inline std::shared_ptr<OpenclDevice> GenerationContext::getDevice() const {
	return m_device;
}

}}

#endif
