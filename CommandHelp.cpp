/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>

#include "CommandHelp.h"

namespace cryo {
namespace gpuPlotGenerator {

CommandHelp::CommandHelp(const CommandsMap& p_commands)
: Command("Print this message."), m_commands(p_commands) {
}

CommandHelp::CommandHelp(const CommandHelp& p_command)
: Command(p_command), m_commands(p_command.m_commands) {
}

CommandHelp::~CommandHelp() throw () {
}

void CommandHelp::help() const {
	std::cout << "Usage: ./gpuPlotGenerator <command> ..." << std::endl;
	std::cout << "    Executes the specified command." << std::endl;
	std::cout << "Usage: ./gpuPlotGenerator help <command>" << std::endl;
	std::cout << "    Print the usage and help of the specified command." << std::endl;
	std::cout << "Parameters:" << std::endl;
	std::cout << "    - command: The command name." << std::endl;
	std::cout << "Commands:" << std::endl;

	for(CommandsMap::const_reference entry : m_commands) {
		std::cout << "    - " << entry.first << ": " << entry.second->getDescription() << std::endl;
	}
}

int CommandHelp::execute(const std::vector<std::string>& p_args) {
	if(p_args.size() < 1) {
		help();
		return 0;
	}

	std::string command(p_args[0]);
	if(m_commands.find(command) == m_commands.end()) {
		std::cout << "[ERROR] Unknown [" << command << "] command" << std::endl;
		std::cout << "----" << std::endl;

		help();
		return -1;
	}

	m_commands.at(command)->help();
	return 0;
}

}}
