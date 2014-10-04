#include "util.h"

namespace cryo {
namespace util {

std::vector<std::string> split(const std::string& p_string, const std::string& p_separator) {
	std::vector<std::string> parts;
	std::size_t offset = 0;
	std::size_t index;
	while((index = p_string.find(p_separator, offset)) != std::string::npos) {
		parts.push_back(p_string.substr(offset, index - offset));
		offset = index + p_separator.length();
	}

	parts.push_back(p_string.substr(offset));

	return parts;
}

}}
