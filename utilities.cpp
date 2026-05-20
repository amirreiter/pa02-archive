#include "utilities.h"

#include <string>
#include <cstdio>

// reads a file into a giant string, by using C i/o methods
std::string read_file_to_string(const std::string& path) {
    auto fp = std::fopen(path.c_str(), "rb");
    if (!fp) {
        return "";
    }

    // size
    std::fseek(fp, 0, SEEK_END);
    const auto size = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);

    // allocation
    std::string file_buffer;
    file_buffer.resize(size);

    // reading
    size_t x = std::fread(&file_buffer[0], 1, size, fp);
    std::fclose(fp);

    return file_buffer;
}
