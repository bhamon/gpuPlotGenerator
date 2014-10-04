/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_COMMAND_VERIFY_H
#define CRYO_GPU_PLOT_GENERATOR_COMMAND_VERIFY_H

#include <string>
#include <vector>

#include "Command.h"

namespace cryo {
namespace gpuPlotGenerator {

class CommandVerify : public cryo::gpuPlotGenerator::Command {
	public:
		CommandVerify();
		CommandVerify(const CommandVerify& p_command);
		virtual ~CommandVerify() throw ();

		virtual void help() const;
		virtual int execute(const std::vector<std::string>& p_args);
};

}}

#endif
