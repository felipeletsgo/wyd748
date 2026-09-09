#include "../internal/ui/GridInsertion.h"
#include <array>
#include <climits>
#include <cstdio>

// A mesma fronteira de SGrid envolve um consumidor observavel: rejeicao nao
// pode tocar lista, ocupacao, binding ou lifetime. Nao simula render DirectX.
int RunGridInsertionTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", name); }
    };
    struct Item { int owner = -1; };
    Item* list[128]{};
    std::array<Item, 129> items{};
    int count = 0, calls = 0, occupied = 0;
    auto append = [&](Item* item) {
        return grid_insertion::Execute(list, count, item, [&] {
            ++calls;
            item->owner = count;
            list[count++] = item;
            ++occupied;
            return 1;
        });
    };
    check(append(nullptr) == 0 && calls == 0, "nulo nao executa callback");
    for (int i = 0; i < 128; ++i)
        check(append(&items[i]) == 1 && list[i] == &items[i] && items[i].owner == i,
            "cada slot valido recebe ownership uma vez");
    check(append(&items[128]) == 0 && count == 128 && calls == 128 &&
        occupied == 128 && items[128].owner == -1,
        "lista cheia preserva ocupacao contador e ownership do caller");
    for (int i = 0; i < 128; ++i)
        check(list[i] == &items[i], "rejeicao conserva todos os slots");
    for (int bad : {INT_MIN, -1, 129, INT_MAX}) {
        count = bad;
        check(append(&items[128]) == 0 && count == bad && calls == 128,
            "contador invalido nao executa mutacao");
    }
    Item* small[2]{};
    check(grid_insertion::CanAppend(small, 1, &items[0]) &&
        !grid_insertion::CanAppend(small, 2, &items[0]),
        "capacidade vem do tipo do array, nao constante 128");
    int retries = 0;
    check(grid_insertion::Execute(small, 0, &items[0], [&] {
        ++retries; return 0;
    }) == 0 && retries == 1, "falha do consumidor propagada sem retry");
    // Consulta real usada por SGrid: varre todas as posicoes e footprints
    // de uma grade 9x7, comparando com uma enumeracao independente de celulas.
    std::array<int, 63> cells{};
    cells[0] = 1;
    cells[31] = 1;
    cells[62] = 1;
    cells[10] = 2; // O legado bloqueia exatamente 1, nao qualquer nao-zero.
    const auto original = cells;
    for (int y = -1; y <= 7; ++y)
        for (int x = -1; x <= 9; ++x)
            for (int height = 0; height <= 8; ++height)
                for (int width = 0; width <= 10; ++width) {
                    bool expected = x >= 0 && y >= 0 && width > 0 && height > 0 &&
                        x + width <= 9 && y + height <= 7;
                    if (expected)
                        for (int slot = 0; slot < 63; ++slot)
                            if (slot % 9 >= x && slot % 9 < x + width &&
                                slot / 9 >= y && slot / 9 < y + height && cells[slot] == 1)
                                expected = false;
                    check(grid_occupancy::CanPlace(cells.data(), 9, 7, x, y, width, height) == expected,
                        "consulta preserva ocupacao e limites em todos os retangulos pequenos");
                }
    for (int bad : {INT_MIN, -1, 0, INT_MAX}) {
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, 0, 0, bad, 1), "largura invalida rejeitada");
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, 0, 0, 1, bad), "altura invalida rejeitada");
    }
    for (int bad : {INT_MIN, -1, INT_MAX}) {
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, bad, 0, 1, 1), "x invalido rejeitado");
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, 0, bad, 1, 1), "y invalido rejeitado");
    }
    check(!grid_occupancy::CanPlace(nullptr, 9, 7, 0, 0, 1, 1), "ocupacao nula rejeitada");
    check(!grid_occupancy::CanPlace(cells.data(), INT_MAX, 2, 0, 0, 1, 1), "produto de dimensoes invalido rejeitado");
    check(!grid_occupancy::CanPlace(cells.data(), 9, -1, 0, 0, 1, 1), "grade negativa rejeitada");
    check(!grid_occupancy::CanPlace(cells.data(), 0, 7, 0, 0, 1, 1), "grade vazia rejeitada");
    check(cells == original, "consulta nao modifica ocupacao");
    // Buffer cercado por sentinelas: cobre recorte de equipamentos, escrita
    // sobre celula ocupada e retirada simetrica, sem depender do renderer.
    for (int y = 0; y < 7; ++y)
        for (int x = 0; x < 9; ++x)
            for (int height = 1; height <= 9; ++height)
                for (int width = 1; width <= 11; ++width) {
                    std::array<int, 65> buffer;
                    buffer.fill(77);
                    auto expected = buffer;
                    for (int slot = 0; slot < 63; ++slot)
                        if (slot % 9 >= x && slot % 9 < x + width &&
                            slot / 9 >= y && slot / 9 < y + height)
                            expected[slot + 1] = 1;
                    check(grid_occupancy::FillClipped(buffer.data() + 1, 9, 7, x, y, width, height, 1) &&
                        buffer == expected, "escrita recortada preserva sentinelas e celulas externas");
                    for (auto& cell : expected)
                        if (cell == 1) cell = 0;
                    check(grid_occupancy::FillClipped(buffer.data() + 1, 9, 7, x, y, width, height, 0) &&
                        buffer == expected, "retirada limpa exatamente o recorte inserido");
                }
    const int invalidRects[][4] = {
        {-1, 0, 1, 1}, {0, -1, 1, 1}, {9, 0, 1, 1}, {0, 7, 1, 1},
        {INT_MIN, 0, 1, 1}, {INT_MAX, 0, 1, 1}, {0, INT_MIN, 1, 1},
        {0, INT_MAX, 1, 1}, {0, 0, 0, 1}, {0, 0, 1, 0},
        {0, 0, -1, 1}, {0, 0, 1, INT_MIN}, {1, 0, INT_MAX, 1}, {0, 1, 1, INT_MAX}
    };
    for (const auto& rect : invalidRects) {
        Item item;
        int localCount = 0;
        const auto before = cells;
        const int result = grid_insertion::Execute(small, localCount, &item, [&] {
            if (!grid_occupancy::FillClipped(cells.data(), 9, 7,
                rect[0], rect[1], rect[2], rect[3], 1)) return 0;
            item.owner = 0;
            ++localCount;
            return 1;
        });
        check(result == 0 && item.owner == -1 && localCount == 0 && cells == before,
            "retangulo invalido preserva memoria contador e ownership");
    }
    check(!grid_occupancy::FillClipped(nullptr, 9, 7, 0, 0, 1, 1, 1), "escrita nula rejeitada");
    check(!grid_occupancy::FillClipped(cells.data(), INT_MAX, 2, 0, 0, 1, 1, 1), "produto de escrita invalido");
    check(grid_occupancy::FillClipped(cells.data(), 9, 7, 0, 0, INT_MAX, INT_MAX, 1),
        "footprint enorme representavel e recortado sem loop proporcional ao item");
    return failures;
}
