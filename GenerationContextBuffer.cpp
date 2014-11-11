/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <algorithm>

#include "constants.h"
#include "GenerationContextBuffer.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationContextBuffer::GenerationContextBuffer(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<PlotsFile>& p_plotsFile)
: GenerationContext(p_config, p_plotsFile) {
	m_buffer = new unsigned char[getMemorySize()];
}

GenerationContextBuffer::~GenerationContextBuffer() throw () {
	delete[] m_buffer;
}

std::size_t GenerationContextBuffer::getMemorySize() const {
	return (std::size_t)m_config->getStaggerSize() * PLOT_SIZE;
}

void GenerationContextBuffer::writeNonces(std::shared_ptr<GenerationWork>& p_work) throw (std::exception) {
	unsigned int staggerSize = m_config->getStaggerSize();
	std::size_t bufferOffset = 0;
	for(unsigned int i = 0, end = p_work->getWorkSize() ; i < end ; ++i, bufferOffset += PLOT_SIZE) {
		unsigned int staggerNonce = (m_noncesWritten + i) % staggerSize;
		for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE) {
// TODO, replace this loop by the one in the "direct" writing strategy
			std::copy_n(p_work->getDevice()->getBufferCpu() + bufferOffset + j, SCOOP_SIZE, m_buffer + (std::size_t)staggerNonce * SCOOP_SIZE + (std::size_t)j * staggerSize);
		}

		if(staggerNonce == staggerSize - 1) {
			m_plotsFile->write(m_buffer, (std::streamsize)PLOT_SIZE * staggerSize);
			m_plotsFile->flush();
		}
	}
}

}}
