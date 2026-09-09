#pragma once

#include <cstddef>

// DTO da porta de transporte, sem layout wire ou dependencia de plataforma.
// Descricao emprestada de um pacote: nao aloca, libera nem prolonga a vida do
// buffer. O chamador garante que size bytes existem enquanto a view for usada.
// opcode e metadado de dispatch; nao substitui nem reescreve o cabecalho wire.
struct PacketView
{
	unsigned int opcode = 0;
	const char* data = nullptr;
	std::size_t size = 0;

	// Verifica somente presenca e comprimento; nao valida o conteudo do pacote.
	bool HasAtLeast(std::size_t required) const
	{
		return data != nullptr && size >= required;
	}

	// Limites inclusivos fornecidos pelo consumidor, antes de qualquer cast ou
	// estreitamento de size. Nao acessa o buffer, mesmo para tamanho invalido.
	bool HasSizeBetween(std::size_t minimum, std::size_t maximum) const
	{
		return HasAtLeast(minimum) && size <= maximum;
	}
};

// Emprestimo gravavel para envio sincrono. O transporte pode preencher o
// cabecalho no armazenamento do chamador, mas nao reter o ponteiro.
// Nao ha conversao de PacketView para este tipo: const nao pode ser removido.
struct MutablePacketView
{
	unsigned int opcode = 0;
	char* data = nullptr;
	std::size_t size = 0;

	// A leitura compartilha o mesmo armazenamento e prazo de validade.
	PacketView AsReadOnly() const { return {opcode, data, size}; }

	// Reutiliza o predicado puro sem duplicar a politica de limites.
	bool HasSizeBetween(std::size_t minimum, std::size_t maximum) const
	{
		return AsReadOnly().HasSizeBetween(minimum, maximum);
	}
};
