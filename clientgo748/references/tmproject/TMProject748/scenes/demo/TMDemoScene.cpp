#include "pch.h"
#include "TMDemoScene.h"

// Stub importado do TMProject posterior e mantido para compatibilidade de
// source. Não inicializa os campos declarados no header porque ainda não existe
// um contrato 7.48 confirmado para recursos, controles ou teardown da demo.
TMDemoScene::TMDemoScene()
{
}

TMDemoScene::~TMDemoScene()
{
}

int TMDemoScene::InitializeScene()
{
	// Pendente: materialização de controles/assets e ownership da cena.
	return 0;
}

int TMDemoScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)
{
	// Stub deliberado: não consome eventos de controles desconhecidos.
	return 0;
}

int TMDemoScene::OnCharEvent(char iCharCode, int lParam)
{
	// Stub deliberado: não consome entrada de teclado.
	return 0;
}

int TMDemoScene::OnPacketEvent(unsigned int dwCode, char* buf)
{
	// buf permanece pertencendo ao dispatcher. Nenhum opcode foi atribuído a
	// esta cena enquanto o contrato não for rastreado.
	return 0;
}

int TMDemoScene::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	// Stub deliberado: não consome entrada de mouse.
	return 0;
}

int TMDemoScene::FrameMove(unsigned int dwServerTime)
{
	// Sem timeline adaptada, a cena não avança estado.
	return 0;
}

void TMDemoScene::ResetDemoPlayer()
{
	// Pendente: lifecycle dos atores da demonstração.
}

void TMDemoScene::ReadTimeTable()
{
	// Pendente: formato e origem da tabela de tempos.
}

void TMDemoScene::CamAction()
{
	// Pendente: contrato de câmera e transições observáveis.
}

void TMDemoScene::ReadStrings()
{
	// Pendente: recurso, encoding e limites das strings da apresentação.
}
