/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef CRYO_GPU_PLOT_GENERATOR_GENERATION_CONFIG_H
#define CRYO_GPU_PLOT_GENERATOR_GENERATION_CONFIG_H

#include <string>
#include <vector>
#include <memory>
#include <exception>

#include "constants.h"
#include "OpenclDevice.h"

namespace cryo {
namespace gpuPlotGenerator {

class GenerationConfig {
	private:
		std::size_t m_platform;
		std::size_t m_device;
		std::size_t m_globalWorkSize;
		std::size_t m_localWorkSize;
		unsigned int m_hashesNumber;

	public:
		GenerationConfig(std::size_t p_platform, std::size_t p_device, std::size_t p_globalWorkSize, std::size_t p_localWorkSize, unsigned int p_hashesNumber) throw (std::exception);
		GenerationConfig(const GenerationConfig& p_other);

		virtual ~GenerationConfig() throw ();

		GenerationConfig& operator=(const GenerationConfig& p_other);

		inline std::size_t getPlatform() const;
		inline std::size_t getDevice() const;
		inline std::size_t getGlobalWorkSize() const;
		inline std::size_t getLocalWorkSize() const;
		inline unsigned int getHashesNumber() const;
		inline void setGlobalWorkSize(std::size_t p_globalWorkSize);
		inline void setLocalWorkSize(std::size_t p_localWorkSize);
		inline void setHashesNumber(unsigned int p_hashesNumber);
		inline std::size_t& globalWorkSize();
		inline std::size_t& localWorkSize();
		inline unsigned int& hashesNumber();

		inline unsigned long long getBufferSize() const;

		void normalize();

	public:
		static std::vector<std::shared_ptr<GenerationConfig>> loadFromFile(const std::string& p_path) throw (std::exception);
		static void storeToFile(const std::string& p_path, const std::vector<std::shared_ptr<GenerationConfig>>& p_configs) throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline std::size_t GenerationConfig::getPlatform() const {
	return m_platform;
}

inline std::size_t GenerationConfig::getDevice() const {
	return m_device;
}

inline std::size_t GenerationConfig::getGlobalWorkSize() const {
	return m_globalWorkSize;
}

inline std::size_t GenerationConfig::getLocalWorkSize() const {
	return m_localWorkSize;
}

inline unsigned int GenerationConfig::getHashesNumber() const {
	return m_hashesNumber;
}

inline void GenerationConfig::setGlobalWorkSize(std::size_t p_globalWorkSize) {
	m_globalWorkSize = p_globalWorkSize;
}

inline void GenerationConfig::setLocalWorkSize(std::size_t p_localWorkSize) {
	m_localWorkSize = p_localWorkSize;
}

inline std::size_t& GenerationConfig::globalWorkSize() {
	return m_globalWorkSize;
}

inline std::size_t& GenerationConfig::localWorkSize() {
	return m_localWorkSize;
}

inline unsigned int& GenerationConfig::hashesNumber() {
	return m_hashesNumber;
}

inline void GenerationConfig::setHashesNumber(unsigned int p_hashesNumber) {
	m_hashesNumber = p_hashesNumber;
}

inline unsigned long long GenerationConfig::getBufferSize() const {
	return m_globalWorkSize * GEN_SIZE;
}

}}

#endif
