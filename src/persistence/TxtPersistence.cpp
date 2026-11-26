#include "TxtPersistence.h"

#include <algorithm>
#include <charconv>
#include <locale>
#include <new>
#include <stdexcept>

namespace txt_persistence {

void validateUtf8(const std::string& text) {
    for (std::size_t i = 0; i < text.size();) {
        const unsigned char first = static_cast<unsigned char>(text[i++]);
        if (first < 0x80) {
            continue;
        }
        int remaining = -1;
        if (first >= 0xc2 && first <= 0xdf) {
            remaining = 1;
        } else if (first >= 0xe0 && first <= 0xef) {
            remaining = 2;
        } else if (first >= 0xf0 && first <= 0xf4) {
            remaining = 3;
        }
        if (remaining < 0 || text.size() - i < static_cast<std::size_t>(remaining)) {
            throw std::invalid_argument("Invalid UTF-8");
        }
        std::uint32_t code = first & ((1u << (6 - remaining)) - 1);
        for (int j = 0; j < remaining; ++j) {
            const unsigned char byte = static_cast<unsigned char>(text[i++]);
            if ((byte & 0xc0) != 0x80) {
                throw std::invalid_argument("Invalid UTF-8 continuation");
            }
            code = (code << 6) | (byte & 0x3f);
        }
        if ((remaining == 1 && code < 0x80) || (remaining == 2 && code < 0x800)
            || (remaining == 3 && code < 0x10000) || code > 0x10ffff
            || (code >= 0xd800 && code <= 0xdfff)) {
            throw std::invalid_argument("Invalid UTF-8 code point");
        }
    }
}

std::int64_t number(const std::string& field) {
    std::int64_t result = 0;
    const std::from_chars_result parsed = std::from_chars(field.data(), field.data() + field.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != field.data() + field.size()) {
        throw std::invalid_argument("Expected a complete int64 field");
    }
    return result;
}

std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = line.find('|', start);
        fields.push_back(line.substr(start, end == std::string::npos ? end : end - start));
        if (end == std::string::npos) {
            return fields;
        }
        start = end + 1;
    }
}

std::vector<Row> readTable(const std::filesystem::path& path, const std::string& expectedHeader) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error(path.string() + ": Cannot open for reading");
    }
    const std::vector<std::string> expected = splitFields(expectedHeader);
    std::vector<std::string> header;
    // ponytail: buffer one text table during startup; stream rows if it no longer fits RAM.
    std::vector<Row> rows;
    std::string line;
    std::size_t lineNumber = 0;
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
                if (fields.size() != expected.size()) {
                    throw std::invalid_argument("Incorrect header: missing or extra columns");
                }
                for (std::size_t i = 0; i < fields.size(); ++i) {
                    if (std::find(expected.begin(), expected.end(), fields[i]) == expected.end()
                        || std::find(fields.begin(), fields.begin() + i, fields[i]) != fields.begin() + i) {
                        throw std::invalid_argument("Incorrect header: unknown or duplicate column");
                    }
                }
                header = fields;
                continue;
            }
            if (fields.size() != header.size()) {
                throw std::invalid_argument("Incorrect field count");
            }
            Row row;
            for (std::size_t i = 0; i < header.size(); ++i) {
                row.emplace(header[i], fields[i]);
            }
            rows.push_back(row);
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
    return rows;
}

std::ofstream openOutput(const std::filesystem::path& path, const std::string& header) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error(path.string() + ": Cannot open for writing");
    }
    output.exceptions(std::ios::badbit | std::ios::failbit);
    output.imbue(std::locale::classic());
    output << header << '\n';
    return output;
}

}
