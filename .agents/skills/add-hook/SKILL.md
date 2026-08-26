---
name: add-hook
description: >-
  Add or modify a native hook in the WYD 7.54 plugin (Sourcer Star Micronics) —
  a new feature handler, a patched game formula, or an intercepted packet. Use
  when a change must run inside TMSRV.exe's own logic (not just when our plugin
  calls a wrapper). Encodes the file-by-file wiring and, critically, the
  inline-asm trampoline pitfalls (stack alignment, CALL vs JMP, thunk vs real
  function, bStatus vs Status) that have crashed this server before.
---

# Adding a native hook

Use this skill only for the legacy C++ plugin loaded by `TMSRV.exe`. Changes in
`wyd-go/` must use `$wyd-go-feature`; do not reproduce a Go feature as a binary
hook merely because a native address or wrapper exists.

A "hook" overwrites bytes at a fixed address inside `TMSRV.exe` so control jumps into our DLL. Get the asm/stack wrong and the server **crashes on every trigger** (often at login, since score recompute runs constantly). Work carefully and prefer verifying against a live OllyDbg data point over guessing.

## Preflight obrigatório

O checkout Go fica em `.../SERVER 7.54/wyd-go`, enquanto os projetos legados
ficam ao lado, em `.../SERVER 7.54/Sourcer Star Micronics` e
`.../SERVER 7.54/Server Star Micronics`. Resolver e exibir os caminhos absolutos
antes de editar; não assumir que `Sourcer Star Micronics/` existe no diretório
corrente. Se a solution, o exe alvo ou a pasta de deploy não existirem, parar e
informar qual artefato falta.

Antes da primeira edição, registrar:

- SHA-256 do `TMSRV.exe` realmente executado;
- endereço preferido, RVA e bytes originais completos no ponto de patch;
- limites das instruções sobrescritas e bytes que precisarão ser reproduzidos;
- callers/callees, thunk versus função real e frequência/thread de execução;
- calling convention, argumentos, retorno e cleanup (`RETN`/`RETN n`);
- registradores, flags, stack e estado global que devem sobreviver;
- risco de reentrância, recursão e concorrência;
- plano de desabilitação/recuperação se o servidor crashar no gatilho.

Endereço ou bytes vindos de outra versão são apenas pista. Não escrever o patch
se qualquer item acima ainda for inferido por semelhança.

## The 5 wiring points (all required)

1. **Handler** — write the C++ logic. Either a method on an existing class in `Objetos.h` (`Merch` = NPC interactions, `Volatiles` = item-use, `Outros`/`Atualizar`/`Packets`/`Respawn`), or a free function. New `.cpp` files must be added to `FunctionsV02.vcxproj` `<ClCompile>`.
2. **Declare it** — method in `Objetos.h` (and include `st_mob.h` there if the signature names `MOB`), or a prototype where free functions live.
3. **Trampoline** — a `__declspec(naked) void NKD_Xxx()` in `Naked.cpp` with inline `__asm` that marshals args, calls the handler, and returns to the right place.
4. **Prototype the trampoline** — `void NKD_Xxx();` in `DLLMain.h` (near the other `NKD_*` decls) or the compile fails with "undeclared identifier."
5. **Register the patch** — a `JMP_NEAR(addr, NKD_Xxx, nopBytes)` (or `JNZ_NEAR`/`CALL_NEAR`) line in `Outros::Carrega_Hook()` at the top of `Naked.cpp`.

Then build & deploy (see the `build-deploy` skill).

## Trampoline rules that have burned this project

**Stack alignment — the crash we hit twice.** At a hooked callsite the native function reads its args from exact stack slots (e.g. `[EBP+8] = clientid`) and often a register (`ECX = player base pointer`). If your trampoline does `CALL original` after entry, that `CALL` **pushes an extra return address**, shifting every stack arg by 4 — the native code then reads a return address where it expected `clientid`, uses it as an array index, and writes to wild memory → instant crash. Do **not** insert a `CALL` that shifts the caller's frame.

**To run code AFTER a native function, for ALL callers — use JMP return-hijack, not CALL:**
```asm
; entered by JMP from the hook site. stack: [ret_caller][arg], ECX = ptr
MOV  g_savedPtr, ECX          ; stash what the post-handler needs
POP  g_savedRet               ; take caller's return addr off the stack
PUSH OFFSET NKD_post          ; make the native fn return into OUR post-handler
MOV  EAX, 0x004123C0          ; the REAL function (see thunk note below)
JMP  EAX                      ; JMP (not CALL) — frame is byte-identical to a normal call
; NKD_post: run our code, then  PUSH g_savedRet / RETN  back to caller.
```
This is correct for both `RETN` and `RETN 4` native functions (the native fn's own return cleanup is unchanged because we JMP into it untouched). Register-save via globals is what the codebase already does, but it is **not reentrant** — use per-hook globals and be wary if the hooked fn can run on multiple threads.

**Thunk vs real function.** Many native entry points are 5-byte thunks (`0x0040149C: JMP 0x004123C0`). Hooking the *thunk* only catches callers that go through it; TMSRV internal code often calls the *real* function (`0x004123C0`) directly and bypasses your thunk hook. If a patch "works when our wrapper calls it but the game wipes it," you hooked the thunk and need to hook the real function instead — confirm which path fires with an OllyDbg breakpoint on both before committing.

**Signature changes = update the asm.** The `PUSH`/`ADD ESP` in a `NKD_*` are hand-tuned to the handler's arg count/order. Change the handler signature → update the trampoline.

## Data-model gotchas

- **`bStatus` vs `Status`** in `st_mob.h`'s `MOB`: `bStatus` is the base/raw score (allocated points); `Status` is the *computed* score that `SendStats`/`SendScore` transmit and combat uses. To change what the player sees/hits with, write `Status` (after the native recompute), not `bStatus`.
- **Bitfields matter.** Some single bytes pack two nibbles (`Speed:4 | ChaosRate:4`, `Merchant:4 | Direction:4`). `include/structs.h` documents the true layout; `st_mob.h` historically mismodeled some as plain `BYTE`, so a naive `+= 1` corrupted the neighboring nibble (this was the movement-speed bug). Match the real bitfield.
- **Never resize/reorder** fields in structs that mirror exe memory — layout is an ABI contract with the binary.
- **Addresses are version-locked** to our exact `TMSRV.exe`. Consult
  `$wyd-dev-knowledge` for source routing, then re-verify every address from a
  reference source in OllyDbg against our executable. When unsure of an address,
  calling convention, or `RETN` vs `RETN 4`, get the live data point first.

## Validação e recuperação

Depois da edição, usar `build-deploy` e separar claramente: build da DLL,
igualdade SHA-256 entre origem/deploy, startup do DBSRV/TMSRV e teste do gatilho
real. Validar também um caminho que não deve acionar o hook e repetir o gatilho
para revelar estado global ou reentrância.

Se houver crash, guardar dump/log e hashes, remover/desabilitar somente o patch
novo por uma cópia recuperável, confirmar que o servidor volta a iniciar e então
mapear a exceção para RVA/instrução. Nunca corrigir um crash alterando stack ou
número de NOPs por tentativa.
