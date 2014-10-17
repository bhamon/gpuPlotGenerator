/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_COMMAND_HELP_H
#define CRYO_GPU_PLOT_GENERATOR_COMMAND_HELP_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Command.h"

namespace cryo {
namespace gpuPlotGenerator {

class CommandHelp : public cryo::gpuPlotGenerator::Command {
	public:
		typedef std::map<std::string, std::shared_ptr<cryo::gpuPlotGenerator::Command>> CommandsMap;

	private:
		const CommandsMap& m_commands;

	public:
		CommandHelp(const CommandsMap& p_commands);
		CommandHelp(const CommandHelp& p_command);
		virtual ~CommandHelp() throw ();

		virtual void help() const;
		virtual int execute(const std::vector<std::string>& p_args);
};

}}

#endif
