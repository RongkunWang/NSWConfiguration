#include "NSWConfiguration/MemfileReader.h"

MemfileReader::MemfileReader(const std::string& path, const char delimiter) : m_file(path), m_delimiter(delimiter) {
    m_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    parse();
}

const std::map<uint16_t, std::vector<uint32_t>>& MemfileReader::read() const {
    
    return m_memory;
}

void MemfileReader::parse() {
    if (m_file.eof()) {
        return;
    }

    auto extract_address = [](std::string str) -> uint16_t {
        std::size_t char_count{ 0 };
        uint32_t address = std::stoul(str.substr(1), &char_count, 16);
        if (char_count != str.size() - 1 || address > 0xFFFF) {
            throw std::runtime_error{ "Invalid address specifier: " + str };
        }
        return static_cast<uint16_t>(address);
    };

    std::string value_str;
    // Get initial address
    std::getline(m_file, value_str, m_delimiter);
    if (value_str.at(0) != '@') {
        throw std::runtime_error{ "Missing address specifier at beginning of file!" };
    }

    uint16_t address = extract_address(value_str);
    while(!m_file.eof() && std::getline(m_file, value_str, m_delimiter)) {
        if(value_str.at(0) == '@') {
            address = extract_address(value_str);
            m_memory.insert({ address, std::vector<uint32_t>{} });
            continue;
        }
        std::size_t chars_processed{};
        uint32_t value = std::stoul(value_str, &chars_processed, 16);
        if (chars_processed != value_str.size()) { 
            throw std::runtime_error{ "Invalid memory value specifier: " + value_str };
        }
        m_memory[address].push_back(value);
    }
}