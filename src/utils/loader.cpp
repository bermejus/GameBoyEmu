#include "loader.hpp"

#include <fstream>
#include <iterator>

std::vector<ubyte> read_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    file.unsetf(std::ios::skipws);

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<ubyte> res;
    res.reserve(size);

    std::copy(std::istream_iterator<ubyte>(file),
              std::istream_iterator<ubyte>(),
              std::back_inserter(res));
    
    return res;
}