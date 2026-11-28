#pragma once

#include <string>
#include <vector>

namespace text_search {

// All inputs are already normalized Unicode text, not UTF-8 bytes.
std::vector<std::u32string> queryWords(const std::u32string& normalized);
std::vector<std::string> makeTrigrams(const std::u32string& normalized);
bool matches(const std::u32string& normalizedText,
             const std::vector<std::u32string>& query);

}
