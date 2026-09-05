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
    return failures;
}
