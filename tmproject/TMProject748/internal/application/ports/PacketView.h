#pragma once

#include <cstddef>

// Transport-port DTO without wire layout or platform dependencies.
// This borrowed packet view neither allocates nor frees storage or extends its lifetime.
// The caller guarantees size bytes during use. Opcode is dispatch metadata; it does not replace or rewrite the wire header.
struct PacketView
{
	unsigned int opcode = 0;
	const char* data = nullptr;
	std::size_t size = 0;

	// Check presence and length only, not packet contents.
	bool HasAtLeast(std::size_t required) const
	{
		return data != nullptr && size >= required;
	}

	// Use inclusive consumer limits before casts or size narrowing.
	// Do not access the buffer, including when the size is invalid.
	bool HasSizeBetween(std::size_t minimum, std::size_t maximum) const
	{
		return HasAtLeast(minimum) && size <= maximum;
	}
};

// Writable borrow for synchronous sending. The transport can fill the caller header but cannot retain the pointer.
// There is no conversion from PacketView; const must not be removed.
struct MutablePacketView
{
	unsigned int opcode = 0;
	char* data = nullptr;
	std::size_t size = 0;

	// The read view shares storage and its lifetime.
	PacketView AsReadOnly() const { return {opcode, data, size}; }

	// Reuse the pure predicate without duplicating the bounds policy.
	bool HasSizeBetween(std::size_t minimum, std::size_t maximum) const
	{
		return AsReadOnly().HasSizeBetween(minimum, maximum);
	}
};
