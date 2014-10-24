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
#include <exception>

namespace cryo {
namespace gpuPlotGenerator {

class GenerationConfig {
	private:
		std::string m_path;
		unsigned long long m_address;
		unsigned long long m_startNonce;
		unsigned int m_noncesNumber;
		unsigned int m_staggerSize;

	public:
		GenerationConfig(const std::string& p_fullPath) throw (std::exception);
		GenerationConfig(const std::string& p_path, unsigned long long p_address, unsigned long long p_startNonce, unsigned int p_noncesNumber, unsigned int p_staggerSize);
		GenerationConfig(const GenerationConfig& p_other);

		virtual ~GenerationConfig() throw ();

		GenerationConfig& operator=(const GenerationConfig& p_other);

		inline std::string getPath() const;
		inline unsigned long long getAddress() const;
		inline unsigned long long getStartNonce() const;
		inline unsigned long long getEndNonce() const;
		inline unsigned int getNoncesNumber() const;
		inline unsigned int getStaggerSize() const;

		std::string getFullPath() const;
		unsigned long long getNoncesSize() const;
		unsigned long long getNonceStaggerOffset(unsigned long long p_nonce) const throw (std::exception);
		unsigned long long getNonceStaggerDecal(unsigned long long p_nonce) const throw (std::exception);

		void normalize() throw (std::exception);
};

}}

namespace cryo {
namespace gpuPlotGenerator {

inline std::string GenerationConfig::getPath() const {
	return m_path;
}

inline unsigned long long GenerationConfig::getAddress() const {
	return m_address;
}

inline unsigned long long GenerationConfig::getStartNonce() const {
	return m_startNonce;
}

inline unsigned long long GenerationConfig::getEndNonce() const {
	return m_startNonce + m_noncesNumber - 1;
}

inline unsigned int GenerationConfig::getNoncesNumber() const {
	return m_noncesNumber;
}

inline unsigned int GenerationConfig::getStaggerSize() const {
	return m_staggerSize;
}

}}

#endif
