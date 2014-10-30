/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_GENERATION_CONTEXT_BUFFER_H
#define CRYO_GPU_PLOT_GENERATOR_GENERATION_CONTEXT_BUFFER_H

#include <memory>
#include <exception>

#include "GenerationContext.h"
#include "GenerationConfig.h"
#include "PlotsFile.h"
#include "GenerationWork.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationContextBuffer : public GenerationContext {
	private:
		unsigned char* m_buffer;

	public:
		GenerationContextBuffer(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<PlotsFile>& p_plotsFile);
		GenerationContextBuffer(const GenerationContextBuffer& p_other) = delete;

		virtual ~GenerationContextBuffer() throw ();

		GenerationContextBuffer& operator=(const GenerationContextBuffer& p_other) = delete;

		virtual std::size_t getMemorySize() const;
		virtual void writeNonces(std::shared_ptr<GenerationWork>& p_work) throw (std::exception);
};

}}

#endif
