#include "pch.h"
#include "MacroLevel.h"
#include "MacroFactory.h"

MacroFactory::MacroFactory() : _map()
{
}

// Localiza o controlador da macro por nível e o cria sob demanda. A factory
// conserva ownership exclusivo no mapa; o ponteiro devolvido é emprestado e
// permanece válido enquanto a factory e a entrada correspondente existirem.
Macro::MacroLevel* MacroFactory::getMacroLevel(const uint32_t level)
{
	auto it = _map.find(level);

	if (it == _map.end())
	{
		Macro::MacroLevel* currentLevel = nullptr;

		// Cada faixa de água possui estratégia própria. Níveis desconhecidos não
		// são materializados para evitar uma macro parcialmente configurada.
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

	// operator[] é seguro aqui porque a entrada existente ou recém-criada foi
	// confirmada acima; o chamador não deve liberar o ponteiro retornado.
	return _map[level].get();
}

// Produz uma visão não proprietária dos níveis já instanciados. O vetor é uma
// cópia, mas seus elementos continuam pertencendo à factory.
const std::vector<Macro::MacroLevel*> MacroFactory::getAllMacroLevel() const
{
	auto tmp = std::vector<Macro::MacroLevel*>();

	for (auto& i : _map)
		tmp.push_back(i.second.get());

	return tmp;
}

void MacroFactory::onEvent()
{
	// Ponto de extensão preservado por compatibilidade. Não há evento global da
	// factory implementado no fluxo atual.
}
