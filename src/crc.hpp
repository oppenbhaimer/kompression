#include <cstddef>
#include <cinttypes>

uint32_t crc_32(uint32_t P, char* M, size_t bufsize, uint32_t v);
uint32_t crc32_fast(const char *s, size_t n);
