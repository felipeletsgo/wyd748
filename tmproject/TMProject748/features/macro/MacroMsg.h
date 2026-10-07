#pragma once

#include "singleton.h"

// Entry facade of the water macro. Translates events received from the scene
// into calls to the right controller and keeps the last recognized level.
class MacroMsg : public Singleton<MacroMsg>
{
protected:
	friend class Singleton<MacroMsg>;

	// Singleton controls the instance. The class is not copyable so that the
	// recognized level has a single source in the client.
	MacroMsg() {}
	virtual ~MacroMsg() {}
	MacroMsg(const MacroMsg&) = delete;
	MacroMsg& operator=(const MacroMsg&) = delete;

public:
	// posX/posY are world coordinates and Param is the item ID. The arguments
	// are plain values and transfer no ownership.
	void onEvent(const int posX, const int posY, const int Param);

	// Legacy declaration still without an implementation. Do not call it until
	// its contract and lifecycle are documented.
	void onPutItem(const int posX, const int posY, const int Level);


	// -1 means that no valid family was recognized. The Singleton instance
	// owns this state.
	int WaterLevel = -1;
};
