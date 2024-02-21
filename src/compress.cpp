#include <iostream>
#include <cstring>
#include "crc.hpp"

void test_crc32(char* s) {
    uint32_t crc32 = crc_32(0x04C11DB7UL, s, strlen(s), 0xFFFFFFFFUL);
    //uint32_t crc32 = crc32_fast(s.data(), (size_t)s.size());
    std::cout << std::hex << crc32 << std::dec << std::endl;

}

void compress(char* srcpath, char* dstpath) {

    std::cout << srcpath << " " << dstpath << std::endl;
}

int main(int argc, char** argv) {

    test_crc32(argv[1]);

    return 0;
}
