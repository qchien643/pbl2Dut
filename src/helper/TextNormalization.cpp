#include "helper/TextNormalization.h"

#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace {
std::u32string decode(const std::string& text)
{
    std::u32string result;
    for (std::size_t i = 0; i < text.size();)
    {
        const unsigned char first = text[i++];
        std::uint32_t value;
        int remaining;
        if (first < 0x80) { value = first; remaining = 0; }
        else if (first >= 0xc2 && first <= 0xdf) { value = first & 0x1f; remaining = 1; }
        else if (first >= 0xe0 && first <= 0xef) { value = first & 0x0f; remaining = 2; }
        else if (first >= 0xf0 && first <= 0xf4) { value = first & 7; remaining = 3; }
        else throw std::invalid_argument("Text normalization requires valid UTF-8");
        const int count = remaining;
        while (remaining-- > 0)
        {
            if (i == text.size() || (static_cast<unsigned char>(text[i]) & 0xc0) != 0x80)
                throw std::invalid_argument("Text normalization requires valid UTF-8");
            value = (value << 6) | (static_cast<unsigned char>(text[i++]) & 0x3f);
        }
        if ((count == 1 && value < 0x80) || (count == 2 && value < 0x800)
            || (count == 3 && value < 0x10000) || value > 0x10ffff
            || (value >= 0xd800 && value <= 0xdfff))
            throw std::invalid_argument("Text normalization requires valid UTF-8");
        result.push_back(static_cast<char32_t>(value));
    }
    return result;
}

char32_t fold(char32_t value)
{
    if (value < 0x80)
        return value >= U'A' && value <= U'Z' ? value + (U'a' - U'A') : value;
    static constexpr std::u32string_view groups[] = {
        U"aAàáảãạâầấẩẫậăằắẳẵặÀÁẢÃẠÂẦẤẨẪẬĂẰẮẲẴẶ",
        U"eEèéẻẽẹêềếểễệÈÉẺẼẸÊỀẾỂỄỆ", U"iIìíỉĩịÌÍỈĨỊ",
        U"oOòóỏõọôồốổỗộơờớởỡợÒÓỎÕỌÔỒỐỔỖỘƠỜỚỞỠỢ",
        U"uUùúủũụưừứửữựÙÚỦŨỤƯỪỨỬỮỰ", U"yYỳýỷỹỵỲÝỶỸỴ", U"dDđĐ"
    };
    for (const auto& group : groups)
        if (group.find(value) != std::u32string::npos) return group[0];
    // ponytail: Vietnamese/ASCII folding only; use a Unicode library for other languages.
    return value;
}

bool vietnameseMark(char32_t value)
{
    return value == 0x300 || value == 0x301 || value == 0x303 || value == 0x309
        || value == 0x323 || value == 0x302 || value == 0x306 || value == 0x31b;
}

bool separator(char32_t value)
{
    return value <= 0x20 || value == 0x7f || value == 0xa0 || value == 0x3000
        || (value >= 0x2000 && value <= 0x206f)
        || (value < 0x80 && !((value >= U'a' && value <= U'z')
            || (value >= U'0' && value <= U'9')));
}

}

std::u32string text_normalization::normalize(const std::string& text)
{
    std::u32string result;
    for (char32_t value : decode(text))
    {
        if (vietnameseMark(value) && !result.empty()
            && std::u32string_view(U"aeiouy").find(result.back()) != std::u32string_view::npos) continue;
        value = fold(value);
        if (separator(value))
        {
            if (!result.empty() && result.back() != U' ') result.push_back(U' ');
        }
        else result.push_back(value);
    }
    if (!result.empty() && result.back() == U' ') result.pop_back();
    return result;
}
