#include "pch.h"
#include "MacroLevel.h"
#include "MacroFactory.h"

MacroFactory::MacroFactory() : _map()
{
}

// Localiza o controlador da macro por nivel e o cria sob demanda. A factory
// conserva ownership exclusivo no mapa; o ponteiro devolvido e emprestado e
// permanece valido enquanto a factory e a entrada correspondente existirem.
Macro::MacroLevel* MacroFactory::getMacroLevel(const uint32_t level)
{
	auto it = _map.find(level);

	if (it == _map.end())
	{
		Macro::MacroLevel* currentLevel = nullptr;

		// Cada faixa de agua possui estrategia propria. Niveis desconhecidos nao
		// sao materializados para evitar uma macro parcialmente configurada.
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

		// A partir daqui a unique_ptr transfere o ownership para a factory.
		_map.insert(std::make_pair(level, std::unique_ptr<Macro::MacroLevel>(currentLevel)));
	}

	// operator[] e seguro aqui porque a entrada existente ou recem-criada foi
	// confirmada acima; o chamador nao deve liberar o ponteiro retornado.
	return _map[level].get();
}

// Produz uma visao nao proprietaria dos niveis ja instanciados. O vetor e uma
// copia, mas seus elementos continuam pertencendo a factory.
const std::vector<Macro::MacroLevel*> MacroFactory::getAllMacroLevel() const
{
	auto tmp = std::vector<Macro::MacroLevel*>();

	for (auto& i : _map)
		tmp.push_back(i.second.get());

	return tmp;
}

void MacroFactory::onEvent()
{
	// Ponto de extensao preservado por compatibilidade. Nao ha evento global da
	// factory implementado no fluxo atual.
}
