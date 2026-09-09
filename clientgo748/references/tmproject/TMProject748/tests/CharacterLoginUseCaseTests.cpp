#include "../internal/application/RequestCharacterLogin.h"
#include <climits>
#include <initializer_list>
#include <cstdio>

namespace
{
// Duble sem wire: aceita qualquer indice, para que uma regressao na validacao
// do caso de uso nao seja escondida pela defesa adicional do encoder.
struct RecordingLoginSender final : ICharacterLoginSender
{
    int calls = 0;
    int lastSlot = -1;
    bool result = true;

    // Registra a intencao recebida e devolve o resultado local configurado.
    bool SendCharacterLogin(int slot) override
    {
        ++calls;
        lastSlot = slot;
        return result;
    }
};
}

// Unidade separada: compila application sem importar wire, socket ou Basedef.
// Acrescenta checks ao runner comum; permanece ativa com NDEBUG em Release.
int RunCharacterLoginUseCaseTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            std::fprintf(stderr, "FAIL: %s\n", name);
            ++failures;
        }
    };
    RecordingLoginSender sender;
    for (int slot = 0; slot < 4; ++slot) {
        check(RequestCharacterLogin(sender, slot), "intencao valida aceita");
        check(sender.calls == slot + 1 && sender.lastSlot == slot,
            "porta semantica recebe slot uma vez");
    }
    for (int slot : {INT_MIN, -1, 4, INT_MAX}) {
        check(!RequestCharacterLogin(sender, slot), "intencao invalida rejeitada");
    }
    check(sender.calls == 4, "caso de uso bloqueia invalidos antes do encoder");
    sender.result = false;
    check(!RequestCharacterLogin(sender, 2), "falha semantica propagada");
    check(sender.calls == 5 && sender.lastSlot == 2, "falha semantica sem retry");
    return failures;
}
