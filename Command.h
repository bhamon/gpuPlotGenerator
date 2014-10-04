/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_COMMAND_H
#define CRYO_GPU_PLOT_GENERATOR_COMMAND_H

#include <string>
#include <vector>

namespace cryo {
namespace gpuPlotGenerator {

class Command {
	protected:
		std::string m_description;

	public:
		Command(const std::string& p_description);
		Command(const Command& p_command);
		virtual ~Command() throw ();

		inline const std::string& getDescription() const;

		virtual void help() const = 0;
		virtual int execute(const std::vector<std::string>& p_args) = 0;
};

inline const std::string& Command::getDescription() const {
	return m_description;
}

}}

#endif
