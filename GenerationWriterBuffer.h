/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_GENERATION_WRITER_BUFFER_H
#define CRYO_GPU_PLOT_GENERATOR_GENERATION_WRITER_BUFFER_H

#include <exception>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "GenerationWriter.h"
#include "GenerationContext.h"
#include "GenerationWork.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationWriterBuffer : public GenerationWriter {
	private:
		std::size_t m_bufferDeviceSize;
		std::size_t m_bufferStaggerSize;
		unsigned char* m_bufferDevice;
		unsigned char* m_bufferStagger;

	public:
		GenerationWriterBuffer(std::size_t p_bufferDeviceSize, std::size_t p_bufferStaggerSize) throw (std::exception);
		GenerationWriterBuffer(const GenerationWriterBuffer& p_other) = delete;
		virtual ~GenerationWriterBuffer() throw ();

		GenerationWriterBuffer& operator=(const GenerationWriterBuffer& p_other) = delete;

		virtual unsigned long long getMemorySize() const;
		virtual void readPlots(std::shared_ptr<GenerationContext>& p_context, std::shared_ptr<GenerationWork>& p_work) throw (std::exception);
		virtual void writeNonces(std::shared_ptr<GenerationContext>& p_context, std::shared_ptr<GenerationWork>& p_work) throw (std::exception);
};

}}

#endif
