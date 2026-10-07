#include "pch.h"
#include "MacroLevel.h"
#include "MacroFactory.h"

MacroFactory::MacroFactory() : _map()
{
}

// Finds the macro controller for a level and creates it on demand. The factory
// keeps exclusive ownership in the map; the returned pointer is borrowed and
// stays valid while the factory and the matching entry exist.
Macro::MacroLevel* MacroFactory::getMacroLevel(const uint32_t level)
{
	auto it = _map.find(level);

	if (it == _map.end())
	{
		Macro::MacroLevel* currentLevel = nullptr;

		// Each water tier has its own strategy. Unknown levels are not
		// materialized, to avoid a partially configured macro.
		if (level == 1)
			currentLevel = new Macro::Water_N();

		else if (level == 2)
			currentLevel = new Macro::Water_M();

		else if (level == 3)
			currentLevel = new Macro::Water_A();

		else
			return nullptr;

		if (currentLevel == nullptr)
			return nullptr;

		// From here the unique_ptr transfers ownership to the factory.
		_map.insert(std::make_pair(level, std::unique_ptr<Macro::MacroLevel>(currentLevel)));
	}

	// operator[] is safe here because the existing or newly created entry was
	// confirmed above; the caller must not free the returned pointer.
	return _map[level].get();
}
