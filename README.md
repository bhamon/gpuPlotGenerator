# GPU plot generator for Burst coin.

A GPU plot generator for Burst coin.
- Author: Cryo
- Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
- Burst: BURST-YA29-QCEW-QXC3-BKXDL

Features :
- Multiple devices support to generate plots.
- Multiple output files support to enhance throughput.
- Buffered or direct writing support.
- Embedded verification tool.
- Easy setup with the embedded tool.
- Clear generation information with percent done and estimated time of arrival.

Based on the code of the official miner and dcct's plotgen.

If you like this software, support me ;)

## Build instructions

### Windows

Install msys/mingw.
Install OpenCL (available in the manufacturer SDKs) or find the include/lib files on your hard drive (some drivers
include those files).

Modify the [PLATFORM] variable to one of [32] or [64] depending on the target platform.
Modify the [OPENCL_INCLUDE] and [OPENCL_LIB] variables of the Makefile to the correct path.
Example:

	OPENCL_INCLUDE = c:\AMDAPPSDK-2.9-1\include
	OPENCL_LIB = c:\AMDAPPSDK-2.9-1\lib\x86_64

Run the following commands :

	cd <this directory>
	make dist

The [dist] folder contains all the necessary files to launch the GPU plotter.

### Linux

Install the build-essential and g++ packets.
Install OpenCL (available in the manufacturer SDKs).
You may have to install the opencl headers ([apt-get install opencl-headers] on Ubuntu).

Modify the [PLATFORM] variable to one of [32] or [64] depending on the target platform.
Modify the [OPENCL_INCLUDE] and [OPENCL_LIB] variables of the Makefile to the correct path.
Example:

	OPENCL_INCLUDE = /opt/AMDAPPSDK-2.9-1/include
	OPENCL_LIB = /opt/AMDAPPSDK-2.9-1/lib/x86_64

Run the following commands :

	cd <this directory>
	make dist

The [dist] folder contains all the necessary files to launch the GPU plotter.

## Setup

The GPU plot generator needs a configured [devices.txt] file in order to work properly. The devices listed
in this file will be used in parrallel by the generation process to compute plots. You can create this file
at hand with the following instructions or directly run the [setup] command described below to be guided through
the [devices.txt] creation process.

Each device must be declared on its own line in the following format:

	[platformId] [deviceId] [globalWorkSize] [localWorkSize] [hashesNumber]

- platformId: The platform id of the device. Can be retrieved with the [listPlatforms] command.
- deviceId: The device id. Can be retrieved with the [listDevices] command.
- globalWorkSize: The amount of nonces to process at the same time (determine the required amount of GPU memory). Should be a power of 2.
- localWorkSize: The amount of parrallel threads that computes the nonces. Must be less or equal than [globalWorkSize]. Should be a power of 2.
- hashesNumber: The number of hashes to compute per GPU calls. Must be between 1 and 8192. Use a value under 8192 only if you experience driver crashes or display freezes.

Example [devices.txt] file content:

	0 0 1024 128 8192
	0 1 2048 64 10

## How to use

Use the commands below to have the commands list and usage:

	./gpuPlotGenerator
	Displays a full help message with all the available commands.

	./gpuPlotGenerator help <command>
	Displays a per-command help message.

	./gpuPlotGenerator <command> ...
	Executes the specified command.

### List platforms

This command lists and describes the OpenCL platforms on your system. Each platform contains one or more devices
that can be used by the generation process.

Example usage:

	./gpuPlotGenerator listPlatforms

### List devices

This command lists and describes the OpenCL devices for a specific platform.

Example usage:

	./gpuPlotGenerator listDevices 0

### Setup

This command is a step by step guide to create the [devices.txt] configuration file.

Example usage:

	./gpuPlotGenerator setup

Use the displayed menu items to add/remove devices from you configuration file.
When adding a device, recommended values for each device parameter will be displayed to help you find the
best parameters for your device.
Don't forget to save before exiting.

### Generation

This command generate nonces using the configured devices and write them to the specified output files.
The generation parameters are inferred from the output files names. The files names format must be:

	<address>_<startNonce>_<noncesNumber>_<staggerSize>

Example usage:

	./gpuPlotGenerator generate auto /path/to/files/123456_0_50000_5000 /path/123456_50000_10000_2000
	This call will generate two plots files.

### Verification

This command verify overlapping nonces between two plots files. It is usefull to verify a plots file integrity.

Example usage:

	./gpuPlotGenerator verify /path/to/generated/123456_0_1000000_8000 /path/to/verification/123456_275000_10_10
	This call will verify that the nonces 275000 to 275009 matches between the two files.

## Troubleshooting

#### No platform is detected on the computer.
#### No device is found on the platforms.
Make sure your hardware is OpenCL compliant. Make sure you have the last drivers installed.

#### After launch, the display freezes and the screen goes black. Few seconds later, the display comes back but the program fails with an OpenCL error and the OS display an information message about a display driver crash.
#### After launch, the display freezes, the screen goes black and, finally, the computer reboots.
The graphic card you use is bound to your display. As the plot generation is an heavy process, it uses all of your graphic card resources in one call. To prevent display crashes, the driver kills all kernels that hang for too long.

To solve this issue, the global work needs to be split. This is the purpose of the [hashesNumber] parameter. Begin with a low value (from 1 to 10), and try to increase it until you experience some display lags.

#### The program fails with a [std::bad_alloc] message.
There is not enough available memory on the CPU side. Try a lower value for the [staggerSize] parameter.

#### The program fails with a [CL_INVALID_WORK_GROUP_SIZE] OpenCL error code.
The specified [globalWorkGroup] isn't valid. Start with a low value (32 or 64) and increase it progressively. Try to use powers of 2 that are much more compliant among graphic cards.

#### The nonces/minutes are incredibly low (no more than few hundreds).
There are many causes for that:
- The configured [globalWorkSize] and [localWorkSize] for your cards are not optimal. Try with different values.
- Some of your cards causes bottlenecks. Tests them one after another to detect the one causing it and try to improve its parameters.
- For NVidia owners: OpenCL implementation is under-efficient on some NVidia cards. Try to tweak your device parameters. If there is no significant improvements, you'll need a CUDA version of this program.

## Tweaks

The preferred way to generate plots is to use graphic cards that aren't bound to the display rendering loop.

Here are some advices to obtain better performances:
- Use powers of 2 values for the [globalWorkSize] and [localWorkSize].
- Use the higher value of [globalWorkSize].
- Use the higher value of [localWorkSize].
- Make sure the [globalWorkSize] is a multiple of [localWorkSize].

To find the best parameters for your graphic card, you can use the informations displayed by the [listDevices] command :
- The [globalWorkSize] can go up to the "Max global memory size" / PLOT_SIZE.
- The [globalWorkSize] can't go above the "Max memory allocation size" / PLOT_SIZE.
- The [globalWorkSize] should be less or equal than the "Max work group size" (doesn't apply to some devices, mostly CPUs).
- The [localWorkSize] should be less or equal than the "Max compute units" (doesn't apply to some devices, mostly CPUs).

When using multiple devices, make sure they have nearly the same performances. One slower device in the middle
of more powerfull ones can be bottleneck.