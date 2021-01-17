#ifndef NSW_MEMFILEREADER_H
#define NSW_MEMFILEREADER_H

#include <cstddef>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

/*
* Immutable representation of an FPGA MEM file.
* Holds data to be written to the board's BRAM
* Possible improvements:
* Templatize in order to match BRAM width (std::bitset?), rather than just uint32_t for holding data
* Refactor into parser class, or separate into parser and data-holder classes
* Question: are the memfiles (RAM blocks) of different widths possible? How does this affect the user of this class?
*/

class MemfileReader {
public:
    MemfileReader(const std::string& path, const char delimiter);
    MemfileReader(const MemfileReader&) = delete;
    // Memfile(Memfile&&) = default;

    const std::map<uint16_t, std::vector<uint32_t>>& read() const;
private:
    std::ifstream m_file;
    char m_delimiter;
    std::map<uint16_t, std::vector<uint32_t>> m_memory;

    void parse();
};

#endif // NSW_MEMFILEREADER_H