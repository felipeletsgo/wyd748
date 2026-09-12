#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>

// Atualizacao local de um slot confirmado pelo servidor, sem UI ou protocolo.
// O array emprestado determina a capacidade de armazenamento (128 no client),
// nao o limite de uso pelo jogador (120). Nao aloca nem retem referencias.
template <typename Item, std::size_t Capacity>
bool ApplyCargoSlot(Item (&cargo)[Capacity], int position, const Item& item)
{
    static_assert(std::is_trivially_copyable<Item>::value,
        "Cargo items must retain byte-copy semantics");
    if (position < 0 || static_cast<std::size_t>(position) >= Capacity)
        return false;

    // memmove conserva os bytes inclusive se origem e destino forem o mesmo
    // slot. A rejeicao acima preserva integralmente o array.
    std::memmove(&cargo[position], &item, sizeof(Item));
    return true;
}
