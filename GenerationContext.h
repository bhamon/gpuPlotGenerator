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
#include <list>

#include "GenerationConfig.h"
#include "PlotsFile.h"
#include "GenerationWork.h"
#include "GenerationDevice.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationContext {
	protected:
		std::shared_ptr<GenerationConfig> m_config;
		std::shared_ptr<PlotsFile> m_plotsFile;
		unsigned int m_noncesDistributed;
		unsigned int m_noncesWritten;
		std::list<std::shared_ptr<GenerationWork>> m_pendingWorks;

	public:
		GenerationContext(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<PlotsFile>& p_plotsFile);
		GenerationContext(const GenerationContext& p_other) = delete;

		virtual ~GenerationContext() throw ();

		GenerationContext& operator=(const GenerationContext& p_other) = delete;

		inline const std::shared_ptr<GenerationConfig>& getConfig() const;
		inline const std::shared_ptr<PlotsFile>& getPlotsFile() const;
		inline unsigned int getNoncesDistributed() const;
		inline unsigned int getNoncesWritten() const;

		inline unsigned int getPendingNonces() const;
		inline bool hasPendingWork() const;
		inline const std::shared_ptr<GenerationWork>& getLastPendingWork() const;

		const std::shared_ptr<GenerationWork>& requestWork(const std::shared_ptr<GenerationDevice>& p_device) throw (std::exception);
		void popLastPendingWork() throw (std::exception);

		virtual std::size_t getMemorySize() const = 0;
		virtual void writeNonces(std::shared_ptr<GenerationWork>& p_work) throw (std::exception) = 0;
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline const std::shared_ptr<GenerationConfig>& GenerationContext::getConfig() const {
	return m_config;
}

inline const std::shared_ptr<PlotsFile>& GenerationContext::getPlotsFile() const {
	return m_plotsFile;
}

inline unsigned int GenerationContext::getNoncesDistributed() const {
	return m_noncesDistributed;
}

inline unsigned int GenerationContext::getNoncesWritten() const {
	return m_noncesWritten;
}

inline unsigned int GenerationContext::getPendingNonces() const {
	return m_noncesDistributed - m_noncesWritten;
}

inline bool GenerationContext::hasPendingWork() const {
	return m_pendingWorks.size() > 0;
}

inline const std::shared_ptr<GenerationWork>& GenerationContext::getLastPendingWork() const {
	return m_pendingWorks.back();
}

}}

#endif
