#ifndef CRYO_GPU_PLOT_GENERATOR_CONSTANTS_H
#define CRYO_GPU_PLOT_GENERATOR_CONSTANTS_H

#include <string>

namespace cryo {
namespace gpuPlotGenerator {

const unsigned int HASH_SIZE = 32;
const unsigned int HASHES_PER_SCOOP = 2;
const unsigned int SCOOP_SIZE = HASHES_PER_SCOOP * HASH_SIZE;
const unsigned int SCOOPS_PER_PLOT = 4096;
const unsigned int PLOT_SIZE = SCOOPS_PER_PLOT * SCOOP_SIZE;
const unsigned int HASH_CAP = 4096;
const unsigned int GEN_SIZE = PLOT_SIZE + 16;
const unsigned int IO_CAP = 1000000000;

const std::string DEVICES_FILE("devices.txt");
const std::string KERNEL_PATH("kernel");

}}

#endif
