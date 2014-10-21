/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_GENERATION_WRITER_H
#define CRYO_GPU_PLOT_GENERATOR_GENERATION_WRITER_H

#include <exception>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "GenerationContext.h"
#include "GenerationWork.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationWriter {
	public:
		GenerationWriter() throw (std::exception);
		GenerationWriter(const GenerationWriter& p_other) = delete;
		virtual ~GenerationWriter() throw ();

		GenerationWriter& operator=(const GenerationWriter& p_other) = delete;

		virtual unsigned long long getMemorySize() const = 0;
		virtual void readPlots(std::shared_ptr<GenerationContext>& p_context, std::shared_ptr<GenerationWork>& p_work) throw (std::exception) = 0;
		virtual void writeNonces(std::shared_ptr<GenerationContext>& p_context, std::shared_ptr<GenerationWork>& p_work) throw (std::exception) = 0;
};

}}

#endif
