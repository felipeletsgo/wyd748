#pragma once

#include "singleton.h"

// Repository of the water-macro controllers. The instance is created on
// demand by Singleton and keeps exclusive ownership of each MacroLevel.
// Callers only receive borrowed pointers and must never free them.
class MacroFactory : public Singleton<MacroFactory>
{
protected:
	friend class Singleton<MacroFactory>;

	// Only Singleton controls the lifecycle of the global instance.
	MacroFactory();
	virtual ~MacroFactory() {}
	MacroFactory(const MacroFactory&) = delete;
	MacroFactory& operator=(const MacroFactory&) = delete;

public:
	// Returns the controller for level (1..3), creating it when needed.
	// Returns nullptr for an unknown level. The pointer is borrowed.
	Macro::MacroLevel* getMacroLevel(const uint32_t level);



private:
	// unique_ptr makes explicit that the factory owns the strategies.
	std::map<uint32_t, std::unique_ptr<Macro::MacroLevel>> _map;
};
