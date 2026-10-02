#include "../internal/application/FieldCoinInputPolicy.h"

#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace
{
// Independent oracle copied from the original B_IG_OK validation sequence.
coin_input::Rejection LegacyValidate(int mode, std::size_t length, bool bFind,
    long long nInputValue, int coin, int& coinReads)
{
    const auto readCoin = [&] { ++coinReads; return coin; };
    if (length <= 0)
        return coin_input::Rejection::Empty;
    if (mode != 12 && mode != 11 && mode != 8 && mode != 3 && mode != 6) {
        if (bFind == true || nInputValue < 0 || nInputValue > readCoin() && !mode)
            return coin_input::Rejection::InvalidAmount;
    }
    if (bFind == true && mode == 12)
        return coin_input::Rejection::InvalidName;
    if (mode == 4 && !field_interaction::IsValidAutoTradePrice(nInputValue))
        return nInputValue <= 0 ? coin_input::Rejection::AutoTradeNonPositive
                                : coin_input::Rejection::AutoTradeAboveLimit;
    if ((mode == kDeclareServerWarPromptMode || mode == kRefuseServerWarPromptMode) &&
        !IsEncodableServerWarTargetChannel(nInputValue))
        return coin_input::Rejection::ServerWarChannel;
    return coin_input::Rejection::None;
}

static_assert(coin_input::AllAmountSource(0) == coin_input::AllSource::Carried, "Mode 0 uses carried coin");
static_assert(coin_input::AllAmountSource(1) == coin_input::AllSource::Carried, "Mode 1 uses carried coin");
static_assert(coin_input::AllAmountSource(7) == coin_input::AllSource::Carried, "Mode 7 uses carried coin");
static_assert(coin_input::AllAmountSource(2) == coin_input::AllSource::Cargo, "Mode 2 uses cargo coin");
static_assert(coin_input::AllAmountSource(4) == coin_input::AllSource::None, "Mode 4 has no all amount");
}

int RunFieldCoinInputPolicyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL field coin input: %s\n", message);
        }
    };

    // Scan precedes the '%' rewrite; only the rewrite mutates the buffer.
    struct Case { const char* input; bool nonDigit; const char* output; };
    for (const Case& item : {
        Case{"", false, ""}, Case{"0", false, "0"}, Case{"0123456789", false, "0123456789"},
        Case{"12a", true, "12a"}, Case{"-5", true, "-5"}, Case{" 5", true, " 5"},
        Case{"5%", true, "5!"}, Case{"%%", true, "!!"}, Case{"\x80" "1", true, "\x80" "1"},
        Case{"/", true, "/"}, Case{":", true, ":"}, Case{"1%2%3", true, "1!2!3"}}) {
        char buffer[32]{};
        std::memcpy(buffer, item.input, std::strlen(item.input) + 1);
        const bool nonDigit = coin_input::ScanAndSanitize(buffer, static_cast<int>(std::strlen(buffer)));
        check(nonDigit == item.nonDigit, "non-digit scan");
        check(!std::strcmp(buffer, item.output), "percent rewrite");
    }
    // A shorter length leaves the remainder untouched.
    char partial[] = "1%2%";
    coin_input::ScanAndSanitize(partial, 2);
    check(!std::strcmp(partial, "1!2%"), "rewrite is bounded by the supplied length");

    const long long values[]{-2147483649LL, -1, 0, 1, 99, 100, 101, 1999999999LL, 2000000000LL,
        2147483647LL, 2147483648LL, 4294967297LL};
    const int coins[]{0, 100, 2147483647};
    for (int mode = -1; mode <= 14; ++mode)
        for (std::size_t length : {std::size_t{0}, std::size_t{1}, std::size_t{12}})
            for (int nonDigit = 0; nonDigit < 2; ++nonDigit)
                for (long long value : values)
                    for (int coin : coins) {
                        int legacyReads = 0;
                        int policyReads = 0;
                        const auto expected = LegacyValidate(mode, length, nonDigit != 0, value, coin, legacyReads);
                        const auto actual = coin_input::Validate(mode, length, nonDigit != 0, value,
                            [&] { ++policyReads; return coin; });
                        check(actual == expected, "rejection matches legacy order");
                        check(policyReads == legacyReads, "carried coin is read only where the scene read it");
                    }

    for (int mode = -1; mode <= 14; ++mode) {
        const auto source = coin_input::AllAmountSource(mode);
        const auto expected = (!mode || mode == 1 || mode == 7) ? coin_input::AllSource::Carried
            : mode == 2 ? coin_input::AllSource::Cargo : coin_input::AllSource::None;
        check(source == expected, "all-amount source");
    }
    return failures;
}
