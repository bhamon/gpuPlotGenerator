/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <algorithm>

#include "constants.h"
#include "GenerationWriterBuffer.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationWriterBuffer::GenerationWriterBuffer(std::size_t p_bufferDeviceSize, std::size_t p_bufferStaggerSize) throw (std::exception)
: m_bufferDeviceSize(p_bufferDeviceSize), m_bufferStaggerSize(p_bufferStaggerSize) {
	m_bufferDevice = new unsigned char[m_bufferDeviceSize];
	m_bufferStagger = new unsigned char[m_bufferStaggerSize];
}

GenerationWriterBuffer::~GenerationWriterBuffer() throw () {
	delete[] m_bufferDevice;
	delete[] m_bufferStagger;
}

unsigned long long GenerationWriterBuffer::getMemorySize() const {
	return (unsigned long long)m_bufferDeviceSize + m_bufferStaggerSize;
}

void GenerationWriterBuffer::readPlots(std::shared_ptr<GenerationContext>&, std::shared_ptr<GenerationWork>& p_work) throw (std::exception) {
	p_work->getDevice()->readPlots(m_bufferDevice, 0, p_work->getWorkSize());
}

void GenerationWriterBuffer::writeNonces(std::shared_ptr<GenerationContext>& p_context, std::shared_ptr<GenerationWork>& p_work) throw (std::exception) {
	unsigned int staggerSize = p_context->getConfig()->getStaggerSize();
	std::size_t bufferDeviceOffset = 0;
	for(unsigned int i = 0, end = p_work->getWorkSize() ; i < end ; ++i, bufferDeviceOffset += PLOT_SIZE) {
		unsigned int staggerNonce = (p_context->getNoncesWritten() + i) % staggerSize;
		for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE) {
			std::copy_n(m_bufferDevice + bufferDeviceOffset + j, SCOOP_SIZE, m_bufferStagger + (std::size_t)staggerNonce * SCOOP_SIZE + (std::size_t)j * staggerSize);
		}

		if(staggerNonce == staggerSize - 1) {
			p_context->getPlotsFile()->write(m_bufferStagger, (std::streamsize)PLOT_SIZE * staggerSize);
			p_context->getPlotsFile()->flush();
		}
	}
}

}}
