/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_GENERATION_WORK_H
#define CRYO_GPU_PLOT_GENERATOR_GENERATION_WORK_H

#include <memory>

#include "GenerationDevice.h"

namespace cryo {
namespace gpuPlotGenerator {

enum GenerationStatus {
	Generating,
	Generated,
	Writing,
	Written
};

class GenerationWork {
	private:
		std::shared_ptr<GenerationDevice> m_device;
		unsigned long long m_startNonce;
		unsigned int m_workSize;
		GenerationStatus m_status;

	public:
		GenerationWork(const std::shared_ptr<GenerationDevice>& p_device, unsigned long long p_startNonce, unsigned int p_workSize);
		GenerationWork(const GenerationWork& p_other);
		virtual ~GenerationWork() throw ();

		GenerationWork& operator=(const GenerationWork& p_other);

		inline const std::shared_ptr<GenerationDevice>& getDevice() const;
		inline unsigned long long getStartNonce() const;
		inline unsigned int getWorkSize() const;
		inline GenerationStatus getStatus() const;
		inline void setStatus(GenerationStatus p_status);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline const std::shared_ptr<GenerationDevice>& GenerationWork::getDevice() const {
	return m_device;
}

inline unsigned long long GenerationWork::getStartNonce() const {
	return m_startNonce;
}

inline unsigned int GenerationWork::getWorkSize() const {
	return m_workSize;
}

inline GenerationStatus GenerationWork::getStatus() const {
	return m_status;
}

inline void GenerationWork::setStatus(GenerationStatus p_status) {
	m_status = p_status;
}

}}

#endif
