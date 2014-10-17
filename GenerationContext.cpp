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
: m_config(p_config), m_plotsFile(p_plotsFile), m_noncesDistributed(0), m_noncesWritten(0), m_available(true) {
}

GenerationContext::GenerationContext(const GenerationContext& p_other)
: m_config(p_other.m_config), m_plotsFile(p_other.m_plotsFile), m_noncesDistributed(p_other.m_noncesDistributed), m_noncesWritten(p_other.m_noncesWritten), m_available(p_other.m_available) {
}

GenerationContext::~GenerationContext() throw () {
}

GenerationContext& GenerationContext::operator=(const GenerationContext& p_other) {
	m_config = p_other.m_config;
	m_plotsFile = p_other.m_plotsFile;
	m_noncesDistributed = p_other.m_noncesDistributed;
	m_noncesWritten = p_other.m_noncesWritten;
	m_available = p_other.m_available;

	return *this;
}

unsigned int GenerationContext::requestWorkSize(unsigned int p_maxSize) {
	unsigned int workSize = std::min(p_maxSize, m_config->getNoncesNumber() - m_noncesDistributed);
	m_noncesDistributed += workSize;
	return workSize;
}

void GenerationContext::appendWorkSize(unsigned int p_workSize) {
	m_noncesWritten += p_workSize;
}

}}
