/*
	GPU plot generator for Burst coin.
	Author: Cryo
	Bitcoin: 138gMBhCrNkbaiTCmUhP9HLU9xwn5QKZgD
	Burst: BURST-YA29-QCEW-QXC3-BKXDL

	Based on the code of the official miner and dcct's plotgen.
*/

#ifndef UTIL_CL
#define UTIL_CL

void memcpy(unsigned char* p_from, unsigned int p_fromOffset, unsigned char* p_to, unsigned int p_toOffset, unsigned int p_length) {
	for(unsigned int i = 0 ; i < p_length ; ++i) {
		p_to[p_toOffset + i] = p_from[p_fromOffset + i];
	}
}

void memcpyFromGlobal(__global unsigned char* p_from, unsigned int p_fromOffset, unsigned char* p_to, unsigned int p_toOffset, unsigned int p_length) {
	for(unsigned int i = 0 ; i < p_length ; ++i) {
		p_to[p_toOffset + i] = p_from[p_fromOffset + i];
	}
}

void memcpyToGlobal(unsigned char* p_from, unsigned int p_fromOffset, __global unsigned char* p_to, unsigned int p_toOffset, unsigned int p_length) {
	for(unsigned int i = 0 ; i < p_length ; ++i) {
		p_to[p_toOffset + i] = p_from[p_fromOffset + i];
	}
}

void memcpyGlobal(__global unsigned char* p_from, unsigned int p_fromOffset, __global unsigned char* p_to, unsigned int p_toOffset, unsigned int p_length) {
	for(unsigned int i = 0 ; i < p_length ; ++i) {
		p_to[p_toOffset + i] = p_from[p_fromOffset + i];
	}
}

unsigned int decodeIntLE(unsigned char* p_buffer, unsigned int p_offset) {
	return
		p_buffer[p_offset] |
		(p_buffer[p_offset + 1] << 8) |
		(p_buffer[p_offset + 2] << 16) |
		(p_buffer[p_offset + 3] << 24);
}

void encodeIntLE(unsigned char* p_buffer, unsigned int p_offset, unsigned int p_value) {
	p_buffer[p_offset] = p_value & 0x0ff;
	p_buffer[p_offset + 1] = (p_value >> 8) & 0x0ff;
	p_buffer[p_offset + 2] = (p_value >> 16) & 0x0ff;
	p_buffer[p_offset + 3] = (p_value >> 24) & 0x0ff;
}

unsigned long decodeLongLE(unsigned char* p_buffer, unsigned int p_offset) {
	return
		p_buffer[p_offset] |
		((unsigned long)p_buffer[p_offset + 1] << 8) |
		((unsigned long)p_buffer[p_offset + 2] << 16) |
		((unsigned long)p_buffer[p_offset + 3] << 24) |
		((unsigned long)p_buffer[p_offset + 4] << 32) |
		((unsigned long)p_buffer[p_offset + 5] << 40) |
		((unsigned long)p_buffer[p_offset + 6] << 48) |
		((unsigned long)p_buffer[p_offset + 7] << 56);
}

void encodeLongLE(unsigned char* p_buffer, unsigned int p_offset, unsigned long p_value) {
	p_buffer[p_offset] = p_value & 0x0ff;
	p_buffer[p_offset + 1] = (p_value >> 8) & 0x0ff;
	p_buffer[p_offset + 2] = (p_value >> 16) & 0x0ff;
	p_buffer[p_offset + 3] = (p_value >> 24) & 0x0ff;
	p_buffer[p_offset + 4] = (p_value >> 32) & 0x0ff;
	p_buffer[p_offset + 5] = (p_value >> 40) & 0x0ff;
	p_buffer[p_offset + 6] = (p_value >> 48) & 0x0ff;
	p_buffer[p_offset + 7] = (p_value >> 56) & 0x0ff;
}

unsigned long decodeIntBEGlobal(__global unsigned char* p_buffer, unsigned int p_offset) {
	return
		(p_buffer[p_offset] << 24) |
		(p_buffer[p_offset + 1] << 16) |
		(p_buffer[p_offset + 2] << 8) |
		p_buffer[p_offset + 3];
}

void encodeIntBEGlobal(__global unsigned char* p_buffer, unsigned int p_offset, unsigned long p_value) {
	p_buffer[p_offset] = (p_value >> 24) & 0x0ff;
	p_buffer[p_offset + 1] = (p_value >> 16) & 0x0ff;
	p_buffer[p_offset + 2] = (p_value >> 8) & 0x0ff;
	p_buffer[p_offset + 3] = p_value & 0x0ff;
}

unsigned long decodeLongBE(unsigned char* p_buffer, unsigned int p_offset) {
	return
		((unsigned long)p_buffer[p_offset] << 56) |
		((unsigned long)p_buffer[p_offset + 1] << 48) |
		((unsigned long)p_buffer[p_offset + 2] << 40) |
		((unsigned long)p_buffer[p_offset + 3] << 32) |
		((unsigned long)p_buffer[p_offset + 4] << 24) |
		((unsigned long)p_buffer[p_offset + 5] << 16) |
		((unsigned long)p_buffer[p_offset + 6] << 8) |
		p_buffer[p_offset + 7];
}

void encodeLongBE(unsigned char* p_buffer, unsigned int p_offset, unsigned long p_value) {
	p_buffer[p_offset] = (p_value >> 56) & 0x0ff;
	p_buffer[p_offset + 1] = (p_value >> 48) & 0x0ff;
	p_buffer[p_offset + 2] = (p_value >> 40) & 0x0ff;
	p_buffer[p_offset + 3] = (p_value >> 32) & 0x0ff;
	p_buffer[p_offset + 4] = (p_value >> 24) & 0x0ff;
	p_buffer[p_offset + 5] = (p_value >> 16) & 0x0ff;
	p_buffer[p_offset + 6] = (p_value >> 8) & 0x0ff;
	p_buffer[p_offset + 7] = p_value & 0x0ff;
}

void encodeLongBEGlobal(__global unsigned char* p_buffer, unsigned int p_offset, unsigned long p_value) {
	p_buffer[p_offset] = (p_value >> 56) & 0x0ff;
	p_buffer[p_offset + 1] = (p_value >> 48) & 0x0ff;
	p_buffer[p_offset + 2] = (p_value >> 40) & 0x0ff;
	p_buffer[p_offset + 3] = (p_value >> 32) & 0x0ff;
	p_buffer[p_offset + 4] = (p_value >> 24) & 0x0ff;
	p_buffer[p_offset + 5] = (p_value >> 16) & 0x0ff;
	p_buffer[p_offset + 6] = (p_value >> 8) & 0x0ff;
	p_buffer[p_offset + 7] = p_value & 0x0ff;
}

#endif