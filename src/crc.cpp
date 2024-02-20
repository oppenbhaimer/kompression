#include <cstddef>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "crc.hpp"

uint32_t crc_32(uint32_t P, char* M, size_t bufsize, uint32_t v) {

    std::cout << M << " " << bufsize << std::endl;
    uint32_t crc_table[256];
    // uint64_t P64 = P;

    for (uint32_t b=0; b<256; b++) {
        uint32_t rem = b;
        for (int i=7; i>=0; i--) {
            rem = (rem>>1) ^ ((rem&1)*P);
        }
        crc_table[(size_t)b] = rem;
    }

    std::cout << std::hex << std::setfill('0') << std::setw(8) << crc_table[0] << " " << crc_table[1] << " " << crc_table[2] << " " << crc_table[3] << std::dec << std::endl;

    // let's not use v now
    // uint32_t reg = v;
    // we also don't care about where the message is put, so append zeros to 
    // the end of M
    // char *buf = new char[bufsize+4];
    // buf[bufsize] = buf[bufsize+1] = buf[bufsize+2] = buf[bufsize+3] = 0;
    // memcpy(buf, M, bufsize);
    uint32_t reg = 0;

    for (int i=0; i<bufsize; i++) {
        reg = (reg<<8) ^ crc_table[(reg>>24) ^ M[i]];
        // size_t top = reg & 0xFF;
        // reg = (reg>>8) | M[i];
        // reg ^= crc_table[top];
    }
    // for (int i=0; i<4; i++) {
    //     reg = ((reg << 8)&0xFFFFFF00UL) | crc_table[(size_t)((reg>>24)&0xFF)];
    // }

    // delete[] buf;
    // uint32_t rs = (reg<<24) | ((reg<<8)&0xFF0000) | ((reg>>8)&0xFF00) | (reg>>24);
    return reg;
}
