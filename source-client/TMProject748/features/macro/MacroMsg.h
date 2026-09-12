#pragma once

#include "singleton.h"

// Fachada de entrada da macro de agua. Traduz eventos recebidos da cena em
// chamadas para o controlador correto e conserva o ultimo nivel reconhecido.
class MacroMsg : public Singleton<MacroMsg>
{
protected:
	friend class Singleton<MacroMsg>;

	// Singleton controla a instancia. A classe nao e copiavel para que o nivel
	// reconhecido tenha uma unica fonte dentro do client.
	MacroMsg() {}
	virtual ~MacroMsg() {}
	MacroMsg(const MacroMsg&) = delete;
	MacroMsg& operator=(const MacroMsg&) = delete;

public:
	// posX/posY sao coordenadas do mundo e Param e o ID do item. Os argumentos
	// sao valores simples e nao transferem ownership.
	void onEvent(const int posX, const int posY, const int Param);

	// Declaracao legada ainda sem implementacao. Nao deve ser chamada ate que
	// seu contrato e lifecycle sejam documentados.
	void onPutItem(const int posX, const int posY, const int Level);

	// Atualiza WaterLevel pelo ID inicial da familia de pergaminhos. Um ID
	// desconhecido restaura o estado invalido (-1).
	void onItemUsed(const int WaterScrollId);

	// -1 significa que nenhuma familia valida foi reconhecida. A instancia
	// Singleton e proprietaria deste estado.
	int WaterLevel = -1;
};
