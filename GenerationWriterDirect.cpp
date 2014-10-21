/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include "GenerationWriterDirect.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationWriterDirect::GenerationWriterDirect(std::size_t p_bufferDeviceSize) throw (std::exception)
: m_bufferDeviceSize(p_bufferDeviceSize) {
	m_bufferDevice = new unsigned char[m_bufferDeviceSize];
}

GenerationWriterDirect::~GenerationWriterDirect() throw () {
	delete[] m_bufferDevice;
}

unsigned long long GenerationWriterDirect::getMemorySize() const {
	return m_bufferDeviceSize;
}

void GenerationWriterDirect::readPlots(std::shared_ptr<GenerationContext>&, std::shared_ptr<GenerationWork>& p_work) throw (std::exception) {
	p_work->getDevice()->readPlots(m_bufferDevice, 0, p_work->getWorkSize());
}

void GenerationWriterDirect::writeNonces(std::shared_ptr<GenerationContext>& p_context, std::shared_ptr<GenerationWork>& p_work) throw (std::exception) {
	unsigned long long startNonce = p_context->getConfig()->getStartNonce() + p_context->getNoncesWritten();
	unsigned int staggerSize = p_context->getConfig()->getStaggerSize();
	std::size_t bufferDeviceOffset = 0;
	for(unsigned int i = 0, end = p_work->getWorkSize() ; i < end ; ++i) {
		std::streamoff nonceStaggerOffset = p_context->getConfig()->getNonceStaggerOffset(startNonce + i);
		std::streamoff nonceStaggerDecal = p_context->getConfig()->getNonceStaggerDecal(startNonce + i);

		for(unsigned int j = 0 ; j < PLOT_SIZE ; j += SCOOP_SIZE, bufferDeviceOffset += SCOOP_SIZE) {
			p_context->getPlotsFile()->seek(nonceStaggerOffset + nonceStaggerDecal + (std::streamoff)j * staggerSize);
			p_context->getPlotsFile()->write(m_bufferDevice + bufferDeviceOffset, SCOOP_SIZE);
		}

		p_context->getPlotsFile()->flush();
	}
}

}}
