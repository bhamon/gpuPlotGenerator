/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_OPENCL_DEVICE_H
#define CRYO_GPU_PLOT_GENERATOR_OPENCL_DEVICE_H

#include <string>
#include <exception>
#include <vector>
#include <memory>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "OpenclPlatform.h"

namespace cryo {
namespace gpuPlotGenerator {

class OpenclDevice {
	private:
		cl_device_id m_handle;

	public:
		OpenclDevice(const cl_device_id& p_handle) throw (std::exception);
		OpenclDevice(const OpenclDevice& p_other);

		virtual ~OpenclDevice() throw ();

		OpenclDevice& operator=(const OpenclDevice& p_other);

		inline const cl_device_id& getHandle() const;

		std::string getType() const throw (std::exception);
		std::string getName() const throw (std::exception);
		std::string getVendor() const throw (std::exception);
		std::string getVersion() const throw (std::exception);
		std::string getDriverVersion() const throw (std::exception);
		unsigned int getMaxClockFrequency() const throw (std::exception);
		unsigned int getMaxComputeUnits() const throw (std::exception);
		unsigned long long getGlobalMemorySize() const throw (std::exception);
		unsigned long long getMaxMemoryAllocationSize() const throw (std::exception);
		std::size_t getMaxWorkGroupSize() const throw (std::exception);
		unsigned long long getLocalMemorySize() const throw (std::exception);
		std::vector<std::size_t> getMaxWorkItemSizes() const throw (std::exception);

	private:
		std::string getInfoString(const cl_platform_info& p_paramName) const throw (std::exception);
		unsigned int getInfoUint(const cl_platform_info& p_paramName) const throw (std::exception);
		std::size_t getInfoSizet(const cl_platform_info& p_paramName) const throw (std::exception);
		unsigned long long getInfoUlong(const cl_platform_info& p_paramName) const throw (std::exception);

	public:
		static std::vector<std::shared_ptr<OpenclDevice>> list(const std::shared_ptr<OpenclPlatform>& p_platform) throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline const cl_device_id& OpenclDevice::getHandle() const {
	return m_handle;
}

}}

#endif
