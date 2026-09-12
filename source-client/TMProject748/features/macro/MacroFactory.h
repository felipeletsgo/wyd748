#pragma once

#include "singleton.h"

// Repositorio dos controladores da macro de agua. A instancia e criada sob
// demanda por Singleton e conserva ownership exclusivo de cada MacroLevel.
// Os chamadores recebem apenas ponteiros emprestados e nunca devem libera-los.
class MacroFactory : public Singleton<MacroFactory>
{
protected:
	friend class Singleton<MacroFactory>;

	// Somente Singleton controla o lifecycle da instancia global.
	MacroFactory();
	virtual ~MacroFactory() {}
	MacroFactory(const MacroFactory&) = delete;
	MacroFactory& operator=(const MacroFactory&) = delete;

public:
	// Retorna o controlador de level (1..3), criando-o quando necessario.
	// Retorna nullptr para nivel desconhecido. O ponteiro e emprestado.
	Macro::MacroLevel* getMacroLevel(const uint32_t level);

	// O vetor retornado e uma copia, mas seus ponteiros continuam pertencendo
	// a factory e permanecem validos durante o lifecycle dela.
	const std::vector<Macro::MacroLevel*> getAllMacroLevel() const;

	// Ponto de extensao legado inerte, preservado por compatibilidade.
	static void onEvent();

private:
	// unique_ptr torna explicito que a factory e proprietaria das estrategias.
	std::map<uint32_t, std::unique_ptr<Macro::MacroLevel>> _map;
};
