#pragma once

#include <cstring>

// Operacoes locais da fila de saida, independentes de WinSock e criptografia.
// Os buffers sao emprestados; o chamador garante a capacidade real informada.
namespace send_buffer
{
    // Caminho raw aceita exatamente o espaco restante; nao possui cabecalho.
    inline bool CanAppendRaw(int size, int queued, int capacity)
    {
        return size >= 0 && queued >= 0 && queued <= capacity &&
            size <= capacity - queued;
    }
// Mantem o limite estrito da fila legada e o tamanho WORD do enquadramento.
// A subtracao so ocorre depois de validar os indices, evitando overflow signed.
inline bool CanAppendPacket(int size, int queued, int capacity, int headerSize)
{
    return headerSize > 0 && size >= headerSize && size <= 0xFFFF &&
        queued >= 0 && queued < capacity && size < capacity - queued;
}

// Descarta somente o prefixo ja enviado. memmove admite sobreposicao; os bytes
// restantes ja estao cifrados e nao podem ser buscados na fila de recepcao.
// Estado invalido retorna false sem acessar memoria nem modificar os indices.
inline bool Compact(char* buffer, int capacity, int& queued, int& sent)
{
    if (!buffer || capacity <= 0 || queued < 0 || queued > capacity ||
        sent < 0 || sent > queued)
        return false;

    const int remaining = queued - sent;
    if (sent > 0 && remaining > 0)
        std::memmove(buffer, buffer + sent, static_cast<unsigned int>(remaining));
    queued = remaining;
    sent = 0;
    return true;
}

// Preserva a tentativa final de flush mesmo se o enqueue falhar, para nao
// bloquear mensagens anteriores. Sucesso exige aceite do pacote E do flush;
// nao significa entrega remota. Os callbacks sao sincronos e chamados uma vez.
template <typename Enqueue, typename Flush>
bool EnqueueAndFlush(Enqueue enqueue, Flush flush)
{
    const bool accepted = enqueue() != 0;
    const bool flushed = flush() != 0;
    return accepted && flushed;
}
}
