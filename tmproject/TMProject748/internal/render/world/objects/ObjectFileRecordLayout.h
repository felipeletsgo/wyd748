#pragma once

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>

// Field*.dat stores a 28-byte base record and an optional 8-byte scale pair.
inline std::size_t ObjectFileRecordSize(std::uint32_t type)
{
	return (type >= 501 && type <= 506) ||
		(type >= 511 && type <= 518) ||
		(type >= 520 && type < 600) ? 36u : 28u;
}

// The loader truncates each leaf coordinate (and coordinate + 1) to int before
// taking % 128. Values in (-1, 0) are valid: truncation maps them to zero.
inline bool ObjectFileLeafMaskCoordinateIsSafe(float coordinate)
{
	return std::isfinite(coordinate) && coordinate > -1.0f &&
		static_cast<double>(coordinate) + 1.0 <
		static_cast<double>((std::numeric_limits<int>::max)()) + 1.0;
}

inline bool ObjectFileIntegerCoordinateIsSafe(float coordinate)
{
	const double value = static_cast<double>(coordinate);
	return std::isfinite(coordinate) &&
		value > static_cast<double>((std::numeric_limits<int>::min)()) - 1.0 &&
		value < static_cast<double>((std::numeric_limits<int>::max)()) + 1.0;
}

// Preserve the two 16-bit grid lanes while avoiding a signed left shift.
inline std::uint32_t ObjectFileSpatialKey(int x, int y)
{
	return static_cast<std::uint32_t>(x >> 5) +
		(static_cast<std::uint32_t>(y >> 5) << 16);
}

inline bool ValidateObjectFileRecords(const unsigned char* data, std::size_t size,
	std::size_t maxRecords, std::size_t maxSeas, std::size_t maxLights,
	float offsetX = 0.0f, float offsetY = 0.0f)
{
	if (size && !data)
		return false;

	std::size_t offset = 0;
	std::size_t count = 0;
	std::size_t seas = 0;
	std::size_t lights = 0;
	while (offset < size)
	{
		if (size - offset < 28 || count == maxRecords)
			return false;
		std::uint32_t type = 0;
		std::memcpy(&type, data + offset, sizeof type);
		const auto recordSize = ObjectFileRecordSize(type);
		if (size - offset < recordSize)
			return false;
		float x = 0.0f;
		float y = 0.0f;
		float height = 0.0f;
		std::memcpy(&x, data + offset + 4, sizeof x);
		std::memcpy(&y, data + offset + 8, sizeof y);
		std::memcpy(&height, data + offset + 12, sizeof height);
		if (!ObjectFileIntegerCoordinateIsSafe(x) ||
			!ObjectFileIntegerCoordinateIsSafe(y) ||
			!ObjectFileIntegerCoordinateIsSafe(height) ||
			!ObjectFileIntegerCoordinateIsSafe(x + offsetX) ||
			!ObjectFileIntegerCoordinateIsSafe(y + offsetY))
			return false;
		if (type >= 311 && type <= 322)
		{
			if (!ObjectFileLeafMaskCoordinateIsSafe(x) ||
				!ObjectFileLeafMaskCoordinateIsSafe(y))
				return false;
		}
		if (type == 2 && seas++ == maxSeas)
			return false;
		if (type >= 511 && type <= 518 && lights++ == maxLights)
			return false;
		offset += recordSize;
		++count;
	}
	return true;
}
