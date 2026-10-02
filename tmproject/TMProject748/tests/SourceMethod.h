#pragma once
#include <cctype>
#include <string>

namespace source_contract {
// Keep offsets while masking non-code braces in comments and literals.
inline std::string MaskCpp(const std::string& source)
{
    auto code = source;
    for (std::size_t i = 0; i < source.size();) {
        const auto start = i;
        if (source.compare(i, 2, "//") == 0) {
            const auto end = source.find('\n', i + 2);
            i = end == std::string::npos ? source.size() : end;
        } else if (source.compare(i, 2, "/*") == 0) {
            const auto end = source.find("*/", i + 2);
            if (end == std::string::npos) return {};
            i = end + 2;
        } else if (source.compare(i, 2, "R\"") == 0) {
            const auto opening = source.find('(', i + 2);
            if (opening == std::string::npos || opening - i > 18) return {};
            const auto closing = ")" + source.substr(i + 2, opening - i - 2) + "\"";
            const auto end = source.find(closing, opening + 1);
            if (end == std::string::npos) return {};
            i = end + closing.size();
        } else if (source[i] == '"' || source[i] == '\'') {
            const char quote = source[i++];
            bool closed = false;
            while (i < source.size()) {
                if (source[i] == '\\') i += i + 1 < source.size() ? 2 : 1;
                else if (source[i++] == quote) { closed = true; break; }
            }
            if (!closed) return {};
        } else { ++i; continue; }
        for (auto j = start; j < i; ++j)
            if (code[j] != '\n') code[j] = ' ';
    }
    return code;
}

// Require a complete signature and exactly one definition. Adjacent methods
// are never delimiters: their owners/order can change independently.
inline std::string Method(const std::string& source, const std::string& signature)
{
    if (signature.empty()) return {};
    const auto code = MaskCpp(source);
    std::string result;
    for (std::size_t start = 0; (start = code.find(signature, start)) != std::string::npos;
         start += signature.size()) {
        const auto line = code.rfind('\n', start);
        const auto first = line == std::string::npos ? 0 : line + 1;
        bool definition = true;
        for (auto i = first; i < start; ++i)
            if (!std::isspace(static_cast<unsigned char>(code[i]))) definition = false;
        auto opening = start + signature.size();
        while (opening < code.size() && std::isspace(static_cast<unsigned char>(code[opening])))
            ++opening;
        if (!definition || opening == code.size() || code[opening] != '{') continue;
        auto end = opening + 1;
        int depth = 1;
        while (end < code.size() && depth != 0) {
            if (code[end] == '{') ++depth;
            if (code[end] == '}') --depth;
            ++end;
        }
        if (depth != 0 || !result.empty()) return {};
        result = source.substr(start, end - start);
    }
    return result;
}
}
