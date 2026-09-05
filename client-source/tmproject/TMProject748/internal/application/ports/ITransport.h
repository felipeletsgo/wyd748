#pragma once

#include "PacketView.h"

// Porta de envio sincrono da aplicacao, independente de Basedef/Win32.
// O adaptador conhece enquadramento, socket e criptografia; a aplicacao nao.
class ITransport
{
public:
	virtual ~ITransport() = default;
	// Empresta armazenamento gravavel apenas durante a chamada: o transporte
	// pode preencher o cabecalho, mas nunca reter/liberar o ponteiro. Rejeita
	// buffer nulo/tamanho invalido. true indica aceite local, nao entrega remota.
	virtual bool Send(const MutablePacketView& packet) = 0;
};
