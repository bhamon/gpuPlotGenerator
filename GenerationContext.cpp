/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <cstdlib>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "util.h"
#include "GenerationContext.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationContext::GenerationContext(const std::shared_ptr<GenerationConfig>& p_config, const std::shared_ptr<PlotsFile>& p_plotsFile)
: m_config(p_config), m_plotsFile(p_plotsFile), m_noncesDistributed(0), m_noncesWritten(0), m_pendingWorks() {
}

GenerationContext::GenerationContext(const GenerationContext& p_other)
: m_config(p_other.m_config), m_plotsFile(p_other.m_plotsFile), m_noncesDistributed(p_other.m_noncesDistributed), m_noncesWritten(p_other.m_noncesWritten), m_pendingWorks(p_other.m_pendingWorks) {
}

GenerationContext::~GenerationContext() throw () {
}

GenerationContext& GenerationContext::operator=(const GenerationContext& p_other) {
	m_config = p_other.m_config;
	m_plotsFile = p_other.m_plotsFile;
	m_noncesDistributed = p_other.m_noncesDistributed;
	m_noncesWritten = p_other.m_noncesWritten;
	m_pendingWorks = p_other.m_pendingWorks;

	return *this;
}

const std::shared_ptr<GenerationWork>& GenerationContext::requestWork(const std::shared_ptr<GenerationDevice>& p_device) throw (std::exception) {
	if(m_config->getNoncesNumber() == m_noncesDistributed) {
		throw std::runtime_error("No more work available");
	}

	std::shared_ptr<GenerationWork> generationWork(new GenerationWork(
		p_device,
		m_config->getStartNonce() + m_noncesDistributed,
		std::min((unsigned int)p_device->getConfig()->getGlobalWorkSize(), m_config->getNoncesNumber() - m_noncesDistributed)
	));

	m_noncesDistributed += generationWork->getWorkSize();
	m_pendingWorks.push_front(generationWork);

	return m_pendingWorks.front();
}

void GenerationContext::popLastPendingWork() throw (std::exception) {
	if(m_pendingWorks.size() == 0) {
		throw std::runtime_error("No pending work available");
	}

	std::shared_ptr<GenerationWork> generationWork(m_pendingWorks.back());
	if(generationWork->getStatus() != GenerationStatus::Written) {
		throw std::runtime_error("Invalid work status");
	}

	m_noncesWritten += generationWork->getWorkSize();
	m_pendingWorks.pop_back();
}

}}
