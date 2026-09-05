#include "../internal/application/ApplyCargoSlot.h"
#include <array>
#include <climits>
#include <cstdio>

// Sem wire/Win32: verifica a mutacao real usada no interceptador do baú.
int RunCargoSlotTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", name); }
    };
    using Item = std::array<unsigned char, 8>;
    Item cargo[128]{};
    const Item item{0x34, 0x12, 1, 2, 3, 4, 5, 6};
    for (int position : {0, 119, 120, 127}) {
        Item before[128];
        std::memcpy(before, cargo, sizeof(cargo));
        check(ApplyCargoSlot(cargo, position, item), "slot de armazenamento valido aceita copia");
        bool unchanged = true;
        for (int i = 0; i < 128; ++i)
            if (i != position && cargo[i] != before[i]) unchanged = false;
        check(cargo[position] == item && unchanged,
            "oito bytes atualizam somente o slot escolhido, incluindo reservados");
    }
    Item snapshot[128];
    std::memcpy(snapshot, cargo, sizeof(cargo));
    for (int position : {INT_MIN, -32768, -1, 128, 32767, INT_MAX})
        check(!ApplyCargoSlot(cargo, position, item), "indice invalido nao escreve no bau");
    check(std::memcmp(snapshot, cargo, sizeof(cargo)) == 0,
        "rejeicoes preservam todos os bytes do bau");
    check(ApplyCargoSlot(cargo, 127, cargo[127]) &&
        std::memcmp(snapshot, cargo, sizeof(cargo)) == 0,
        "reaplicar o mesmo slot e idempotente com alias");
    return failures;
}
