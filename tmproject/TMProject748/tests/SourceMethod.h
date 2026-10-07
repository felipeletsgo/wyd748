#pragma once
#include <cctype>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

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

// A method body in which every call to another `owner` member defined in
// `source` is followed by that member's body (recursively, up to `depth`).
// Statements moved into private helpers therefore keep their original order
// for ordering contracts, without depending on where the helpers are placed.
inline std::string WithHelpers(const std::string& source, const std::string& signature,
    const std::string& owner, int depth = 3)
{
    const auto body = Method(source, signature);
    if (body.empty() || depth == 0) return body;
    const auto code = MaskCpp(source);
    const auto isName = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
    struct Helper { std::string name, signature; };
    std::vector<Helper> helpers;
    const auto qualifier = owner + "::";
    for (std::size_t at = 0; (at = code.find(qualifier, at)) != std::string::npos; at += qualifier.size()) {
        const auto line = code.rfind('\n', at);
        const auto first = line == std::string::npos ? 0 : line + 1;
        if (first == at || std::isspace(static_cast<unsigned char>(code[first]))) continue;   // not a definition
        auto nameEnd = at + qualifier.size();
        while (nameEnd < code.size() && isName(code[nameEnd])) ++nameEnd;
        if (nameEnd == code.size() || code[nameEnd] != '(') continue;
        int parens = 0;
        auto close = nameEnd;
        for (; close < code.size(); ++close) {
            if (code[close] == '(') ++parens;
            if (code[close] == ')' && --parens == 0) break;
        }
        if (close == code.size()) continue;
        const auto helperSignature = source.substr(first, close + 1 - first);
        if (helperSignature != signature && !Method(source, helperSignature).empty())
            helpers.push_back({ source.substr(at + qualifier.size(), nameEnd - at - qualifier.size()), helperSignature });
    }
    const auto bodyCode = MaskCpp(body);
    std::vector<std::pair<std::size_t, const Helper*>> calls;
    for (const auto& helper : helpers) {
        const auto call = helper.name + "(";
        for (std::size_t at = 0; (at = bodyCode.find(call, at)) != std::string::npos; at += call.size()) {
            if (at > 0 && (isName(bodyCode[at - 1]) || bodyCode[at - 1] == ':' || bodyCode[at - 1] == '.' || bodyCode[at - 1] == '>'))
                continue;
            calls.emplace_back(at, &helper);
        }
    }
    std::sort(calls.begin(), calls.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    std::string result;
    std::size_t copied = 0;
    for (const auto& [at, helper] : calls) {
        if (at < copied) continue;
        auto lineEnd = body.find('\n', at);
        if (lineEnd == std::string::npos) lineEnd = body.size();
        result += body.substr(copied, lineEnd - copied);
        result += "\n" + WithHelpers(source, helper->signature, owner, depth - 1);
        copied = lineEnd;
    }
    return result + body.substr(copied);
}
}
