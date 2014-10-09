/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_OPENCL_PLATFORM_H
#define CRYO_GPU_PLOT_GENERATOR_OPENCL_PLATFORM_H

#include <string>
#include <vector>
#include <exception>
#include <memory>
#include <CL/cl.h>

namespace cryo {
namespace gpuPlotGenerator {

class OpenclPlatform {
	private:
		cl_platform_id m_handle;

	public:
		OpenclPlatform(const cl_platform_id& p_handle);
		OpenclPlatform(const OpenclPlatform& p_other);
		virtual ~OpenclPlatform();

		OpenclPlatform& operator=(const OpenclPlatform& p_other);

		inline const cl_platform_id& getHandle() const;

		std::string getName() const throw (std::exception);
		std::string getVendor() const throw (std::exception);
		std::string getVersion() const throw (std::exception);

	private:
		std::string getInfoString(const cl_platform_info& p_paramName) const throw (std::exception);

	public:
		static std::vector<std::shared_ptr<OpenclPlatform>> list() throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline const cl_platform_id& OpenclPlatform::getHandle() const {
	return m_handle;
}

}}

#endif
