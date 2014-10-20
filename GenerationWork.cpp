/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include "GenerationWork.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationWork::GenerationWork(const std::shared_ptr<GenerationDevice>& p_device, unsigned long long p_startNonce, unsigned int p_workSize)
: m_device(p_device), m_startNonce(p_startNonce), m_workSize(p_workSize), m_status(GenerationStatus::Generating) {
}

GenerationWork::GenerationWork(const GenerationWork& p_other)
: m_device(p_other.m_device), m_startNonce(p_other.m_startNonce), m_workSize(p_other.m_workSize), m_status(p_other.m_status) {
}

GenerationWork::~GenerationWork() throw () {
}

GenerationWork& GenerationWork::operator=(const GenerationWork& p_other) {
	m_device = p_other.m_device;
	m_startNonce = p_other.m_startNonce;
	m_workSize = p_other.m_workSize;
	m_status = p_other.m_status;

	return *this;
}

}}
