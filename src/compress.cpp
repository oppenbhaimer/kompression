#include <iostream>
#include "crc.hpp"

void test_crc32() {
    std::string s = "123456789";
    uint32_t crc32 = crc_32(0xEDB88320UL, s.data(), (size_t)s.size(), 0xFFFFFFFFUL);
    //uint32_t crc32 = crc32_fast(s.data(), (size_t)s.size());
    std::cout << std::hex << crc32 << std::dec << std::endl;
}

void compress(char* srcpath, char* dstpath) {

    std::cout << srcpath << " " << dstpath << std::endl;
}

int main(int argc, char** argv) {

    test_crc32();

    return 0;
}
