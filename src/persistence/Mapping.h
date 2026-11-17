#pragma once

#include "repositories/IRepository.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace txt_persistence {

std::int64_t number(const std::string& field);

// Each column pairs its output getter with its input conversion rule.
template <typename Getter, typename Reader>
struct Column {
    const char* name;
    Getter getter;
    Reader read;
};

template <typename Getter>
auto integerColumn(const char* name, Getter getter) {
    return Column<Getter, decltype(&number)>{name, getter, number};
}

template <typename Getter>
auto textColumn(const char* name, Getter getter) {
    const auto reader = [](const std::string& field) -> const std::string& {
        return field;
    };
    return Column<Getter, decltype(reader)>{name, getter, reader};
}

template <typename Target, typename Getter>
auto referenceColumn(const char* name, Getter getter, const IRepository<Target>& repository) {
    const auto reader = [&repository, name](const std::string& field) -> const Target& {
        const EntityId id = number(field);
        if (id <= 0) {
            throw std::invalid_argument(std::string(name) + ": reference ID must be positive");
        }
        const Target* record = repository.findById(id);
        if (record == nullptr) {
            throw std::invalid_argument(std::string(name) + ": reference points to a missing record");
        }
        return *record;
    };
    return Column<Getter, decltype(reader)>{name, getter, reader};
}

using ColumnPositions = std::map<std::string, std::size_t>;

template <typename T, typename... Columns>
class Mapping {
public:
    explicit Mapping(Columns... columns) : columns_(std::move(columns)...) {}

    std::array<const char*, sizeof...(Columns)> columnNames() const {
        return std::apply([](const auto&... column) {
            return std::array<const char*, sizeof...(Columns)>{column.name...};
        }, columns_);
    }

    ColumnPositions bindHeader(const std::vector<std::string>& header) const {
        const auto names = columnNames();
        if (header.size() != names.size()) {
            throw std::invalid_argument("Incorrect header: missing or extra columns");
        }
        ColumnPositions positions;
        for (std::size_t index = 0; index < header.size(); ++index) {
            if (std::find(names.begin(), names.end(), header[index]) == names.end()
                || !positions.emplace(header[index], index).second) {
                throw std::invalid_argument("Incorrect header: unknown or duplicate column");
            }
        }
        return positions;
    }

    T readRow(const std::vector<std::string>& fields, const ColumnPositions& positions) const {
        // Mapping order is constructor argument order, not input file order.
        return std::apply([&](const auto&... column) {
            return T(column.read(fields.at(positions.at(column.name)))...);
        }, columns_);
    }

    void writeHeader(std::ostream& output) const {
        bool first = true;
        for (const char* name : columnNames()) {
            output << (first ? "" : "|") << name;
            first = false;
        }
    }

    void writeRow(std::ostream& output, const T& value) const {
        std::size_t index = 0;
        std::apply([&](const auto&... column) {
            ((output << (index++ == 0 ? "" : "|") << (value.*column.getter)()), ...);
        }, columns_);
    }

private:
    std::tuple<Columns...> columns_;
};

template <typename T, typename... Columns>
auto mapping(Columns... columns) {
    return Mapping<T, Columns...>(std::move(columns)...);
}

}
