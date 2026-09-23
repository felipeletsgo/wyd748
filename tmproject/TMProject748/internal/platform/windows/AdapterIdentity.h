#pragma once

#include <Windows.h>
#include <iphlpapi.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>

// The 7.48 login packet stores the first adapter's GUID as four 32-bit words.
// Parse only a complete GUID so an API error or malformed adapter name cannot
// leave a partially populated identity in the packet.
inline bool ParseAdapterIdentity(const char* name, std::size_t capacity, int (&identity)[4])
{
	if (name == nullptr)
		return false;

	unsigned int words[4]{};
	std::size_t digits = 0;
	bool terminated = false;
	for (std::size_t i = 0; i < capacity; ++i)
	{
		const unsigned char ch = static_cast<unsigned char>(name[i]);
		if (ch == 0)
		{
			terminated = true;
			break;
		}
		if (ch == '{' || ch == '}' || ch == '-')
			continue;
		unsigned int value;
		if (ch >= '0' && ch <= '9')
			value = ch - '0';
		else if (ch >= 'a' && ch <= 'f')
			value = ch - 'a' + 10;
		else if (ch >= 'A' && ch <= 'F')
			value = ch - 'A' + 10;
		else
			return false;
		if (digits >= 32)
			return false;
		words[digits / 8] = (words[digits / 8] << 4) | value;
		++digits;
	}
	if (!terminated || digits != 32)
		return false;

	static_assert(sizeof(words) == sizeof(identity), "adapter identity ABI changed");
	std::memcpy(identity, words, sizeof(words));
	return true;
}

inline bool ReadFirstAdapterIdentity(int (&identity)[4])
{
	DWORD size = 0;
	if (GetAdaptersInfo(nullptr, &size) != ERROR_BUFFER_OVERFLOW ||
		size < sizeof(IP_ADAPTER_INFO))
		return false;

	void* buffer = std::malloc(size);
	if (buffer == nullptr)
		return false;

	const DWORD result = GetAdaptersInfo(static_cast<PIP_ADAPTER_INFO>(buffer), &size);
	bool parsed = false;
	if (result == ERROR_SUCCESS)
	{
		const auto* adapter = static_cast<const IP_ADAPTER_INFO*>(buffer);
		parsed = ParseAdapterIdentity(adapter->AdapterName,
			sizeof(adapter->AdapterName), identity);
	}
	std::free(buffer);
	return parsed;
}
