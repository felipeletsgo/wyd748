#pragma once

// Owner decision (2026-10-20): colored runes flash on the floor of the
// secret-room (Magic Chamber) entrance hall in Field0813 while the local
// player is inside it. The hall is bounded by the walls at x=1038 and
// y=1678/1702; the ticket tile is (1045..1047, 1689..1691). No native 7.48
// generator was found, so this is a client-only project visual: it uses the
// stock colored rune textures Effect\pattern01..04.wys (effect list 82..85)
// and needs no packet or server state.
namespace entrance_runes
{
constexpr int kHallMinX = 1039;
constexpr int kHallMaxX = 1058;
constexpr int kHallMinY = 1679;
constexpr int kHallMaxY = 1701;
// Rune centers stay one tile inside the hall so a 2x2-cell decal never
// projects onto a wall.
constexpr int kRuneMinX = 1041;
constexpr int kRuneMaxX = 1056;
constexpr int kRuneMinY = 1681;
constexpr int kRuneMaxY = 1699;
constexpr int kFirstTexture = 82;
constexpr int kTextureCount = 4;
constexpr int kGridCells = 2;
constexpr unsigned int kLifetimeMs = 4000;
constexpr int kEntryBurst = 5;
constexpr unsigned int kMinDelayMs = 350;
constexpr unsigned int kDelaySpreadMs = 700;

constexpr bool InHall(int x, int y)
{
	return x >= kHallMinX && x <= kHallMaxX && y >= kHallMinY && y <= kHallMaxY;
}

constexpr int Positive(int roll)
{
	return roll < 0 ? -(roll + 1) : roll;
}

constexpr int Texture(int roll)
{
	return kFirstTexture + Positive(roll) % kTextureCount;
}

constexpr int RuneX(int roll)
{
	return kRuneMinX + Positive(roll) % (kRuneMaxX - kRuneMinX + 1);
}

constexpr int RuneY(int roll)
{
	return kRuneMinY + Positive(roll) % (kRuneMaxY - kRuneMinY + 1);
}

constexpr unsigned int NextDelayMs(int roll)
{
	return kMinDelayMs + static_cast<unsigned int>(Positive(roll)) % kDelaySpreadMs;
}

// Entering the hall spawns a burst at once; afterwards one rune appears at
// each randomized delay until the player leaves. Unsigned subtraction keeps
// the schedule valid across server-time wraparound.
constexpr bool Due(unsigned int now, unsigned int scheduled)
{
	return static_cast<int>(now - scheduled) >= 0;
}
}
