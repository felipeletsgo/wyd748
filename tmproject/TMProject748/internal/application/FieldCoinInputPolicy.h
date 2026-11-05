#pragma once

#include "FieldInteractionPolicy.h"
#include "../wire/ServerWarLetterContract.h"

#include <cstddef>

// Local validation of the field-scene numeric/name input prompt (B_IG_OK).
// The scene keeps control lookup, messages, focus, packets and every mode's
// action. This decides only which existing rejection, if any, applies.
namespace coin_input
{
// Reports any byte outside '0'..'9' (plain char comparison, as in the scene),
// then rewrites '%' to '!' in place. The scan precedes the rewrite.
inline bool ScanAndSanitize(char* text, int length)
{
    bool nonDigit = false;
    for (int n = 0; n < length; ++n) {
        if (text[n] < '0' || text[n] > '9') {
            nonDigit = true;
            break;
        }
    }
    for (int n = 0; n < length; ++n) {
        if (text[n] == '%')
            text[n] = '!';
    }
    return nonDigit;
}

enum class Rejection
{
    None,
    Empty,                  // refocus only
    InvalidAmount,          // message 34, also hides the optional chat selector
    InvalidName,            // message 409 (mode 12)
    AutoTradeNonPositive,   // message 34 (mode 4)
    AutoTradeAboveLimit,    // message 143 with the 2,000,000,000 limit (mode 4)
    ServerWarChannel        // message 34 (server-war declare/refuse modes)
};

// Modes whose text is not a currency amount checked against the carried coin.
constexpr bool SkipsAmountCheck(int mode)
{
    return mode == 12 || mode == 11 || mode == 8 || mode == 3 || mode == 6;
}

// Largest amount that the 32-bit signed Parm/TradeMoney wire fields carry.
constexpr long long kMaxWireAmount = 2147483647LL;

// Currency modes whose action narrows the amount to a 32-bit wire field with no
// later bound: Cargo deposit (0), trade gold (1), Cargo withdraw (2) and store
// gold (7). Modes 4, 9, 10 and 12 have their own later bounds.
constexpr bool NarrowsToWireAmount(int mode)
{
    return mode == 0 || mode == 1 || mode == 2 || mode == 7;
}

// Checks run in the original order. The carried coin is read only where the
// scene read it: after the non-digit and negative tests, before the mode test.
// A08-01: an amount above the wire domain is an invalid amount. Without this
// bound, a trade amount with 11 or more digits overflowed the 11-byte
// "%10lld" display buffer in B_IG_OK and terminated the client.
template<class CarriedCoin>
Rejection Validate(int mode, std::size_t length, bool nonDigit, long long value,
    CarriedCoin&& carriedCoin)
{
    if (length == 0)
        return Rejection::Empty;
    if (!SkipsAmountCheck(mode) &&
        (nonDigit || value < 0 || (value > carriedCoin() && !mode) ||
            (NarrowsToWireAmount(mode) && value > kMaxWireAmount)))
        return Rejection::InvalidAmount;
    if (nonDigit && mode == 12)
        return Rejection::InvalidName;
    if (mode == 4 && !field_interaction::IsValidAutoTradePrice(value))
        return value <= 0 ? Rejection::AutoTradeNonPositive : Rejection::AutoTradeAboveLimit;
    if ((mode == kDeclareServerWarPromptMode || mode == kRefuseServerWarPromptMode) &&
        !IsEncodableServerWarTargetChannel(value))
        return Rejection::ServerWarChannel;
    return Rejection::None;
}

enum class AllSource { None, Carried, Cargo };

// Source for the "all" shortcut, which only fills the prompt.
constexpr AllSource AllAmountSource(int mode)
{
    if (!mode || mode == 1 || mode == 7)
        return AllSource::Carried;
    if (mode == 2)
        return AllSource::Cargo;
    return AllSource::None;
}
}
