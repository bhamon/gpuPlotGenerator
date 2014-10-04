/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#include "util.cl"
#include "shabal.cl"

#define HASH_SIZE			32
#define HASHES_PER_SCOOP	2
#define SCOOP_SIZE			(HASHES_PER_SCOOP * HASH_SIZE)
#define SCOOPS_PER_PLOT		4096
#define PLOT_SIZE			(SCOOPS_PER_PLOT * SCOOP_SIZE)
#define HASH_CAP			4096
#define GEN_SIZE			(PLOT_SIZE + 16)

__kernel void nonce_step1(__global unsigned char* p_buffer, unsigned int p_size, unsigned long p_address, unsigned long p_startNonce) {
	size_t id = get_global_id(0);
	if(id >= p_size) {
		return;
	}

	unsigned long currentNonce = p_startNonce + id;
	unsigned int genOffset = GEN_SIZE * id;

	encodeLongBEGlobal(p_buffer, genOffset + PLOT_SIZE, p_address);
	encodeLongBEGlobal(p_buffer, genOffset + PLOT_SIZE + 8, currentNonce);
}

__kernel void nonce_step2(__global unsigned char* p_buffer, unsigned int p_size, unsigned long p_startNonce, unsigned int p_hashesOffset, unsigned int p_hashesNumber) {
	size_t id = get_global_id(0);
	if(id >= p_size) {
		return;
	}

	unsigned int genOffset = GEN_SIZE * id;
	unsigned char hash[HASH_SIZE];

	if(p_hashesNumber * HASH_SIZE > p_hashesOffset) {
		p_hashesNumber = p_hashesOffset / HASH_SIZE;
	}

	unsigned int len = GEN_SIZE - p_hashesOffset;
	if(len > HASH_CAP) {
		len = HASH_CAP;
	}

	shabal_context_t context;
	for(unsigned int i = 0 ; i < p_hashesNumber ; ++i) {
		shabal_init(&context);
		shabal_update(&context, p_buffer, genOffset + p_hashesOffset, len);
		shabal_digest(&context, hash, 0);

		barrier(CLK_LOCAL_MEM_FENCE);
		memcpyToGlobal(hash, 0, p_buffer, genOffset + p_hashesOffset - HASH_SIZE, HASH_SIZE);

		p_hashesOffset -= HASH_SIZE;
		len += HASH_SIZE;
		len &= ((len >> 12) * 48) ^ 8191;
	}
}

__kernel void nonce_step3(__global unsigned char* p_buffer, unsigned int p_size) {
	size_t id = get_global_id(0);
	if(id >= p_size) {
		return;
	}

	unsigned int genOffset = GEN_SIZE * id;
	unsigned char hash[HASH_SIZE];

	shabal_context_t context;
	shabal_init(&context);
	shabal_update(&context, p_buffer, genOffset, GEN_SIZE);
	shabal_digest(&context, hash, 0);

	barrier(CLK_LOCAL_MEM_FENCE);

	unsigned int len = PLOT_SIZE;
	for(unsigned int i = 0 ; i < len ; ++i) {
		p_buffer[genOffset + i] ^= hash[i % HASH_SIZE];
	}
}