#pragma once

#include "PacketView.h"

// Synchronous application send port, independent of Basedef and Win32.
// The adapter owns framing, sockets and encryption; the application does not.
class ITransport
{
public:
	virtual ~ITransport() = default;
	// Borrow writable storage only during this call. The transport can fill the header but cannot retain or free the pointer.
	// Reject null buffers and invalid sizes. True means local acceptance, not remote delivery.
	virtual bool Send(const MutablePacketView& packet) = 0;
};
