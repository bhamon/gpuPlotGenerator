#ifndef CRYO_UTIL_H
#define CRYO_UTIL_H

#include <string>
#include <vector>
#include <exception>

namespace cryo {
namespace util {

std::vector<std::string> split(const std::string& p_string, const std::string& p_separator);
template<typename Iterator> std::string join(Iterator& p_begin, Iterator& p_end, const std::string& p_separator);

template<typename T> std::vector<T> splitValue(T p_value, const std::vector<T>& p_units) throw (std::exception);
template<typename T> std::string formatValue(T p_value, const std::vector<T>& p_units, const std::vector<std::string>& p_labels) throw (std::exception);

}}

#include <sstream>
#include <stdexcept>

namespace cryo {
namespace util {

template<typename Iterator>
std::string join(Iterator p_begin, Iterator p_end, const std::string& p_separator) {
	if(p_begin == p_end) {
		return "";
	}

	std::ostringstream out;
	out << *p_begin++;

	for(; p_begin != p_end ; p_begin++) {
		out << p_separator << *p_begin;
	}

	return out.str();
}

template<typename T>
std::vector<T> splitValue(T p_value, const std::vector<T>& p_units) throw (std::exception) {
	if(p_units.size() == 0) {
		throw std::runtime_error("Units vector must contain at least one element");
	}

	std::vector<T> parts;
	typename std::vector<T>::const_iterator it(p_units.begin());
	typename std::vector<T>::const_iterator end(p_units.end());

	do {
		parts.push_back(p_value % *it);
		p_value /= *it++;
	} while(it != end && p_value > 0);

	if(p_value > 0) {
		parts.push_back(p_value);
	}

	return parts;
}

template<typename T>
std::string formatValue(T p_value, const std::vector<T>& p_units, const std::vector<std::string>& p_labels) throw (std::exception) {
	std::vector<T> parts(splitValue(p_value, p_units));
	if(parts.size() > p_labels.size()) {
		throw std::runtime_error("Not enough labels to format the specified value");
	}

	typename std::vector<T>::const_reverse_iterator it(parts.crbegin());
	typename std::vector<T>::const_reverse_iterator end(parts.crend());
	std::vector<std::string>::const_iterator label(p_labels.begin() + parts.size() - 1);

	std::ostringstream out;
	out << *it++ << *label--;

	while(it != end) {
		out << " " << *it++ << *label--;
	}

	return out.str();
}

}}

#endif
