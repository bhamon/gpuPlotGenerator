/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_OPENCL_ERROR_H
#define CRYO_GPU_PLOT_GENERATOR_OPENCL_ERROR_H

#include <string>
#include <stdexcept>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

namespace cryo {
namespace gpuPlotGenerator {

class OpenclError : public std::runtime_error {
	private:
		cl_int m_code;

	public:
		OpenclError(cl_int p_code, const std::string& p_message);
		OpenclError(cl_int p_code, const char* p_message);
		OpenclError(const OpenclError& p_other);

		virtual ~OpenclError() throw ();

		OpenclError& operator=(const OpenclError& p_other);

		inline cl_int getCode() const;
		std::string getCodeString() const;
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline cl_int OpenclError::getCode() const {
	return m_code;
}

}}

#endif
