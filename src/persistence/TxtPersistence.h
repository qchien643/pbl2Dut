#pragma once

#include "persistence/Mapping.h"

#include <filesystem>
#include <fstream>
#include <locale>
#include <sstream>

namespace txt_persistence {

void validateUtf8(const std::string& text);
std::vector<std::string> splitFields(const std::string& line);

template <typename RepositoryType, typename MappingType>
void load(const std::filesystem::path& path, RepositoryType& repository, const MappingType& modelMapping) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error(path.string() + ": Cannot open for reading");
    }
    ColumnPositions positions;
    std::size_t lineNumber = 0;
    std::string line;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (lineNumber == 1 && line.compare(0, 3, "\xef\xbb\xbf") == 0) {
            line.erase(0, 3);
        }
        try {
            validateUtf8(line);
            const std::vector<std::string> fields = splitFields(line);
            if (lineNumber == 1) {
                positions = modelMapping.bindHeader(fields);
                continue;
            }
            if (fields.size() != positions.size()) {
                throw std::invalid_argument("Incorrect field count");
            }
            if (!repository.insert(modelMapping.readRow(fields, positions))) {
                throw std::invalid_argument("Duplicate primary key");
            }
        } catch (const std::bad_alloc&) {
            throw;
        } catch (const std::exception& error) {
            throw std::runtime_error(path.string() + ":" + std::to_string(lineNumber) + ": " + error.what());
        }
    }
    if (input.bad() || !input.eof()) {
        throw std::runtime_error(path.string() + ": Read failed");
    }
    if (lineNumber == 0) {
        throw std::runtime_error(path.string() + ":1: Missing header");
    }
}

template <typename RepositoryType, typename MappingType>
void save(const std::filesystem::path& path, const RepositoryType& repository, const MappingType& modelMapping) {
    using T = typename RepositoryType::value_type;
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error(path.string() + ": Cannot open for writing");
    }
    modelMapping.writeHeader(output);
    output << '\n';
    std::size_t lineNumber = 1;
    for (const T& value : repository) {
        ++lineNumber;
        try {
            std::ostringstream row;
            row.exceptions(std::ios::badbit | std::ios::failbit);
            row.imbue(std::locale::classic());
            modelMapping.writeRow(row, value);
            const std::string text = row.str();
            validateUtf8(text);
            output << text << '\n';
            if (!output) {
                throw std::runtime_error("Write failed");
            }
        } catch (const std::bad_alloc&) {
            throw;
        } catch (const std::exception& error) {
            throw std::runtime_error(path.string() + ":" + std::to_string(lineNumber) + ": " + error.what());
        }
    }
    output.flush();
    if (!output) {
        throw std::runtime_error(path.string() + ": Flush failed");
    }
    output.close();
    if (output.fail()) {
        throw std::runtime_error(path.string() + ": Close failed");
    }
}

}
