#pragma once

#include <cstddef>
#include <climits>

// Consulta pura do buffer emprestado de ocupacao. Nao altera itens ou ownership.
namespace grid_occupancy
{
    inline bool ContainsRectangle(int columns, int rows, int x, int y, int width, int height)
    {
        // Validar antes de somar/multiplicar: coordenadas de mouse e dimensoes
        // invalidas nao podem virar indices negativos ou overflow signed.
        return columns > 0 && rows > 0 && columns <= INT_MAX / rows &&
            x >= 0 && y >= 0 && x < columns && y < rows &&
            width > 0 && height > 0 && width <= columns - x && height <= rows - y;
    }

    // O chamador fornece columns*rows celulas; somente o valor 1 bloqueia,
    // preservando a regra legada. Rejeicao geometrica nao acessa o buffer.
    inline bool CanPlace(const int* cells, int columns, int rows,
        int x, int y, int width, int height)
    {
        if (!cells || !ContainsRectangle(columns, rows, x, y, width, height))
            return false;
        for (int dy = 0; dy < height; ++dy)
            for (int dx = 0; dx < width; ++dx)
                if (cells[(y + dy) * columns + x + dx] == 1)
                    return false;
        return true;
    }
}

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
