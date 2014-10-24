/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_COMMAND_GENERATE_H
#define CRYO_GPU_PLOT_GENERATOR_COMMAND_GENERATE_H

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <exception>

#include "Command.h"
#include "GenerationDevice.h"
#include "GenerationContext.h"
#include "GenerationWork.h"
#include "GenerationWriter.h"

namespace cryo {
namespace gpuPlotGenerator {

class CommandGenerate : public cryo::gpuPlotGenerator::Command {
	public:
		CommandGenerate();
		CommandGenerate(const CommandGenerate& p_command);
		virtual ~CommandGenerate() throw ();

		virtual void help() const;
		virtual int execute(const std::vector<std::string>& p_args);
};

void computePlots(
	std::exception_ptr& p_error,
	std::mutex& p_mutex,
	std::condition_variable& p_barrier,
	std::list<std::shared_ptr<GenerationContext>>& p_generationContexts,
	std::shared_ptr<GenerationDevice>& p_generationDevice
) throw (std::exception);

void writeNonces(
	std::exception_ptr& p_error,
	std::mutex& p_mutex,
	std::condition_variable& p_barrier,
	std::list<std::shared_ptr<GenerationContext>>& p_generationContexts,
	std::shared_ptr<GenerationWriter>& p_writer
) throw (std::exception);

}}

#endif
