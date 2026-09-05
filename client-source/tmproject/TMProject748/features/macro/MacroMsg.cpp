#include "pch.h"
#include "MacroLevel.h"
#include "MacroFactory.h"
#include "MacroMsg.h"

// Roteia uma mensagem de uso/movimento para a estrategia da faixa de agua.
// posX/posY sao coordenadas do mundo e Param e o ID do item recebido pelo
// fluxo da macro; nenhum desses valores tem ownership associado.
void MacroMsg::onEvent(const int posX, const int posY, const int Param)
{
	int Level = 0;

	// As tres familias de IDs pertencem as tres faixas de dificuldade. Param 0
	// e o marcador legado para movimento sem uso de item.
	if (Param >= 3173 && Param <= 3181)
		Level = 1;

	else if (Param >= 777 && Param <= 785)
		Level = 2;

	else if (Param >= 3182 && Param <= 3190)
		Level = 3;

	if (Param == 0)
		Level = 1;

	if (Level == 0)
		return;

	auto macroLevel = MacroFactory::instance().getMacroLevel(Level);

	if (!macroLevel)
		return;

	// Dentro desta area a macro executa deslocamento; fora dela consome o item
	// pela regra do nivel selecionado.
	if (posX >= 1954 && posY >= 1764 && posX <= 1976 && posY <= 1773)
		macroLevel->DoMove(posX, posY);
	else
		macroLevel->UseItem(posX, posY, Param);
}

void MacroMsg::onItemUsed(const int WaterScrollId)
{
	// Registra o nivel da agua a partir do primeiro pergaminho de cada familia.
	// IDs nao reconhecidos invalidam o estado em vez de reutilizar nivel antigo.
	switch (WaterScrollId)
	{
	case 3173:
		WaterLevel = 1;
		break;

	case 777:
		WaterLevel = 2;
		break;

	case 3182:
		WaterLevel = 3;
		break;

	default:
		WaterLevel = -1;
		break;
	}
}
