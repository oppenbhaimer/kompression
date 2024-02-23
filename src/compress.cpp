#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>

constexpr int kDebugLevel = 2;
constexpr int kMaxHuffCodeLength = 11;

#define CHECK(cond) do{if(!(cond)){fprintf(stderr,"%s:%d CHECK %s\n", __FILE__, __LINE__, #cond);exit(1);}}while(0);
#define LOGV(level, s, ...) do{if(level<=kDebugLevel) fprintf(stderr, s, ##__VA_ARGS__);}while(0);

std::string toBinary(int v, int size) {
  std::string result;
  for (int j = 0; j < size; ++j) {
    result += ((v>>(size-j-1))&1) ? "1" : "0";
  }
  return result;
}

int log2(int v) {
  if (v > 0) {
    return 31 - __builtin_clz(v);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// CRC table
////////////////////////////////////////////////////////////////////////////////

const uint32_t crc32_table[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t crc32(uint8_t* buf, size_t len) {
    uint32_t reg = 0xFFFFFFFFUL;
    while (len-- > 0) {
        reg = (reg>>8) ^ crc32_table[(reg^(*buf++))&0xFF];
    }
    return ~reg;
}

////////////////////////////////////////////////////////////////////////////////
/// Bit Reading
////////////////////////////////////////////////////////////////////////////////

class BitReader {
    public:
    BitReader(uint8_t* buffer, uint8_t* end): _current(buffer), _end(end) {
        refill();
    }

    int readBit() {
        int r = _bits >> 31;
        _bits <<= 1;
        ++_position;
        return r;
    }

    int readBits(int n) {
        int r = (_bits >> 1) >> (31 - n);
        _bits <<= n;
        _position += n;
        return r;
    }

    void refill() {
        while (_position >= 0) {
            _bits |= (_current < _end ? *_current : 0) << _position;
            _position -= 8;
            ++_current;
        }
    }

    void byteAlign() {
        int extra_bits = _position & 0x7;
        if (extra_bits) {
           readBits(8 - extra_bits);
        }
    }

    uint32_t bits() const { return _bits; }
    uint8_t* cursor() const { return _current - ((24 - _position) / 8); }
    uint8_t* end() const {return _end; }

    private:
    uint8_t* _current;
    uint8_t* _end;
    uint32_t _bits = 0;
    int _position = 24;
};

class BitWriter {
    public:
    BitWriter(uint8_t* buffer): _start(buffer), _current(buffer) {}

    void writeBit(int v) {
        _bits = (_bits << 1) | v;
        ++_position;
        if (_position >= 8) flush();
    }

    void writeBits(int v, int n) {
        _bits = (_bits << n) | v;
        _position += n;
        if (_position >= 8) flush();
    }

    size_t finish() {
        flush();
        if (_position > 0) {
            // Final byte is a bit tricky.  Handle it specially.
            *_current = (_bits & ((1 << _position) - 1)) << (8 - _position);
            ++_current;
            _position = 0;
        }
        return (_current - _start);
    }

    private:
    void flush() {
        while (_position >= 8) {
            _position -= 8;
            *_current = (_bits >> _position) & 0xFF;
            ++_current;
        }
    }

    uint8_t* _start;
    uint8_t* _current;
    uint32_t _bits = 0;
    int _position = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// Huffman Encoder 
////////////////////////////////////////////////////////////////////////////////

struct Node {
    int freq;
    int symbol;
    Node* l;
    Node* r;
};

struct Comparator {
    bool operator()(const Node* l, const Node* r) {
        return l->freq > r->freq;
    }
};

class HuffmanEncoder {

    public:

    HuffmanEncoder(BitWriter& writer): _writer(writer) {
        for (int i = 0; i < _max_symbols; ++i) {
            _nodes[i].symbol = i;
            _nodes[i].freq = 0;
        }
    }

    void walk(Node* n, int level) {
        if (n->symbol != -1) {
            // reuse freq to mean distance from root
            n->freq = level;
            return;
        }

        walk(n->l, level+1);
        walk(n->r, level+1);
    }

    // heuristic-based length limiting. Perfect length limited coding would use 
    // the package merge algorithm
    // TODO implement package merge as an exercise
    void limitLength(int num_symbols) {
        int k = 0;
        int maxk = (1 << kMaxHuffCodeLength) - 1;
        // truncate code
        for (int i = num_symbols-1; i>=0; i--) {
            _nodes[i].freq = std::min(_nodes[i].freq, kMaxHuffCodeLength);
            k += 1 << (kMaxHuffCodeLength - _nodes[i].freq);
        }

        // fixup - first pass
        for (int i=num_symbols-1; i>=0 && k>maxk; i--) {
            while (_nodes[i].freq < kMaxHuffCodeLength) {
                _nodes[i].freq++;
                k -= 1 << (kMaxHuffCodeLength - _nodes[i].freq);
            }
        }

        // fixup - second pass (reduce error)
        for (int i=0; i<num_symbols; i++) {
            while (k + (1 << (kMaxHuffCodeLength - _nodes[i].freq)) <= maxk) {
                k += 1 << (kMaxHuffCodeLength - _nodes[i].freq);
                --_nodes[i].freq;
            }
        }
    }

    void writeTable(int num_symbols) {
        const int kSymBits = log2(_max_symbols);
        _writer.writeBits(num_symbols-1, kSymBits);

        for (int i=0; i<num_symbols; i++) {
            Node symbol = _nodes[i];
            _writer.writeBits(symbol.symbol, kSymBits);
            _writer.writeBits(symbol.freq-1, 4);
        }

        _writer.finish();
    }

    void buildCodes(int num_symbols) {
        int code = 0;
        int last_level = -1;

        for (int i=0; i<num_symbols; i++) {
            int level = _nodes[i].freq;
            if (last_level != level) {
                if (last_level != -1) {
                    code++;
                    code <<= (level - last_level);
                }
                last_level = level;
            }
            else {
                code++;
            }

            int symbol = _nodes[i].symbol;
            _length[symbol] = level;
            _code[symbol] = code;

            LOGV(2, "code:%s hex:%x level:%d symbol:%d\n", 
                    toBinary(code, level).c_str(), code, level, symbol)
        }

    }

    void buildTable() {
        Node* q[256];
        int num_symbols = 0;
        for (int i=0; i<_max_symbols; i++) {
            if (_nodes[i].freq > 0) {
                _nodes[num_symbols] = _nodes[i];
                q[num_symbols] = &_nodes[num_symbols];
                num_symbols++;
            }
        }

        Comparator c;
        std::make_heap(&q[0], &q[num_symbols], c); // slightly faster than std::priority_queue

        for (int i=num_symbols; i>1; i--) {
            Node* n1 = q[0];
            std::pop_heap(&q[0], &q[i], c);
            Node* n2 = q[0];
            std::pop_heap(&q[0], &q[i-1], c);

            Node* parent = &_nodes[num_symbols+i];
            parent->freq = n1->freq + n2->freq;
            parent->symbol = -1;
            parent->l = n2;
            parent->r = n1;
            q[i-2] = parent;
            std::push_heap(&q[0], &q[i-1], c);
        }

        walk(q[0], num_symbols == 1 ? 1 : 0);

        std::sort(&_nodes[0], &_nodes[num_symbols], 
                  [](const Node& l, const Node& r) { return l.freq < r.freq; });

        limitLength(num_symbols);
        writeTable(num_symbols);
        buildCodes(num_symbols);
    }

    void encode(int symbol) {
        _writer.writeBits(_code[symbol], _length[symbol]);
    }

    void scan(int symbol) {
        _nodes[symbol].freq++;
    }

    private:
    BitWriter& _writer;
    Node _nodes[512];
    uint8_t _length[256];
    int _code[256];
    const int _max_symbols = 256;

};

class HuffmanDecoder {
    public:

    HuffmanDecoder(BitReader& reader): _reader(reader) {}

    void readTable() {
        _reader.refill();
        _num_symbols = _reader.readBits(_sym_bits);

        for (int i=0; i<_num_symbols; i++) {
            _reader.refill();
            int symbol = _reader.readBits(_sym_bits);
            int codelen = _reader.readBits(4) + 1;
            LOGV(2, "sym:%d len:%d\n", symbol, codelen);

            _codelen_count[codelen]++;
            _symbol_length[i] = codelen;
            _symbol[i] = symbol;
            _min_codelen = std::min(_min_codelen, codelen);
            _max_codelen = std::max(_max_codelen, codelen);
        }

        LOGV(1, "num_sym %d codelen(min:%d max:%d)\n",
                _num_symbols, _min_codelen, _max_codelen);
        
        _reader.byteAlign();
    }

    uint8_t decodeOne() {
        _reader.refill();
        int n = _reader.bits() >> (32 - _max_codelen);
        int len = _bits_to_len[n];
        _reader.readBits(len);
        return _bits_to_sym[n];
    }

    void decode(uint8_t* output, uint8_t* output_end) {
        uint8_t* src = _reader.cursor();
        uint8_t* src_end = _reader.end();
        int position = 24;
        uint32_t bits = 0;

        while (true) {
            while (position >= 0) {
                bits |= (src < src_end ? *src++ : 0) << position;
                position -= 8;
            }
            int n = bits >> (32 - _max_codelen);
            int len = _bits_to_len[n];
            *output++ = _bits_to_sym[n];
            if (output >= output_end) {
                break;
            }
            bits <<= len;
            position += len;
        }
    }

    // read canonical table
    void assignCodes() {
        int p = 0;
        uint8_t* cursym = &_symbol[0];
        for (int i=_min_codelen; i <= _max_codelen; i++) {
            int n = _codelen_count[i];
            if (n) {
                int shift = _max_codelen - i;
                memset(_bits_to_len + p, i, n << shift);
                int m = 1 << shift;
                do {
                    memset(_bits_to_sym + p, *cursym++, m);
                    p += m;
                } while (--n);
            }
        }
    }

    private:
    BitReader& _reader;
    int _num_symbols;
    int _sym_bits;
    int _min_codelen, _max_codelen;
    int _codelen_count[kMaxHuffCodeLength];
    uint8_t _bits_to_len[512];
    uint8_t _bits_to_sym[512];
    uint8_t _symbol_length[512];
    uint8_t _symbol[512];
};

std::vector<uint8_t> read_file_into_buf(const std::string& fileName) {
    std::ifstream fileStream(fileName, std::ios::binary | std::ios::ate);  // Open the file in binary mode and set the position to end.

    if (!fileStream) {
        throw std::runtime_error("Error opening file: " + fileName);
    }
    
    auto size = static_cast<std::streamsize>(fileStream.tellg());   // Get current position (end of the file)
    std::vector<uint8_t> buffer(static_cast<size_t>(size));    // Create a vector to hold all bytes.
    
    fileStream.seekg(0);                          // Set position back to beginning.

    if (!fileStream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()))) {
        throw std::runtime_error("Error reading from file: " + fileName);
    }

    return buffer;
}

void write_buf_into_file(std::vector<uint8_t>& buf, std::string outfile) {
    std::ofstream fileStream(outfile, std::ios::binary);  // Open the file in binary mode.

    if (!fileStream) {
        throw std::runtime_error("Error opening file: " + outfile);
    }

    fileStream.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(buf.size()));  // Write data from buffer to file.

    if (fileStream.bad()) {
        throw std::runtime_error("Error writing to file: " + outfile);
    }
}

void test_huffman_encode_file(std::string filename, std::string outfile) {

    std::vector<uint8_t> buf = read_file_into_buf(filename);
    BitReader br(buf.data(), buf.data()+buf.size());
    // uint8_t* outbuf = new uint8_t[buf.size()];
    std::vector<uint8_t> outbuf(buf.size());
    BitWriter bw(outbuf.data());
    HuffmanEncoder enc(bw);

    for (int i=0; i<buf.size(); i++) {
        enc.scan(buf[i]);
    }
    auto tic = std::chrono::high_resolution_clock::now();
    enc.buildTable();
    for (int i=0; i<buf.size(); i++) {
        enc.encode(buf[i]);
    }
    auto toc = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(toc-tic);

    int n_bytes = bw.finish();
    outbuf.resize(n_bytes);
    write_buf_into_file(outbuf, outfile);

    std::cout << "Compressed " << buf.size() << " bytes to " << n_bytes << " bytes in " << duration.count() << " ms\n";
}

void test_huffman_decode_file(std::string filename, std::string outfile) {

    std::vector<uint8_t> buf = read_file_into_buf(filename);
    BitReader br(buf.data(), buf.data()+buf.size());
    HuffmanDecoder dec(br);
    std::vector<uint8_t> output(buf.size()*1.5);

    auto tic = std::chrono::high_resolution_clock::now();
    dec.readTable();
    dec.assignCodes();
    dec.decode(output.data(), output.data()+output.size());
    auto toc = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(toc-tic);

    write_buf_into_file(output, outfile);

    std::cout << "Decompressed" << buf.size() << " bytes in " << duration.count() << " ms\n";
}

void test_huffman_encode() {
    std::string buf = "AAAAAAAABBBBBBBCCCCD";
    BitReader br((uint8_t*)buf.data(), (uint8_t*)(buf.data()+buf.size()));
    uint8_t* outbuf = new uint8_t[1024];
    BitWriter bw(outbuf);
    HuffmanEncoder enc(bw);

    for (int i=0; i<buf.size(); i++) {
        enc.scan(buf[i]);
    }
    enc.buildTable();
    for (int i=0; i<buf.size(); i++) {
        enc.encode(buf[i]);
    }

    delete[] outbuf;
}

int main(int argc, char** argv) {

    test_huffman_decode_file(argv[1], argv[2]);

    return 0;
}
