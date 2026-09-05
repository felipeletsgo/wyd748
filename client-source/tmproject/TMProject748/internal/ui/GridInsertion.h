#pragma once

#include <cstddef>

// Fronteira pura da lista visual: nao decide geometria, ocupacao ou wire.
// A capacidade vem do array real. Rejeicao deixa ownership com o chamador.
namespace grid_insertion
{
    template <typename Item, std::size_t Capacity>
    bool CanAppend(Item* const (&)[Capacity], int count, const Item* item)
    {
        return item != nullptr && count >= 0 &&
            static_cast<std::size_t>(count) < Capacity;
    }

    // Executa a insercao legada uma vez, somente quando ha espaco na lista.
    // O callback e dono de todas as mutacoes, inclusive count e binding.
    // Nenhum ponteiro e retido/liberado aqui e falhas nao causam retry.
    template <typename Item, std::size_t Capacity, typename Insert>
    int Execute(Item* const (&list)[Capacity], int count, Item* item, Insert&& insert)
    {
        if (!CanAppend(list, count, item))
            return 0;
        return insert();
    }
}
