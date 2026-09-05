#pragma once

#include "Basedef.h"

// Application-facing transport boundary. Implementations own socket and
// encryption details; use cases request only a validated wire message.
class ITransport
{
public:
	virtual ~ITransport() = default;
	// The implementation must not take ownership of the caller's buffer.
	virtual bool Send(const MSG_STANDARD* message, unsigned int size) = 0;
};
