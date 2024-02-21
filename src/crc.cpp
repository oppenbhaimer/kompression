#include <cstddef>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "crc.hpp"

uint32_t crc32_table[256];

void build_crc32_table(void) {
	for(uint32_t i=0;i<256;i++) {
		uint32_t ch=i;
		uint32_t crc=0;
		for(size_t j=0;j<8;j++) {
			uint32_t b=(ch^crc)&1;
			crc>>=1;
			if(b) crc=crc^0xEDB88320;
			ch>>=1;
		}
		crc32_table[i]=crc;
	}
}

uint32_t crc32_fast(const char *s,size_t n) {
    build_crc32_table();
	uint32_t crc=0xFFFFFFFF;
	
	for(size_t i=0;i<n;i++) {
		char ch=s[i];
		uint32_t t=(ch^crc)&0xFF;
		crc=(crc>>8)^crc32_table[t];
	}
	
	return ~crc;
}

uint32_t reverse(uint32_t x, int bits)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1); // Swap _<>_
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2); // Swap __<>__
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4); // Swap ____<>____
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8); // Swap ...
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16); // Swap ...
    return x >> (32 - bits);
}

uint32_t crc_32(uint32_t P, char* M, size_t bufsize, uint32_t v) {

    std::cout << M << " " << bufsize << std::endl;
    uint32_t crc_table[256];
    // uint64_t P64 = P;

    for (uint32_t b=0; b<256; b++) {
        uint32_t rem = (b<<24);
        for (int i=7; i>=0; i--) {
            if (rem & (1<<31)) {
                rem = (rem<<1) ^ P;
            }
            else {
                rem = (rem<<1);
            }
        }
        crc_table[(size_t)b] = rem;
    }

    std::cout << std::hex;
    for (int i=0; i<64; i++) {
        for (int j=0; j<4; j++) {
            std::cout << std::setfill('0') << std::setw(8) << crc_table[i*4 + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::dec;
    // std::cout << std::hex << std::setfill('0') << std::setw(8) << crc_table[0] << " " << crc_table[1] << " " << crc_table[2] << " " << crc_table[3] << std::dec << std::endl;

    // let's not use v now
    // uint32_t reg = v;
    // we also don't care about where the message is put, so append zeros to 
    // the end of M
    // char *buf = new char[bufsize+4];
    // buf[bufsize] = buf[bufsize+1] = buf[bufsize+2] = buf[bufsize+3] = 0;
    // memcpy(buf, M, bufsize);
    uint32_t reg = -1; // 0xFFFFFFFFUL;
    std::cout << std::hex << reg << std::endl;

    for (int i=0; i<bufsize; i++) {
        uint32_t t = (reg>>24);
        // uint32_t val = reverse(M[i], 8);
        reg = (reg<<8) | M[i];
        reg ^= crc_table[t];
        // size_t top = reg & 0xFF;
        // reg = (reg>>8) | M[i];
        // reg ^= crc_table[top];
    }
    // reg ^= 0xFFFFFFFFUL;
    for (int i=0; i<4; i++) {
        reg = (reg<<8) ^ crc_table[(size_t)(reg>>24)];
    }

    // delete[] buf;
    // uint32_t rs = (reg<<24) | ((reg<<8)&0xFF0000) | ((reg>>8)&0xFF00) | (reg>>24);
    // bit reflect
    // return reverse(reg, 32)^0xFFFFFFFFUL;
    return reg;
}
