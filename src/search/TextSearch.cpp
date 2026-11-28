#include "search/TextSearch.h"

#include <algorithm>
#include <stdexcept>
#include <string_view>

namespace {
std::string encode(std::u32string_view text)
{
    std::string result;
    for (char32_t value : text)
    {
        if (value < 0x80) result.push_back(static_cast<char>(value));
        else if (value < 0x800)
        {
            result.push_back(static_cast<char>(0xc0 | (value >> 6)));
            result.push_back(static_cast<char>(0x80 | (value & 0x3f)));
        }
        else if (value < 0x10000)
        {
            result.push_back(static_cast<char>(0xe0 | (value >> 12)));
            result.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (value & 0x3f)));
        }
        else
        {
            result.push_back(static_cast<char>(0xf0 | (value >> 18)));
            result.push_back(static_cast<char>(0x80 | ((value >> 12) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (value & 0x3f)));
        }
    }
    return result;
}

bool oneEdit(std::u32string_view a, std::u32string_view b)
{
    const std::size_t small = std::min(a.size(), b.size());
    if (std::max(a.size(), b.size()) - small > 1) return false;
    std::size_t i = 0;
    while (i < small && a[i] == b[i]) ++i;
    if (i == small) return true;
    if (a.size() == b.size())
    {
        if (a.substr(i + 1) == b.substr(i + 1))
            return true;
        return i + 1 < small && a[i] == b[i + 1] && a[i + 1] == b[i]
            && a.substr(i + 2) == b.substr(i + 2);
    }
    if (a.size() > b.size())
        return a.substr(i + 1) == b.substr(i);
    return a.substr(i) == b.substr(i + 1);
}

}

std::vector<std::u32string> text_search::queryWords(const std::u32string& normalized)
{
    std::vector<std::u32string> result;
    for (std::size_t start = 0; start < normalized.size();)
    {
        const auto end = normalized.find(U' ', start);
        result.push_back(normalized.substr(start, end == normalized.npos ? end : end - start));
        start = end == normalized.npos ? normalized.size() : end + 1;
    }
    for (const auto& word : result)
        if (word.size() < 3)
            throw std::invalid_argument("Enter at least 3 characters per search word");
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

std::vector<std::string> text_search::makeTrigrams(const std::u32string& normalized)
{
    std::vector<std::string> result;
    const std::u32string_view text(normalized);
    for (std::size_t i = 0; i + 2 < text.size(); ++i)
        if (text[i] != U' ' && text[i + 1] != U' ' && text[i + 2] != U' ')
            result.push_back(encode(text.substr(i, 3)));
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

bool text_search::matches(const std::u32string& normalizedText, const std::vector<std::u32string>& query)
{
    const std::u32string_view title(normalizedText);
    for (std::u32string_view part : query)
    {
        bool found = false;
        for (std::size_t start = 0; start < title.size();)
        {
            const auto end = title.find(U' ', start);
            const auto word = title.substr(start, end == title.npos ? end : end - start);
            start = end == title.npos ? title.size() : end + 1;
            if (word.find(part) != std::u32string::npos) { found = true; break; }
            if (!oneEdit(part, word)) continue;
            for (std::size_t i = 0; i + 2 < part.size(); ++i)
                if (word.find(part.substr(i, 3)) != std::u32string::npos)
                { found = true; break; }
            if (found) break;
        }
        if (!found) return false;
    }
    return !query.empty();
}
