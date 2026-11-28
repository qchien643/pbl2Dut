#pragma once

#include <string>

namespace text_normalization {

// Lowercase, remove Vietnamese accents and collapse separators to single spaces.
// Reject invalid UTF-8; return Unicode characters for character-based matching.
std::u32string normalize(const std::string& text);

}
