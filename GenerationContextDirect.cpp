/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <algorithm>

#include "constants.h"
#include "GenerationContextDirect.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationContextDirect::GenerationContextDirect(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<PlotsFile>& p_plotsFile)
: GenerationContext(p_config, p_plotsFile) {
}

GenerationContextDirect::~GenerationContextDirect() throw () {
}

std::size_t GenerationContextDirect::getMemorySize() const {
	return 0;
}

void GenerationContextDirect::writeNonces(std::shared_ptr<GenerationWork>& p_work) throw (std::exception) {
	unsigned long long startNonce = m_config->getStartNonce() + m_noncesWritten;
	std::size_t bufferOffset = 0;
	for(unsigned int i = 0, end = p_work->getWorkSize() ; i < end ; ++i) {
		m_plotsFile->seek((std::streamoff)m_config->getNonceStaggerOffset(startNonce + i) + m_config->getNonceStaggerDecal(startNonce + i), std::ios::beg);

		for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE, bufferOffset += SCOOP_SIZE) {
			m_plotsFile->write(p_work->getDevice()->getBufferCpu() + bufferOffset, SCOOP_SIZE);
			m_plotsFile->seek(((std::streamoff)m_config->getStaggerSize() - 1) * SCOOP_SIZE, std::ios::cur);
		}

	}

	m_plotsFile->flush();
}

}}
