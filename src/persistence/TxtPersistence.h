#pragma once

#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "repositories/TicketRepository.h"

#include <filesystem>
#include <fstream>
#include <locale>
#include <sstream>

namespace txt_persistence {

void validateUtf8(const std::string& text);
std::int64_t number(const std::string& field);
const char* header(const Film*);
const char* header(const Auditorium*);
const char* header(const Seat*);
const char* header(const Screening*);
const char* header(const Ticket*);
Film parse(const std::string* f, const Film*);
Auditorium parse(const std::string* f, const Auditorium*);
void writeRecord(std::ostream& out, const Film& value);
void writeRecord(std::ostream& out, const Auditorium& value);
void writeRecord(std::ostream& out, const Seat& value);
void writeRecord(std::ostream& out, const Screening& value);
void writeRecord(std::ostream& out, const Ticket& value);

template <typename RepositoryType, typename Parser>
void load(const std::filesystem::path& path, RepositoryType& repository, Parser parser) {
    using T = typename RepositoryType::value_type;
    const T* tag = nullptr;
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error(path.string() + ": Cannot open for reading");
    }
    const std::string expected = header(tag);
    std::size_t fieldCount = 1;
    for (char character : expected) {
        fieldCount += character == '|';
    }
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
            if (lineNumber == 1) {
                if (line != expected) {
                    throw std::invalid_argument("Incorrect header");
                }
                continue;
            }
            std::string fields[5];
            std::size_t start = 0;
            for (std::size_t i = 0; i < fieldCount; ++i) {
                const std::size_t end = line.find('|', start);
                if ((i + 1 == fieldCount) != (end == std::string::npos)) {
                    throw std::invalid_argument("Incorrect field count");
                }
                if (end == std::string::npos) {
                    fields[i] = line.substr(start);
                } else {
                    fields[i] = line.substr(start, end - start);
                    start = end + 1;
                }
            }
            if (!repository.insert(parser(fields))) {
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

template <typename RepositoryType>
void load(const std::filesystem::path& path, RepositoryType& repository) {
    using T = typename RepositoryType::value_type;
    const T* tag = nullptr;
    load(path, repository, [tag](const std::string* fields) {
        return parse(fields, tag);
    });
}

template <typename RepositoryType>
void save(const std::filesystem::path& path, const RepositoryType& repository) {
    using T = typename RepositoryType::value_type;
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error(path.string() + ": Cannot open for writing");
    }
    output << header(static_cast<const T*>(nullptr)) << '\n';
    std::size_t lineNumber = 1;
    for (const T& value : repository) {
        ++lineNumber;
        try {
            std::ostringstream row;
            row.exceptions(std::ios::badbit | std::ios::failbit);
            row.imbue(std::locale::classic());
            writeRecord(row, value);
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
