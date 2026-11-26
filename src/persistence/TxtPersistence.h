#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace txt_persistence {

using Row = std::map<std::string, std::string>;

void validateUtf8(const std::string& text);
std::int64_t number(const std::string& field);
std::vector<std::string> splitFields(const std::string& line);

// Rows contain temporary text, not the entities owned by repositories.
std::vector<Row> readTable(const std::filesystem::path& path, const std::string& expectedHeader);

// Used only for staging files. Exceptions report open/write/flush/close failures.
std::ofstream openOutput(const std::filesystem::path& path, const std::string& header);

}
