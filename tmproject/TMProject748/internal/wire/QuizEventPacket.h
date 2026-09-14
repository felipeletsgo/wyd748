#pragma once
#include "MessageHeader.h"
#include <array>
#include <cstdint>
#include <cstring>

// EXTENSAO_COORDENADA v1. Legacy 0x1C6/0x2C7 cannot authorize rewards.
namespace quiz_event
{
    constexpr unsigned short ChallengeOpcode = 0x7F10;
    constexpr unsigned short AnswerOpcode = 0x7F11;
    struct Challenge
    {
        MSG_STANDARD Header;
        std::uint16_t Version, Kind;
        std::array<unsigned char, 16> Token;
        std::uint32_t TimeoutMs;
        char Question[96];
        std::int32_t Answers[4];
    };
    struct Answer
    {
        MSG_STANDARD Header;
        std::uint16_t Version, Choice;
        std::array<unsigned char, 16> Token;
        std::uint32_t Reserved;
    };
    static_assert(sizeof(Challenge) == 148 && offsetof(Challenge, Question) == 36 &&
        offsetof(Challenge, Answers) == 132, "Quiz challenge ABI");
    static_assert(sizeof(Answer) == 36 && offsetof(Answer, Token) == 16 &&
        offsetof(Answer, Reserved) == 32, "Quiz answer ABI");

    inline bool Parse(const void* bytes, std::size_t size, Challenge& out)
    {
        if (!bytes || size != sizeof(Challenge)) return false;
        Challenge p{};
        std::memcpy(&p, bytes, sizeof(p));
        if (p.Header.Size != sizeof(p) || p.Header.Type != ChallengeOpcode ||
            p.Version != 1 || p.Kind > 1 || p.Token == std::array<unsigned char, 16>{}) return false;
        if (p.Kind == 1)
        {
            if (p.TimeoutMs != 10000 || !p.Question[0] || !std::memchr(p.Question, 0, sizeof(p.Question))) return false;
            for (int i = 0; i < 4; ++i)
            {
                if (p.Answers[i] <= 0) return false;
                for (int j = 0; j < i; ++j) if (p.Answers[i] == p.Answers[j]) return false;
            }
        }
        out = p;
        return true;
    }
    struct State
    {
        bool Active = false;
        std::uint32_t OpenedAt = 0;
        std::array<unsigned char, 16> Token{};
        bool Expired(std::uint32_t now) const { return Active && std::uint32_t(now - OpenedAt) >= 10000; }
        bool Apply(const Challenge& p, std::uint32_t now)
        {
            if (p.Kind == 0)
            {
                if (!Active || Token != p.Token) return false;
                Active = false;
                return true;
            }
            // Replayed show packets cannot reopen the last answered/expired round.
            if (Token == p.Token) return false;
            Token = p.Token; OpenedAt = now; Active = true;
            return true;
        }
        bool Respond(std::uint16_t id, unsigned int choice, std::uint32_t now, Answer& out)
        {
            if (!Active || choice > 3) return false;
            if (Expired(now)) { Active = false; return false; }
            Active = false;
            out = {};
            out.Header.Size = sizeof(out); out.Header.Type = AnswerOpcode; out.Header.ID = id;
            out.Version = 1; out.Choice = static_cast<std::uint16_t>(choice); out.Token = Token;
            return true;
        }
    };
}
