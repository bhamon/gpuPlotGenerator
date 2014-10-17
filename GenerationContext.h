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

#include "GenerationConfig.h"
#include "PlotsFile.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationContext {
	private:
		std::shared_ptr<GenerationConfig> m_config;
		std::shared_ptr<PlotsFile> m_plotsFile;
		unsigned int m_noncesDistributed;
		unsigned int m_noncesWritten;
		bool m_available;

	public:
		GenerationContext(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<PlotsFile>& p_plotsFile);
		GenerationContext(const GenerationContext& p_other);

		virtual ~GenerationContext() throw ();

		GenerationContext& operator=(const GenerationContext& p_other);

		inline const std::shared_ptr<GenerationConfig>& getConfig() const;
		inline const std::shared_ptr<PlotsFile>& getPlotsFile() const;
		inline unsigned int getNoncesDistributed() const;
		inline unsigned int getNoncesWritten() const;
		inline bool isAvailable() const;
		inline void setAvailable(bool p_available);

		inline unsigned long long getCurrentDistributedNonce() const;
		inline unsigned long long getCurrentWrittenNonce() const;
		inline unsigned int getPendingNonces() const;

		unsigned int requestWorkSize(unsigned int p_maxSize);
		void appendWorkSize(unsigned int p_workSize);
};

}}

#include "constants.h"

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

inline bool GenerationContext::isAvailable() const {
	return m_available;
}

inline void GenerationContext::setAvailable(bool p_available) {
	m_available = p_available;
}

inline unsigned long long GenerationContext::getCurrentDistributedNonce() const {
	return m_config->getStartNonce() + m_noncesDistributed;
}

inline unsigned long long GenerationContext::getCurrentWrittenNonce() const {
	return m_config->getStartNonce() + m_noncesWritten;
}

inline unsigned int GenerationContext::getPendingNonces() const {
	return m_noncesDistributed - m_noncesWritten;
}

}}

#endif
