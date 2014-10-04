/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include <fstream>
#include <sstream>
#include <streambuf>

#include "constants.h"
#include "OpenclDevice.h"
#include "OpenclError.h"

namespace cryo {
namespace gpuPlotGenerator {

OpenclDevice::OpenclDevice(cl_device_id& p_device, std::size_t p_globalWorkSize, std::size_t p_localWorkSize, unsigned int p_hashesNumber) throw (std::exception)
: m_globalWorkSize(p_globalWorkSize), m_localWorkSize(p_localWorkSize), m_hashesNumber(p_hashesNumber), m_context(0), m_commandQueue(0), m_buffer(0), m_program(0), m_kernels{0, 0, 0} {
	cl_int error;

	m_context = clCreateContext(0, 1, &p_device, NULL, NULL, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL context");
	}

	m_commandQueue = clCreateCommandQueue(m_context, p_device, 0, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL command queue");
	}

	m_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(unsigned char) * m_globalWorkSize * GEN_SIZE, 0, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL GPU buffer");
	}

	std::string source = loadSource("kernel/nonce.cl");
	const char* sources[] = {source.c_str()};
	std::size_t sourcesLength[] = {source.length()};
	m_program = clCreateProgramWithSource(m_context, 1, sources, sourcesLength, &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL program");
	}

	error = clBuildProgram(m_program, 1, &p_device, "-I kernel", 0, 0);
	if(error != CL_SUCCESS) {
		std::size_t logSize;
		cl_int subError = clGetProgramBuildInfo(m_program, p_device, CL_PROGRAM_BUILD_LOG, 0, 0, &logSize);
		if(subError != CL_SUCCESS) {
			throw OpenclError(subError, "Unable to retrieve the OpenCL build info size");
		}

		char* log = new char[logSize];
		subError = clGetProgramBuildInfo(m_program, p_device, CL_PROGRAM_BUILD_LOG, logSize, (void*)log, 0);
		if(subError != CL_SUCCESS) {
			delete[] log;
			throw OpenclError(subError, "Unable to retrieve the OpenCL build info");
		}

		std::ostringstream message;
		message << "Unable to build the OpenCL program" << std::endl;
		message << log;

		delete[] log;

		throw OpenclError(error, message.str());
	}

	m_kernels[0] = clCreateKernel(m_program, "nonce_step1", &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL step1 kernel");
	}

	error = clSetKernelArg(m_kernels[0], 0, sizeof(cl_mem), (void*)&m_buffer);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step1 kernel arguments");
	}

	m_kernels[1] = clCreateKernel(m_program, "nonce_step2", &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL step2 kernel");
	}

	error = clSetKernelArg(m_kernels[1], 0, sizeof(cl_mem), (void*)&m_buffer);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step2 kernel arguments");
	}

	m_kernels[2] = clCreateKernel(m_program, "nonce_step3", &error);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to create the OpenCL step3 kernel");
	}

	error = clSetKernelArg(m_kernels[2], 0, sizeof(cl_mem), (void*)&m_buffer);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step3 kernel arguments");
	}
}

OpenclDevice::~OpenclDevice() throw () {
	if(m_kernels[2]) { clReleaseKernel(m_kernels[2]); }
	if(m_kernels[1]) { clReleaseKernel(m_kernels[1]); }
	if(m_kernels[0]) { clReleaseKernel(m_kernels[0]); }
	if(m_program) { clReleaseProgram(m_program); }
	if(m_buffer) { clReleaseMemObject(m_buffer); }
	if(m_commandQueue) { clReleaseCommandQueue(m_commandQueue); }
	if(m_context) { clReleaseContext(m_context); }
}

void OpenclDevice::computePlots(unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_workSize) throw (std::exception) {
	if(p_workSize > m_globalWorkSize) {
		throw std::runtime_error("Global work size too low for the requested work size");
	}

	cl_int error;

	error = clSetKernelArg(m_kernels[0], 1, sizeof(unsigned int), (void*)&p_workSize);
	error |= clSetKernelArg(m_kernels[0], 2, sizeof(unsigned long long), (void*)&p_address);
	error |= clSetKernelArg(m_kernels[0], 3, sizeof(unsigned long long), (void*)&p_startNonce);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Unable to set the OpenCL step1 kernel arguments");
	}

	error = clEnqueueNDRangeKernel(m_commandQueue, m_kernels[0], 1, 0, &m_globalWorkSize, &m_localWorkSize, 0, 0, 0);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Error in step1 kernel launch");
	}

	unsigned int hashesSize = m_hashesNumber * HASH_SIZE;
	for(unsigned int i = 0 ; i < PLOT_SIZE ; i += hashesSize) {
		unsigned int hashesOffset = PLOT_SIZE - i;

		error = clSetKernelArg(m_kernels[1], 1, sizeof(unsigned int), (void*)&p_workSize);
		error |= clSetKernelArg(m_kernels[1], 2, sizeof(unsigned long long), (void*)&p_startNonce);
		error |= clSetKernelArg(m_kernels[1], 3, sizeof(unsigned int), (void*)&hashesOffset);
		error |= clSetKernelArg(m_kernels[1], 4, sizeof(unsigned int), (void*)&m_hashesNumber);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Unable to set the OpenCL step2 kernel arguments");
		}

		error = clEnqueueNDRangeKernel(m_commandQueue, m_kernels[1], 1, 0, &m_globalWorkSize, &m_localWorkSize, 0, 0, 0);
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

	error = clEnqueueNDRangeKernel(m_commandQueue, m_kernels[2], 1, 0, &m_globalWorkSize, &m_localWorkSize, 0, 0, 0);
	if(error != CL_SUCCESS) {
		throw OpenclError(error, "Error in step3 kernel launch");
	}
}

void OpenclDevice::readPlots(unsigned char* p_buffer, std::size_t p_offset, unsigned int p_size) throw (std::exception) {
	if(p_offset >= m_globalWorkSize) {
		throw std::runtime_error("Offset out of GPU buffer bounds");
	} else if(p_offset + p_size > m_globalWorkSize) {
		throw std::runtime_error("Size out of GPU buffer bounds");
	}

	std::size_t offsetGpu = p_offset * GEN_SIZE;
	std::size_t offsetCpu = 0;
	for(unsigned int i = 0 ; i < p_size ; ++i, offsetGpu += GEN_SIZE, offsetCpu += PLOT_SIZE) {
		int error = clEnqueueReadBuffer(m_commandQueue, m_buffer, CL_TRUE, sizeof(unsigned char) * offsetGpu, sizeof(unsigned char) * PLOT_SIZE, p_buffer + offsetCpu, 0, 0, 0);
		if(error != CL_SUCCESS) {
			throw OpenclError(error, "Error in synchronous read");
		}
	}
}

std::string OpenclDevice::loadSource(const std::string& p_file) const throw (std::exception) {
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
