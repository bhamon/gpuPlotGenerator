#include <stdexcept>
#include <fcntl.h>

#include "constants.h"
#include "PlotsFile.h"

namespace cryo {
namespace gpuPlotGenerator {

void PlotsFile::preallocate(const std::string& p_path, unsigned long long p_size) throw (std::exception) {
	int fd = open(p_path.c_str(), O_RDWR | O_CREAT | O_TRUNC);
	if(fd == -1) {
		throw std::runtime_error("Unable to open the output file");
	}

	if(fallocate(fd, 0, 0, p_size) == -1) {
		throw std::runtime_error("Unable to extend output file");
	}

	close(fd);
}

}}
