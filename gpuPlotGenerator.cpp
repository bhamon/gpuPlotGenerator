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

#include "CommandHelp.h"
#include "CommandListPlatforms.h"
#include "CommandListDevices.h"
#include "CommandSetup.h"
#include "CommandGenerate.h"
#include "CommandVerify.h"

int main(int p_argc, char* p_argv[]) {
	std::cout << "-------------------------" << std::endl;
	std::cout << "GPU plot generator v3.1.0" << std::endl;
	std::cout << "-------------------------" << std::endl;
	std::cout << "Author:   Cryo" << std::endl;
	std::cout << "Bitcoin:  138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD" << std::endl;
	std::cout << "Burst:    BURST-YA29-QCEW-QXC3-BKXDL" << std::endl;
	std::cout << "----" << std::endl;

	typedef std::map<std::string, cryo::gpuPlotGenerator::Command*> CommandsMap;
	CommandsMap commands;
	commands.insert(CommandsMap::value_type("help", new cryo::gpuPlotGenerator::CommandHelp(commands)));
	commands.insert(CommandsMap::value_type("listPlatforms", new cryo::gpuPlotGenerator::CommandListPlatforms()));
	commands.insert(CommandsMap::value_type("listDevices", new cryo::gpuPlotGenerator::CommandListDevices()));
	commands.insert(CommandsMap::value_type("setup", new cryo::gpuPlotGenerator::CommandSetup()));
	commands.insert(CommandsMap::value_type("generate", new cryo::gpuPlotGenerator::CommandGenerate()));
	commands.insert(CommandsMap::value_type("verify", new cryo::gpuPlotGenerator::CommandVerify()));

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

	int returnCode = commands.at(command)->execute(args);
	for(CommandsMap::const_reference entry : commands) {
		delete entry.second;
	}

	return returnCode;
}
