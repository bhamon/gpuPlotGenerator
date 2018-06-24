/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>

#include "CommandHelp.h"
#include "CommandListPlatforms.h"
#include "CommandListDevices.h"
#include "CommandSetup.h"
#include "CommandGenerate.h"
#include "CommandVerify.h"

int main(int p_argc, char* p_argv[]) {
	std::cout << "-------------------------" << std::endl;
	std::cout << "GPU plot generator v4.2.1" << std::endl;
	std::cout << "-------------------------" << std::endl;
	std::cout << "Author:   Cryo" << std::endl;
	std::cout << "Bitcoin:  138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD" << std::endl;
	std::cout << "Burst:    BURST-YA29-QCEW-QXC3-BKXDL" << std::endl;
	std::cout << "----" << std::endl;

	typedef cryo::gpuPlotGenerator::CommandHelp::CommandsMap CommandsMap;
	CommandsMap commands;
	commands.insert(CommandsMap::value_type("help", CommandsMap::mapped_type(new cryo::gpuPlotGenerator::CommandHelp(commands))));
	commands.insert(CommandsMap::value_type("listPlatforms", CommandsMap::mapped_type(new cryo::gpuPlotGenerator::CommandListPlatforms())));
	commands.insert(CommandsMap::value_type("listDevices", CommandsMap::mapped_type(new cryo::gpuPlotGenerator::CommandListDevices())));
	commands.insert(CommandsMap::value_type("setup", CommandsMap::mapped_type(new cryo::gpuPlotGenerator::CommandSetup())));
	commands.insert(CommandsMap::value_type("generate", CommandsMap::mapped_type(new cryo::gpuPlotGenerator::CommandGenerate())));
	commands.insert(CommandsMap::value_type("verify", CommandsMap::mapped_type(new cryo::gpuPlotGenerator::CommandVerify())));

	if(p_argc == 1) {
		commands.at("help")->help();
		return -1;
	}

	std::string command(p_argv[1]);
	std::vector<std::string> args(p_argv + 2, p_argv + p_argc);
	if(commands.find(command) == commands.end()) {
		std::cout << "[ERROR] Unknown [" << command << "] command" << std::endl;
		std::cout << "----" << std::endl;

		commands.at("help")->help();
		return -1;
	}

	return commands.at(command)->execute(args);
}
