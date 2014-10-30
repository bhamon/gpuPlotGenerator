/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <stdexcept>
#include <sstream>
#include <fstream>
#include <streambuf>

#include "OpenclError.h"
#include "GenerationDevice.h"

namespace cryo {
namespace gpuPlotGenerator {

GenerationDevice::GenerationDevice(const std::shared_ptr<DeviceConfig>& p_config, const std::shared_ptr<OpenclDevice>& p_device) throw (std::exception)
: m_config(p_config), m_device(p_device), m_context(0), m_commandQueue(0), m_bufferDevice(0), m_program(0), m_kernels{0, 0, 0}, m_available(true) {
	m_bufferCpu = new unsigned char[getMemorySize()];

	cl_int error;

	m_context = clCreateContext(0, 1, &m_device->getHandle(), NULL, NULL, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL context");
	}

	m_commandQueue = clCreateCommandQueue(m_context, m_device->getHandle(), 0, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL command queue");
	}

	m_bufferDevice = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(unsigned char) * m_config->getGlobalWorkSize() * GEN_SIZE, 0, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL GPU buffer");
	}

	std::string source(loadSource(KERNEL_PATH + "/nonce.cl"));
	const char* sources[] = {source.c_str()};
	std::size_t sourcesLength[] = {source.length()};
	m_program = clCreateProgramWithSource(m_context, 1, sources, sourcesLength, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL program");
	}

	std::string includePath("-I " + KERNEL_PATH);
	error = clBuildProgram(m_program, 1, &m_device->getHandle(), includePath.c_str(), 0, 0);
	if(error != CL_SUCCESS) {
		std::size_t logSize;
		cl_int subError = clGetProgramBuildInfo(m_program, m_device->getHandle(), CL_PROGRAM_BUILD_LOG, 0, 0, &logSize);
		if(subError != CL_SUCCESS) {
			throw OpenclError(subError, "Unable to retrieve the OpenCL build info size");
		}

		std::unique_ptr<char[]> log(new char[logSize]);
		subError = clGetProgramBuildInfo(m_program, m_device->getHandle(), CL_PROGRAM_BUILD_LOG, logSize, (void*)log.get(), 0);
		if(subError != CL_SUCCESS) {
			throw OpenclError(subError, "Unable to retrieve the OpenCL build info");
		}

		std::ostringstream message;
		message << "Unable to build the OpenCL program" << std::endl;
		message << log.get();

		throw OpenclError(error, message.str());
	}

	m_kernels[0] = clCreateKernel(m_program, "nonce_step1", &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL step1 kernel");
	}

	error = clSetKernelArg(m_kernels[0], 0, sizeof(cl_mem), (void*)&m_bufferDevice);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step1 kernel arguments");
	}

	m_kernels[1] = clCreateKernel(m_program, "nonce_step2", &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL step2 kernel");
	}

	error = clSetKernelArg(m_kernels[1], 0, sizeof(cl_mem), (void*)&m_bufferDevice);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step2 kernel arguments");
	}

	m_kernels[2] = clCreateKernel(m_program, "nonce_step3", &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL step3 kernel");
	}

	error = clSetKernelArg(m_kernels[2], 0, sizeof(cl_mem), (void*)&m_bufferDevice);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step3 kernel arguments");
	}
}

GenerationDevice::~GenerationDevice() throw () {
	if(m_kernels[2]) { clReleaseKernel(m_kernels[2]); }
	if(m_kernels[1]) { clReleaseKernel(m_kernels[1]); }
	if(m_kernels[0]) { clReleaseKernel(m_kernels[0]); }
	if(m_program) { clReleaseProgram(m_program); }
	if(m_bufferDevice) { clReleaseMemObject(m_bufferDevice); }
	if(m_commandQueue) { clReleaseCommandQueue(m_commandQueue); }
	if(m_context) { clReleaseContext(m_context); }

	delete[] m_bufferCpu;
}

void GenerationDevice::computePlots(unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_workSize) throw (std::exception) {
	if(p_workSize > m_config->getGlobalWorkSize()) {
		throw std::runtime_error("Global work size too low for the requested work size");
	}

	cl_int error;
	std::size_t globalWorkSize = m_config->getGlobalWorkSize();
	std::size_t localWorkSize = m_config->getLocalWorkSize();

	error = clSetKernelArg(m_kernels[0], 1, sizeof(unsigned int), (void*)&p_workSize);
	error |= clSetKernelArg(m_kernels[0], 2, sizeof(unsigned long long), (void*)&p_address);
	error |= clSetKernelArg(m_kernels[0], 3, sizeof(unsigned long long), (void*)&p_startNonce);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step1 kernel arguments");
	}

	error = clEnqueueNDRangeKernel(m_commandQueue, m_kernels[0], 1, 0, &globalWorkSize, &localWorkSize, 0, 0, 0);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Error in step1 kernel launch");
	}

	unsigned int hashesNumber = m_config->getHashesNumber();
	unsigned int hashesSize = hashesNumber * HASH_SIZE;
	for(unsigned int i = 0 ; i < PLOT_SIZE ; i += hashesSize) {
		unsigned int hashesOffset = PLOT_SIZE - i;

		error = clSetKernelArg(m_kernels[1], 1, sizeof(unsigned int), (void*)&p_workSize);
		error |= clSetKernelArg(m_kernels[1], 2, sizeof(unsigned long long), (void*)&p_startNonce);
		error |= clSetKernelArg(m_kernels[1], 3, sizeof(unsigned int), (void*)&hashesOffset);
		error |= clSetKernelArg(m_kernels[1], 4, sizeof(unsigned int), (void*)&hashesNumber);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to set the OpenCL step2 kernel arguments");
		}

		error = clEnqueueNDRangeKernel(m_commandQueue, m_kernels[1], 1, 0, &globalWorkSize, &localWorkSize, 0, 0, 0);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Error in step2 kernel launch");
		}

		error = clFinish(m_commandQueue);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Error in step2 kernel finish");
		}
	}

	error = clSetKernelArg(m_kernels[2], 1, sizeof(unsigned int), (void*)&p_workSize);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step3 kernel arguments");
	}

	error = clEnqueueNDRangeKernel(m_commandQueue, m_kernels[2], 1, 0, &globalWorkSize, &localWorkSize, 0, 0, 0);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Error in step3 kernel launch");
	}
}

void GenerationDevice::bufferPlots() throw (std::exception) {
	std::size_t offsetGpu = 0;
	std::size_t offsetCpu = 0;
	for(unsigned int i = 0, end = m_config->getGlobalWorkSize() ; i < end ; ++i, offsetGpu += GEN_SIZE, offsetCpu += PLOT_SIZE) {
		int error = clEnqueueReadBuffer(m_commandQueue, m_bufferDevice, CL_TRUE, sizeof(unsigned char) * offsetGpu, sizeof(unsigned char) * PLOT_SIZE, m_bufferCpu + offsetCpu, 0, 0, 0);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Error in synchronous read");
		}
	}
}

std::string GenerationDevice::loadSource(const std::string& p_file) const throw (std::exception) {
	std::ifstream stream(p_file, std::ios::in);
	if(!stream) {
		throw std::runtime_error("Unable to open the source file");
	}

	std::string str;

	stream.seekg(0, std::ios::end);
	str.reserve(stream.tellg());
	stream.seekg(0, std::ios::beg);

	str.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

	return str;
}

}}
