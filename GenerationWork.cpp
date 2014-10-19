//TODO
// status
// flush
// stack dans génération contexte
// Je t'aime, mon petit geek mouton d'amour =)

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
