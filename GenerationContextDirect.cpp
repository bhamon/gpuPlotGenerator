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

GenerationContextDirect::GenerationContextDirect(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<PlotsFile>& p_plotsFile, unsigned int p_staggerSize)
: GenerationContext(p_config, p_plotsFile), m_staggerSize(p_staggerSize) {
	m_buffer = new unsigned char[getMemorySize()];
}

GenerationContextDirect::~GenerationContextDirect() throw () {
	delete[] m_buffer;
}

std::size_t GenerationContextDirect::getMemorySize() const {
	return (std::size_t)m_staggerSize * PLOT_SIZE;
}

void GenerationContextDirect::writeNonces(std::shared_ptr<GenerationWork>& p_work) throw (std::exception) {
	std::size_t deviceOffset = 0;
	for(unsigned int i = 0, end = p_work->getWorkSize() ; i < end ; ++i) {
		unsigned int staggerNonce = (m_noncesWritten + i) % m_staggerSize;
		std::size_t cpuOffset = staggerNonce * SCOOP_SIZE;
		switch(m_config->getVersion()) {
			case 1:{
				for(
					unsigned int j = 0 ;
					j < SCOOPS_PER_PLOT ;
					++j, deviceOffset += SCOOP_SIZE, cpuOffset += m_staggerSize * SCOOP_SIZE
				) {
					std::copy_n(p_work->getDevice()->getBufferCpu() + deviceOffset, SCOOP_SIZE, m_buffer + cpuOffset);
				}
				break;
			}
			case 2:{
				for(
					unsigned int j = 0 ;
					j < PLOT_SIZE ;
					j += SCOOP_SIZE, cpuOffset += m_staggerSize * SCOOP_SIZE
				) {
					std::copy_n(
						p_work->getDevice()->getBufferCpu() + deviceOffset + PLOT_SIZE - j - SCOOP_SIZE,
						HASH_SIZE,
						m_buffer + cpuOffset
					);
					std::copy_n(
						p_work->getDevice()->getBufferCpu() + deviceOffset + j,
						HASH_SIZE,
						m_buffer + cpuOffset + HASH_SIZE
					);
				}

				deviceOffset += PLOT_SIZE;
				break;
			}
		}

		if(staggerNonce == m_staggerSize - 1) {
			unsigned int staggerGroup = m_noncesWritten / m_staggerSize;
			m_plotsFile->seek((std::streamoff)staggerGroup * m_staggerSize * SCOOP_SIZE, std::ios::beg);

			std::size_t cpuOffset = 0;
			std::size_t staggerLength = (std::size_t)m_staggerSize * SCOOP_SIZE;
			for(unsigned int j = 0 ; j < SCOOPS_PER_PLOT ; ++j, cpuOffset += staggerLength) {
				m_plotsFile->write(m_buffer + cpuOffset, staggerLength);
				m_plotsFile->seek(((std::streamoff)m_config->getNoncesNumber() - m_staggerSize) * SCOOP_SIZE, std::ios::cur);
			}

			m_plotsFile->flush();
		}
	}
}

}}
