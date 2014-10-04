/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include "Command.h"

namespace cryo {
namespace gpuPlotGenerator {

Command::Command(const std::string& p_description)
: m_description(p_description) {
}

Command::Command(const Command& p_command)
: m_description(p_command.m_description) {
}

Command::~Command() throw () {
}

}}
